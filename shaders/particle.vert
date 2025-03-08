#version 450

layout(binding = 0) uniform UniformLocationObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ulo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in mat4 inInstanceMat;

void main() {
    mat4 m = ulo.view * ulo.model;
    // Column 0:
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;

    // Column 1:
    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;

    // Column 2:
    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    gl_Position = ulo.proj * m * inInstanceMat * vec4(inPosition, 1.0);
}
