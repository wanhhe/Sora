#include "material.h"
#include "texture.h"
#include "shader.h"
#include <string>
#include "file_system.h"
#include "renderer_console.h"

unsigned int Material::cur_id = 0;

Material::Material() { id = cur_id++; }
Material::~Material() { RendererConsole::GetInstance()->AddLog("delete Material");        }
ModelMaterial::~ModelMaterial(){ RendererConsole::GetInstance()->AddLog("delete Model Material");  }
PBRMaterial::~PBRMaterial() { RendererConsole::GetInstance()->AddLog("delete PBR Material");    }
UnlitMaterial::~UnlitMaterial() { RendererConsole::GetInstance()->AddLog("delete Unlit Material");  }
NPRMaterial::~NPRMaterial() { RendererConsole::GetInstance()->AddLog("delete NPR Material"); }
bool Material::IsValid() { return shader != nullptr; }

void Material::SetTexture(Texture2D **slot, Texture2D *new_tex)
{
    (*slot)->textureRefs.RemoveRef(this);
    (*slot) = new_tex;
    (*slot)->textureRefs.AddRef(this);
}

void Material::DefaultSetup(std::vector<Texture2D *> default_textures)
{
    unsigned int gl_tex_id = 0;
    for (auto tex : material_variables.allTextures)
    {
        glActiveTexture(GL_TEXTURE4 + gl_tex_id);
        glUniform1i(glGetUniformLocation(shader->ID, (tex->slot_name + ".texture").c_str()), 4 + gl_tex_id);
        shader->setVec2((tex->slot_name + ".tilling").c_str(), tex->variable.tilling );
        shader->setVec2((tex->slot_name + ".offset").c_str(), tex->variable.offset );
        glBindTexture(GL_TEXTURE_2D, (*tex->variable.texture)->id);
        gl_tex_id++;
    }
    shader->use();
    for (auto value : material_variables.allColor)
    {
        shader->setVec3(value->slot_name.c_str(), glm::vec3(value->variable[0], value->variable[1], value->variable[2]));
    }
    for (auto value : material_variables.allFloat)
    {
        shader->setFloat(value->slot_name.c_str(), *value->variable);
    }
    for (auto value : material_variables.allInt)
    {
        shader->setInt(value->slot_name.c_str(), *value->variable);
    }
    for (auto value : material_variables.allVec3)
    {
        shader->setVec3(value->slot_name.c_str(), glm::vec3(value->variable[0], value->variable[1], value->variable[2]));
    }
}

void Material::OnTextureRemoved(Texture2D *removed_texture)
{
    for (int i = 0; i < material_variables.allTextures.size(); i++)
    {
        if (*(material_variables.allTextures[i]->variable.texture) == removed_texture)
        {
            this->SetTexture(material_variables.allTextures[i]->variable.texture, EditorContent::editor_tex["default_tex"]);
        }
    }
}

void Material::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["cur_id"] = cur_id;
    objectJson["cull_face"] = cullface;
}

void Material::Load(const nlohmann::json& objectJson) {
    id = objectJson["id"];
    cur_id = objectJson["cur_id"];
    cullface = objectJson["cull_face"];
}

Material* MaterialManager::CreateMaterialByType(EMaterialType type)
{
    switch (type)
    {
    case EMaterialType::MODEL_MATERIAL :
        return new ModelMaterial();
        break;
    
    case EMaterialType::PBR_MATERIAL :
        return new PBRMaterial();
        break;

    case EMaterialType::Unlit_MATERIAL :
        return new UnlitMaterial();
        break;

    case EMaterialType::NPR_MATERIAL:
        return new NPRMaterial();
        break;

    default:
        return new PBRMaterial();
        break;
    }
}

ModelMaterial::ModelMaterial() : Material()
{
    shader = Shader::LoadedShaders["model.fs"];

    albedo = EditorContent::editor_tex["default_tex"];
    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("albedo_map", MaterialTexture2D(&albedo)));
    albedo->textureRefs.AddRef(this);
    material_variables.allColor.push_back(new MaterialSlot<float*>("color", color));
}

ModelMaterial::ModelMaterial(Texture2D *_albedo) : Material::Material()
{
    shader = Shader::LoadedShaders["model.fs"];

    albedo = _albedo;
    albedo->textureRefs.AddRef(this);
    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("albedo_map", MaterialTexture2D(&albedo)));
    material_variables.allColor.push_back(new MaterialSlot<float *>("color", color));
}

void ModelMaterial::Setup(std::vector<Texture2D *> default_textures)
{ 
    DefaultSetup(default_textures);
}

void ModelMaterial::Save(nlohmann::json& objectJson) {
    Material::Save(objectJson);
    objectJson["name"] = name;
    objectJson["albedo_map"] = albedo->name;
    objectJson["color"] = { color[0], color[1], color[2] };
}

void ModelMaterial::Load(const nlohmann::json& objectJson) {
    // name = objectJson["name"];
    Material::Load(objectJson);
    SetTexture(&albedo, Texture2D::LoadedTextures[objectJson["albedo_map"]]);
    color[0] = objectJson["color"][0];
    color[1] = objectJson["color"][1];
    color[2] = objectJson["color"][2];
}

