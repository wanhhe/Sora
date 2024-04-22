#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool isDepth;

float near = 0.1; 
float far  = 100.0; 

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
    vec4 col = texture(screenTexture, TexCoords).rgba;
    if (isDepth) {
        float depth = LinearizeDepth(col.x) / far;
        col.x = depth;
        col.y = depth;
        col.z = depth;
        col.a = 1;
    }
    FragColor = col;
} 