#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in vec4 aWeight;
layout (location = 6) in ivec4 aJoint;

out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 LightDir;
    vec3 LightColor;
    vec3 ViewPos;
    vec3 T;
    vec3 B;
    vec3 N;
    vec4 FragPosLightSpace;
    mat4 ViewMat;
    // ivec4 joint;
    // vec4 weight;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_view;
uniform mat4 light_projection;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightColor;

uniform mat4 animated[220]; // 已经是动画变换矩阵了
uniform bool isSkin;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    if (isSkin) {
        mat4 skin = animated[aJoint.x] * aWeight.x + animated[aJoint.y] * aWeight.y + animated[aJoint.z] * aWeight.z + animated[aJoint.w] * aWeight.w;
        vs_out.FragPos = vec3(model * skin * vec4(aPos, 1.0));
    } else {
        vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    }
    // vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    // vs_out.FragPos = vec3(aJoint.x, aJoint.y, aJoint.z);
    vs_out.Normal = normalMatrix * aNormal;  
    vs_out.TexCoords = aTexCoords;
    vec3 T = normalize(normalMatrix * aTangent).xyz;
    vec3 N = normalize(normalMatrix * aNormal).xyz;
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);
    vs_out.T = T;
    vs_out.B = B;
    vs_out.N = N;
    vs_out.LightDir = lightDir;
    vs_out.LightColor = lightColor;
    vs_out.ViewPos = viewPos;
    vs_out.FragPosLightSpace = light_projection * light_view * vec4(vs_out.FragPos, 1.0);
    vs_out.ViewMat = view;
    // vs_out.joint = aJoint;
    // vs_out.weight = aWeight;
    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}