PBRMaterial::PBRMaterial() : Material::Material()
{
    shader = Shader::LoadedShaders["PBR.fs"];

    // Init all material variables
    albedo_map->textureRefs.AddRef(this);
    normal_map->textureRefs.AddRef(this);
    ao_map->textureRefs.AddRef(this);
    roughness_map->textureRefs.AddRef(this);
    metal_map->textureRefs.AddRef(this);

    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("albedo_map", MaterialTexture2D(&albedo_map)));
    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("normal_map", MaterialTexture2D(&normal_map)));
    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("ao_map", MaterialTexture2D(&ao_map)));
    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("roughness_map", MaterialTexture2D(&roughness_map)));
    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("metal_map", MaterialTexture2D(&metal_map)));

    material_variables.allColor.push_back(new MaterialSlot<float*>("color", color));

    material_variables.allFloat.push_back(new MaterialSlot<float*>("normalStrength", &normal_strength));
    material_variables.allFloat.push_back(new MaterialSlot<float*>("aoStrength", &ao_strength));
    material_variables.allFloat.push_back(new MaterialSlot<float*>("roughnessStrength", &roughness_strength));
    material_variables.allFloat.push_back(new MaterialSlot<float*>("metalStrength", &metal_strength));
    material_variables.allFloat.push_back(new MaterialSlot<float*>("shadowStrength", &shadow_strength));
}

void PBRMaterial::Setup(std::vector<Texture2D *> default_textures)
{ 
    DefaultSetup(default_textures);
}

void PBRMaterial::Save(nlohmann::json& objectJson) {
    Material::Save(objectJson);
    objectJson["name"] = name;
    objectJson["albedo_map"] = albedo_map->name;
    objectJson["normal_map"] = normal_map->name;
    objectJson["ao_map"] = ao_map->name;
    objectJson["roughness_map"] = roughness_map->name;
    objectJson["metal_map"] = metal_map->name;
    objectJson["color"] = {color[0], color[1], color[2]};
    objectJson["normal_strength"] = normal_strength;
    objectJson["ao_strength"] = ao_strength;
    objectJson["roughness_strength"] = roughness_strength;
    objectJson["metal_strength"] = metal_strength;
    objectJson["shadow_strength"] = shadow_strength;
}

void PBRMaterial::Load(const nlohmann::json& objectJson) {
    Material::Load(objectJson);
    SetTexture(&albedo_map, Texture2D::LoadedTextures[objectJson["albedo_map"]]);
    SetTexture(&normal_map, Texture2D::LoadedTextures[objectJson["normal_map"]]);
    SetTexture(&ao_map, Texture2D::LoadedTextures[objectJson["ao_map"]]);
    SetTexture(&roughness_map, Texture2D::LoadedTextures[objectJson["roughness_map"]]);
    SetTexture(&metal_map, Texture2D::LoadedTextures[objectJson["metal_map"]]);
    color[0] = objectJson["color"][0];
    color[1] = objectJson["color"][1];
    color[2] = objectJson["color"][2];
    normal_strength = objectJson["normal_strength"];
    ao_strength = objectJson["ao_strength"];
    roughness_strength = objectJson["roughness_strength"];
    metal_strength = objectJson["metal_strength"];
    shadow_strength = objectJson["shadow_strength"];
}

UnlitMaterial::UnlitMaterial() : Material()
{
    shader = Shader::LoadedShaders["texture.fs"];

    albedo_map = EditorContent::editor_tex["default_tex"];
    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("albedo_map", MaterialTexture2D(&albedo_map)));
    albedo_map->textureRefs.AddRef(this);
}

void UnlitMaterial::Setup(std::vector<Texture2D *> default_textures)
{ 
    DefaultSetup(default_textures);
}

void UnlitMaterial::Save(nlohmann::json& objectJson) {
    Material::Save(objectJson);
    objectJson["name"] = name;
    objectJson["albedo_map"] = albedo_map->name;
}

void UnlitMaterial::Load(const nlohmann::json& objectJson) {
    Material::Load(objectJson);
    SetTexture(&albedo_map, Texture2D::LoadedTextures[objectJson["albedo_map"]]);
}

NPRMaterial::NPRMaterial() : Material() {
    shader = Shader::LoadedShaders["npr1.fs"];

    albedo_map->textureRefs.AddRef(this);

    material_variables.allTextures.push_back(new MaterialSlot<MaterialTexture2D>("albedo_map", MaterialTexture2D(&albedo_map)));
}

void NPRMaterial::Setup(std::vector<Texture2D*> default_textures) {
    DefaultSetup(default_textures);
}

void NPRMaterial::Save(nlohmann::json& objectJson) {
    Material::Save(objectJson);
    objectJson["name"] = name;
    objectJson["albedo_map"] = albedo_map->name;
}

void NPRMaterial::Load(const nlohmann::json& objectJson) {
    Material::Load(objectJson);
    SetTexture(&albedo_map, Texture2D::LoadedTextures[objectJson["albedo_map"]]);
}