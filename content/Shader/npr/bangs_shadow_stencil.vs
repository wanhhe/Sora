#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 aLightPos;
uniform float _Offset;

void main() {
   vec3 FragPos = vec3(model * vec4(aPos, 1.0));
   vec3 lightDir = normalize(aLightPos - FragPos);
   vec4 position = projection * view * model * vec4(aPos, 1.0);
   vec2 lightOffset = normalize(vec2(view * vec4(aLightPos, 1.0)));
   position.xy += lightOffset * _Offset;
   gl_Position = position;
}