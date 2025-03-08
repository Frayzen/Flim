#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform PointDesc {
  float size;
  float edgeSize;
  bool applyDiffuse;
} pointDesc;


void main() {
    outColor = vec4(1, 0, 0, 1);
}
