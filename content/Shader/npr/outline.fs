#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture_diffuse1;
uniform vec3 outlineColor;
void main() {
//"   FragColor = vec4(0.5, 0.0, 0.0, 1.0);
   FragColor = vec4(texture(texture_diffuse1, TexCoords).rgb * outlineColor, 1.0);
}