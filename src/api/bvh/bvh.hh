#pragma once

#include "api/render/mesh.hh"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <queue>
#include <stack>
#include <vector>

namespace Flim {

template <int nbElems> struct AABB {
  Vector3f min;
  Vector3f max;
  uint32_t elems[nbElems]; // indices of the element
};

struct Ray {
  Vector3f origin;
  Vector3f direction;
  bool cull;

  bool intersects(const Vector3f &v0, const Vector3f &v1, const Vector3f &v2,
                  float &dist) const {
    const float EPSILON = 0.0000001f;

    // Edges sharing v0
    Vector3f edge1 = v1 - v0;
    Vector3f edge2 = v2 - v0;

    // Begin calculating determinant - also used to calculate barycentric
    // coordinate u
    Vector3f pvec = direction.cross(edge2);

    // If determinant is near zero, ray lies in plane of triangle
    float det = edge1.dot(pvec);

    // Culling version (only hits front-facing triangles)
    if (det < EPSILON)
      return false;

    if (cull) {
      if (det < EPSILON)
        return false;
    } else {
      if (std::abs(det) < EPSILON)
        return false;
    }
    float invDet = 1.0f / det;

    // Calculate distance from v0 to ray origin
    Vector3f tvec = origin - v0;

    // Calculate u parameter and test bound
    float u = tvec.dot(pvec) * invDet;
    if (u < 0.0f || u > 1.0f)
      return false;

    // Prepare to test v parameter
    Vector3f qvec = tvec.cross(edge1);

    // Calculate v parameter and test bound
    float v = direction.dot(qvec) * invDet;
    if (v < 0.0f || u + v > 1.0f)
      return false;

    // Calculate t (distance), ray intersects triangle
    float t = edge2.dot(qvec) * invDet;

    if (t > EPSILON && t < dist) { // ray intersection
      dist = t;
      return true;
    }

    return false;
  }
  template <int nbElems> bool intersects(const AABB<nbElems> &box) const {
    Vector3f invDir = {1.0f / direction.x(), 1.0f / direction.y(),
                       1.0f / direction.z()};

    float tmin = -INFINITY;
    float tmax = INFINITY;

    // X-axis slab
    float tx1 = (box.min.x() - origin.x()) * invDir.x();
    float tx2 = (box.max.x() - origin.x()) * invDir.x();
    tmin = std::max(tmin, std::min(tx1, tx2));
    tmax = std::min(tmax, std::max(tx1, tx2));

    // Y-axis slab
    float ty1 = (box.min.y() - origin.y()) * invDir.y();
    float ty2 = (box.max.y() - origin.y()) * invDir.y();
    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    // Z-axis slab
    float tz1 = (box.min.z() - origin.z()) * invDir.z();
    float tz2 = (box.max.z() - origin.z()) * invDir.z();
    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    // Valid if the exit point is not behind the ray and entry precedes exit
    return tmax >= std::max(0.0f, tmin);
  }
};

// This class is subobtimal but does the job. Do not use for intensive
// computations
template <int nbElems> class BVH {
public:
  BVH(uint32_t triangles_amount, const void *triangles_ptr,
      uint32_t triangles_offset, uint32_t triangles_stride,
      uint32_t vertices_amount, const void *vertice_ptr,
      uint32_t vertices_offset, uint32_t vertices_stride)
      : boxes((triangles_amount + nbElems - 1) / nbElems),
        triangles(triangles_amount), vertices(vertices_amount) {
    const uint8_t *triangle_base = (const uint8_t *)triangles_ptr;
    for (size_t i = 0; i < triangles_amount; i++)
      triangles[i] = *(const Triangle *)(triangle_base + triangles_offset +
                                         (i * triangles_stride));
    const uint8_t *vertices_base = (const uint8_t *)vertice_ptr;
    for (size_t i = 0; i < vertices_amount; i++)
      vertices[i] = *(const Vector3f *)(vertices_base + vertices_offset +
                                        (i * vertices_stride));
    for (size_t i = 0; i < triangles.size(); i++) {
      AABB<nbElems> &box = boxes[i / nbElems];
      box.elems[i % nbElems] = i;
    }
    // Make sure coherent values for last aabb elems
    if (triangles.size() % nbElems) {
      for (size_t i = triangles.size() % nbElems; i < nbElems; i++)
        boxes.back().elems[i] = boxes.back().elems[0];
    }
    const float inf = INFINITY;
    for (size_t cur = boxes.size(); cur > 0; cur--) {
      size_t boxi = cur - 1;
      AABB<nbElems> &box = boxes[boxi];

      Vector3f min = {inf, inf, inf};
      Vector3f max = -min;
      for (uint32_t tid : box.elems) {
        const Triangle &t = triangles[tid];
        for (uint32_t vid : t) {
          min = min.cwiseMin(vertices[vid]);
          max = max.cwiseMax(vertices[vid]);
        }
      }
      auto left_i = boxi * 2 + 1;
      auto right_i = boxi * 2 + 2;
      if (left_i < boxes.size()) {
        min = min.cwiseMin(boxes[left_i].min);
        max = max.cwiseMax(boxes[left_i].max);
      }
      if (right_i < boxes.size()) {
        min = min.cwiseMin(boxes[right_i].min);
        max = max.cwiseMax(boxes[right_i].max);
      }
      box.min = min;
      box.max = max;
    }
  }

  BVH(const Mesh &m)
      : BVH(m.getTriangles().size(), m.getTriangles().data(), 0,
            sizeof(Triangle), m.getVertices().size(), m.getVertices().data(),
            offsetof(Vertex, pos), sizeof(Vertex)) {};

  bool castRay(const Ray &ray, uint32_t *result) {
    std::queue<uint32_t> candidates;
    candidates.push(0);
    float dist = INFINITY;

    while (!candidates.empty()) {
      uint32_t cur = candidates.front();
      candidates.pop();
      AABB<nbElems> &box = boxes[cur];
      if (ray.intersects(box)) {
        for (uint32_t elem : box.elems) {
          const Triangle &t = triangles[elem];
          const Vector3f &v1 = vertices[t.x()];
          const Vector3f &v2 = vertices[t.y()];
          const Vector3f &v3 = vertices[t.z()];
          if (ray.intersects(v1, v2, v3, dist))
            *result = elem;
        }
        uint32_t left = 2 * cur + 1;
        uint32_t right = 2 * cur + 2;
        if (left < boxes.size())
          candidates.push(left);
        if (right < boxes.size())
          candidates.push(right);
      }
    }
    return dist != INFINITY;
  }

private:
  std::vector<AABB<nbElems>> boxes;
  std::vector<Triangle> triangles;
  std::vector<Vector3f> vertices;
};
}; // namespace Flim
