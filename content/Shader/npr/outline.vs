#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float outlineWidth;
uniform float aspect;

void main() {
    TexCoords = aTexCoords;
    vec4 FragPos = projection * view * model * vec4(aPos, 1.0);
    // todo 到底在什么空间操作
    // vec4 Normal = projection * view * vec4(normalize(transpose(inverse(mat3(model))) * aNormal), 0);
    vec4 Normal = projection * vec4(normalize(transpose(inverse(mat3(view * model))) * aNormal), 0);
    Normal.x *= aspect;
    FragPos.xy += Normal.xy * FragPos.w * outlineWidth * 0.001;
    gl_Position = FragPos;
}