#version 330 core
layout (location = 0) in vec3 position;
layout (location = 5) in vec4 aWeight;
layout (location = 6) in ivec4 aJoint;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    FragPos = vec3(model * vec4(position, 1.0));
}