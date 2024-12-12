#version 450

layout(binding = 0) uniform UniformLocationObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ulo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ulo.proj * ulo.view * ulo.model * vec4(inPosition, 1.0);
    fragColor = inPosition;
    fragTexCoord = inTexCoord;
}
