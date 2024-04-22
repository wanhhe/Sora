#version 330 core
out vec4 color;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;

void main()
{             
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    color = vec4(result, 1.0f);
}