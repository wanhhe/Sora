#version 330 core
out vec4 FragColor;

in VS_OUT{
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
} fs_in;

struct Texture2D
{
    sampler2D texture;
    vec2 tilling;
    vec2 offset;
};

vec4 SampleTexture(Texture2D tex, vec2 uv)
{
    return texture(tex.texture, vec2(uv.xy * tex.tilling) + tex.offset);
}

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 color;
};

struct SpotLight {
    vec3 position;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 color;
};

#define POINT_LIGHTS 50
#define SPOT_LIGHTS 10

uniform sampler2D shadowMap;
uniform Texture2D albedo_map;
uniform vec3 color;

uniform PointLight pointLights[POINT_LIGHTS];
uniform SpotLight spotLights[SPOT_LIGHTS];
uniform int numPointLights;
uniform int numSpotLights;

vec3 CalcDirLight(vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

float near = 0.1; 
float far  = 100.0; 

// blinn-phong
void main()
{    
    vec3 norm = normalize(fs_in.Normal);
    vec3 result = CalcDirLight(norm, normalize(fs_in.LightDir));

    for(int i = 0; i < numPointLights; i++) {
        result += CalcPointLight(pointLights[i], norm, normalize(fs_in.LightDir)); 
    }
    for(int i = 0; i < numSpotLights; i++) {
        result += CalcSpotLight(spotLights[i], norm, normalize(fs_in.LightDir)); 
    }
            
    FragColor = vec4(result, 1.0);
}


// calculates the color when using a directional light.
vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    float specularStrength = 0.2;

    vec4 albedo = SampleTexture(albedo_map, fs_in.TexCoords);

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * fs_in.LightColor * albedo.xyz * color;
  	
    // diffuse 
    vec3 lightDir = -normalize(fs_in.LightDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * albedo.xyz * color;

    // specular
    vec3 reflectDir = reflect(-lightDir, normal); 
    vec3 halfwayDir = normalize(lightDir + viewDir); 
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 5);
    vec3 specular = specularStrength * spec * fs_in.LightColor;

    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir); 
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 5);
    // attenuation
    float distance = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec4 albedo = SampleTexture(albedo_map, fs_in.TexCoords);
    vec3 ambient = light.color * albedo.xyz;
    vec3 diffuse = light.color * diff * albedo.xyz;
    vec3 specular = 0.2 * spec * albedo.xyz;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir); 
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 5);
    // attenuation
    float distance = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-fs_in.LightDir)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec4 albedo = SampleTexture(albedo_map, fs_in.TexCoords);
    vec3 ambient = light.color * albedo.xyz;
    vec3 diffuse = light.color * diff * albedo.xyz;
    vec3 specular = 0.2 * spec * albedo.xyz;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}