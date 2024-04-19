#version 330 core
out vec4 FragColor;

#define POINT_LIGHTS 50
#define SPOT_LIGHTS 10

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

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
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 color;
};


uniform PointLight pointLights[POINT_LIGHTS];
uniform SpotLight spotLights[SPOT_LIGHTS];
uniform int numPointLights;
uniform int numSpotLights;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform Texture2D albedo_map;
uniform Texture2D normal_map;
uniform Texture2D metal_map;
uniform Texture2D roughness_map;
uniform Texture2D ao_map;

uniform vec3 color;
uniform float normalStrength;
uniform float aoStrength;
uniform float roughnessStrength;
uniform float metalStrength;
uniform bool useIBL;

uniform vec3 viewPos;


struct PBRLightingInfo
{
    vec3 Lo;
    vec3 ambient;
};

float saturate(float x)
{
    return clamp(x, 0, 1);
}

vec3 saturate(vec3 x)
{
    return clamp(x, vec3(0,0,0), vec3(1,1,1));
}

float GGX(float cos, float k)
{
    return cos / (k + (1 - k) * cos);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0) * 1, 5.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1 - F0) * pow(1.0 - cosTheta, 5.0);
}

PBRLightingInfo CalculatePBRLighting(vec3 n, vec3 albedo, float m, float r, vec3 ambient, float ao)
{
    float PI = 3.1415926;
    vec3 v = normalize(viewPos - WorldPos);
    vec3 R = reflect(-v, n); 

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, m);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < numPointLights; i++) {
        vec3 l = normalize(pointLights[i].position - WorldPos);
        vec3 h = normalize(l + v);
        float distance = length(pointLights[i].position - WorldPos);
        // float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + 
        //         pointLights[i].quadratic * (distance * distance));
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = pointLights[i].color * attenuation;

        float NdotH = max(dot(n, h), 0);
        float NdotL = max(dot(n, l), 0);
        float NdotV = max(dot(n, v), 0);

        // D
        float a = pow(r, 2);
        float a2 = max(pow(a, 2), 0.000001);
        float D_n = a2 / (PI * (pow(NdotH, 2) * (a2 - 1) + 1));

        //F
        vec3 F = fresnelSchlick(max(dot(h, v), 0), F0);

        //G
        float k = pow((a + 1), 2) / 8;
        float G = GGX(saturate(NdotL), k) * GGX(saturate(NdotV), k);

        vec3 specular = (D_n * F * G) / max((4 * NdotV * NdotL), 0.001);

        vec3 Ks = F;
        vec3 Kd = saturate(1 - Ks) * (1 - m);

        Lo += (Kd * albedo / PI + specular) * radiance * NdotL;
    }

    PBRLightingInfo pbr;
    pbr.Lo = Lo;
    if (useIBL) {
        // ambient lighting (we now use IBL as the ambient term)
        vec3 F = fresnelSchlickRoughness(max(dot(n, v), 0.0), F0, r);

        vec3 kS = F;
        vec3 kD = saturate(1 - kS) * (1 - m);

        vec3 irradiance = texture(irradianceMap, n).rgb;
        vec3 diffuse = irradiance * albedo;

        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R,  r * MAX_REFLECTION_LOD).rgb;    
        vec2 brdf  = texture(brdfLUT, vec2(max(dot(n, v), 0.0), r)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        pbr.ambient = (kD * diffuse + specular) * ao;
    } else {
        pbr.ambient = ambient; 
    }

    return pbr;
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = SampleTexture(normal_map, TexCoords).rgb * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{    
    vec4 albedo = SampleTexture(albedo_map, TexCoords);
    vec4 metallic = SampleTexture(metal_map, TexCoords) * metalStrength;
    vec4 ao = SampleTexture(ao_map, TexCoords) * aoStrength;
    vec4 roughness = SampleTexture(roughness_map, TexCoords) * roughnessStrength;
    vec3 N = getNormalFromMap();
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * albedo.xyz;

    PBRLightingInfo PBR = CalculatePBRLighting(N, color * albedo.xyz, metallic.x, roughness.x, ambient, ao.x);
    vec3 result = (PBR.Lo + PBR.ambient) / (PBR.Lo + PBR.ambient + vec3(1.0));
    FragColor = vec4(result, 1.0);
}