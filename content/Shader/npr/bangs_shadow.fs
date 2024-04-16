#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D ilmTexture;
uniform sampler2D texture_diffuse1;
uniform sampler2D sdfMap;
uniform sampler2D hairLightTexture;
uniform sampler2D hairShadowMap;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform vec3 _ShadowColor;
uniform vec3 _SpecularColor;
uniform float _SpecularRange;
uniform float _SpecularMulti;
uniform float _SpecularGloss;
uniform vec3 _LightColor0;
uniform vec3 _FresnelColor;
uniform float _FresnelEff;
uniform vec3 _HairShadowColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
   return F0 +(1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
   vec3 diffuseTex = texture(texture_diffuse1, TexCoords).rgb;
   vec3 ilmTex = texture(ilmTexture, TexCoords).rgb;
   vec3 normal = normalize(Normal);
   vec3 lightDir = normalize(lightPos - FragPos);
   vec3 viewDir = normalize(viewPos - FragPos);
   vec3 halfDir = normalize(lightDir + viewDir);
   float NdotH = max(dot(normal, halfDir), 0.0);
   float halfLambert = dot(normal, lightDir) * 0.5 + 0.5;
   float threshold = (halfLambert + ilmTex.g) * 0.5;
   // 这里可以考虑设为相同阴影色或者刘海色, 根据是唯一阴影色还是平滑阴影色决定吧
   vec3 diffuse = _ShadowColor;
   //  vec3 diffuse = _HairShadowColor;
   diffuse *= diffuseTex;
   vec3 specular = vec3(0.0);
   float specularSize = pow(NdotH, _SpecularGloss);
   float specularMark = ilmTex.b;
   if (specularSize >= 1 - specularMark * _SpecularRange) {
       specular = _SpecularMulti * ilmTex.r * _SpecularColor;
   }
   vec3 fresnel = _FresnelEff * fresnelSchlick(max(dot(viewDir, halfDir), 0.0), vec3(0.04));
   vec3 color = _LightColor0 * (diffuse + specular) + fresnel * _FresnelColor;
   FragColor = vec4(color, 1.0);
}