#version 450

layout(binding = 0) uniform UniformLocationObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ulo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in mat4 inInstanceMat;

layout(location = 1) out vec4 color;

void main() {
    mat4 transform = ulo.model * inInstanceMat;
    gl_Position = ulo.proj * ulo.view * transform * vec4(inPosition, 1.0);
    vec4 pos = gl_Position;
    if (pos.w != 0)
      pos /= pos.w;
    color = vertexColor;
}
