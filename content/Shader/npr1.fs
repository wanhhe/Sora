#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

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

uniform Texture2D albedo_map;
uniform sampler2D ilmTexture1;
// uniform sampler2D albedo_map;
uniform sampler2D sdfMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 _AmbientColor;
uniform vec3 _LightColor;
uniform float _ShadowEdge0;
uniform float _ShadowEdge1;
uniform vec3 _SpecularColor;
uniform float _SpecularRange;
uniform float _SpecularGloss;
uniform vec3 _RimColor;
uniform float _RimAmount;
uniform float _MinRim;
uniform float _MaxRim;
uniform float _IsFace;
uniform float _RimBloomExp;
uniform float _Bloom;

void main() {
   // vec3 color = texture(albedo_map, TexCoords).rgb;
   vec3 color = SampleTexture(albedo_map, TexCoords).rgb;
   vec3 ilmTex = texture(ilmTexture1, TexCoords).rgb;
   vec3 normal = normalize(Normal);
   if(!gl_FrontFacing) normal = -normal;
   vec3 viewDir = normalize(viewPos - FragPos);
   vec3 lightDir = normalize(lightPos - FragPos);
   float NdotL = max(dot(normal, lightDir), 0);
   float wrapLambert = NdotL + ilmTex.g;
   float shadowStep = smoothstep(_ShadowEdge0, _ShadowEdge1, wrapLambert);
   // vec3 diffuseShadow = mix(_AmbientColor, _LightColor, shadowStep);
   vec3 halfwayDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(normal, halfwayDir), 0), _SpecularGloss);
   float specularRange = step(_SpecularRange, (spec + clamp(dot(normal.xz, lightDir.xz), 0.0, 1.0)) * ilmTex.r * ilmTex.b);
   vec3 specular = specularRange * _SpecularColor;
   vec3 Front = vec3(0, 0, -1.0);
   float rimDot = 1 - clamp(dot(viewDir, normal), 0.0, 1.0);
   rimDot = smoothstep(_MinRim, _MaxRim, rimDot);
   float rimIntensity = rimDot * clamp(dot(-Front.xz, lightDir.xz), 0.0, 1.0);
   rimIntensity = smoothstep(_RimAmount - 0.3, _RimAmount + 0.1, rimIntensity);
   if (dot(normal, Front) < 0) rimIntensity = 0.0;
//"   vec3 rimColor = rimIntensity * _LightColor;
   vec3 rimColor = rimIntensity * _RimColor;
   vec4 sdfIlm = texture(sdfMap, TexCoords);
   vec4 r_sdfIlm = texture(sdfMap, vec2(1 - TexCoords.x, TexCoords.y));
   vec3 Left = normalize(vec3(1,0,0));
   vec3 Right = -Left;
   float ctrl = step(0, dot(Front.xz, lightDir.xz));
   float faceShadow = ctrl * min(step(dot(Left.xz, lightDir.xz), r_sdfIlm.r), step(dot(Right.xz, lightDir.xz), sdfIlm.r));
   // 如果是脸的话就是1，身体就是0，然后乘
   float isFace = step(0.1, _IsFace);
   float shadow = isFace * faceShadow + (1.0 - isFace) * shadowStep;
   // vec3 diffuse = mix(_AmbientColor, _LightColor, faceShadow);
   vec3 diffuse = mix(_AmbientColor, _LightColor, shadow);
//"   float ctrl = 1 - clamp(dot(Front, lightDir) * 0.5 + 0.5, 0.0, 1.0);
//"   float ilm = dot(lightDir, Left) > 0 ? sdfIlm.r : r_sdfIlm.r;
//"   float isShadow = step(ilm, ctrl);
//"   float bias = smoothstep(0.6, 0.8, abs(ctrl - ilm));
//"   vec3 diffuse = color;
//"   if (ctrl > 0.99 || isShadow == 1) diffuse = mix_LightColor, _AmbientColor ,bias);
//"   vec3 lighting = rimColor + (shadow + specular) * color;
   vec3 lighting = (rimColor + diffuse + specular) * color;
   float rimBloom;
   float f = 1.0 - clamp(dot(viewDir, normal), 0.0, 1.0);
// 背光直接是1， 不背光
   if (dot(normal, lightDir) < 0.2 || f < 0.6) {
       rimBloom = 1.0;
   } else {
       rimBloom = pow(f, _RimBloomExp) * _Bloom * dot(normal, lightDir);
   }


   FragColor = vec4(color , rimBloom);
   
   // if (ilmTex.g == 0.0) FragColor = vec4(1.0);
//"   FragColor = vec4(mix(_AmbientColor, _LightColor, shadowStep), 1.0);
//"   FragColor = vec4(specular, 1.0);
//"   if (spec == 0) FragColor = vec4(0.5, 0, 0, 1.0);
//"   if (ilmTex.r == 0 || ilmTex.b == 0) FragColor = vec4(0, 0.5, 0, 1.0);
//"   if (rimColor.x > 0.2 || rimColor.y > 0.2 || rimColor.z > 0.2) FragColor = vec4(0.5, 0, 0, 1.0);
//"   if (NdotL > 0) FragColor = vec4(0.5, 0, 0, 1.0);
//"   if(!gl_FrontFacing) FragColor = vec4(0.5, 0, 0, 1.0);
//"   if (dot(vec2(0, 1), lightDir.xz) < 0) FragColor = vec4(0.5, 0, 0, 1.0);
//"   if (sdfIlm.x > 0.7) FragColor = vec4(0.5, 0, 0, 1.0);
}