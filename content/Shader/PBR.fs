#version 330 core
layout (location = 0) out vec4 FragColor;

#define POINT_LIGHTS 50
#define SPOT_LIGHTS 10

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
    // ivec4 joint;
    // vec4 weight;
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

uniform sampler2D shadowMap;
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
uniform float shadowStrength;
uniform bool useIBL;

float near = 0.1; 
float far  = 100.0;


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

vec3 CalculatePointLighting(vec3 n, vec3 v, float r, vec3 F0, float m, vec3 albedo) {
    float PI = 3.1415926;
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < numPointLights; i++) {
        vec3 l = normalize(pointLights[i].position - fs_in.FragPos);
        vec3 h = normalize(l + v);
        float distance = length(pointLights[i].position - fs_in.FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + 
                pointLights[i].quadratic * (distance * distance));
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
    return Lo;
}

vec3 CalculateSpotLighting(vec3 n, vec3 v, float r, vec3 F0, float m, vec3 albedo) {
    float PI = 3.1415926;
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < numSpotLights; i++) {
        vec3 l = normalize(spotLights[i].position - fs_in.FragPos);
        float theta = dot(l, normalize(-spotLights[i].direction));
        float epsilon = spotLights[i].cutOff - spotLights[i].outerCutOff;
        float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);  
        vec3 h = normalize(l + v);
        float distance = length(spotLights[i].position - fs_in.FragPos);
        float attenuation = 1.0 / (spotLights[i].constant + spotLights[i].linear * distance + 
                spotLights[i].quadratic * (distance * distance));
        vec3 radiance = spotLights[i].color * attenuation;

        float NdotH = dot(n, h);
        float NdotL = dot(n, l);
        float NdotV = dot(n, v);

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

        Lo += (Kd * albedo / PI + specular) * intensity * NdotL;
    }
    return Lo;
}

vec3 CalculateDirLighting(vec3 n, vec3 v, float r, vec3 F0, float m, vec3 albedo) {
    float PI = 3.1415926;
    vec3 l = -normalize(fs_in.LightDir);
    vec3 h = normalize(l + v);
    vec3 radiance = fs_in.LightColor;

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

    return (Kd * albedo / PI + specular) * radiance * NdotL;
}

PBRLightingInfo CalculatePBRLighting(vec3 n, vec3 albedo, float m, float r, vec3 ambient, float ao)
{
    float PI = 3.1415926;
    vec3 v = normalize(fs_in.ViewPos - fs_in.FragPos);
    vec3 R = reflect(-v, n); 

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, m);

    vec3 Lo = vec3(0.0);

    Lo += CalculateDirLighting(n, v, r, F0, m, albedo);
    Lo += CalculatePointLighting(n, v, r, F0, m, albedo);
    Lo += CalculateSpotLighting(n, v, r, F0, m, albedo);

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
        vec2 brdf = texture(brdfLUT, vec2(max(dot(n, v), 0.0), r)).rg;
        // vec2 brdf = texture(brdfLUT, vec2(0.5,0.5)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
        pbr.ambient = (kD * diffuse + specular) * ao;
    } else {
        pbr.ambient = ambient; 
    }

    return pbr;
}

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

vec3 GetPBRLightingResult(PBRLightingInfo PBR, float NdotL, float shadow)
{
    // return  (1 - shadow) * (PBR.Lo + PBR.ambient) / (PBR.Lo + PBR.ambient + vec3(1.0)) + PBR.ambient * shadow;
    return  (1 - shadow) * (PBR.Lo + PBR.ambient) + PBR.ambient * shadow;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;
    // 检查当前片段是否在阴影中
    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.0001);
    // float bias = 0.00001;
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow * shadowStrength;
}

void main()
{    
    vec4 albedo = SampleTexture(albedo_map, fs_in.TexCoords);
    vec4 metallic = SampleTexture(metal_map, fs_in.TexCoords) * metalStrength;
    vec4 ao = SampleTexture(ao_map, fs_in.TexCoords) * aoStrength;
    vec4 roughness = SampleTexture(roughness_map, fs_in.TexCoords) * roughnessStrength;
    vec3 normalWS = SampleTexture(normal_map, fs_in.TexCoords).rgb;

    normalWS = normalize(normalWS * 2.0 - 1.0);
    normalWS.xy *= normalStrength;
    mat3 TBN = mat3(fs_in.T, fs_in.B, fs_in.N);
    normalWS = normalize(TBN * normalWS);
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * fs_in.LightColor * albedo.xyz;
    vec3 lightDir = normalize(-fs_in.LightDir);
    float NdotL = max(dot(normalWS, lightDir), 0.0);

    PBRLightingInfo PBR = CalculatePBRLighting(normalWS, color * albedo.xyz, metallic.x, roughness.x, ambient, ao.x);
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normalWS, lightDir);
    vec3 midres = GetPBRLightingResult(PBR, NdotL, shadow);
    FragColor = vec4(midres.rgb, 1.0);
    // FragColor = vec4((PBR.Lo + PBR.ambient) / (PBR.Lo + PBR.ambient + vec3(1.0)), 1.0);
}