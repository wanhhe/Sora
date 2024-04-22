#include "scene_object.h"
#include "model.h"

unsigned int SceneObject::cur_id = 0;

SceneObject::SceneObject()
{
    id = cur_id++;
    this->name = "object";
    atr_transform = new ATR_Transform();
}

SceneObject::SceneObject(std::string _name, bool _is_editor) : name(_name), is_editor(_is_editor)
{ 
    id = cur_id++; 
    atr_transform = new ATR_Transform();
}

SceneObject::~SceneObject() {}

void SceneObject::RenderAttribute() 
{
    atr_transform->UI_Implement();
}

void SceneObject::Save(nlohmann::json& objectJson) {
    objectJson["name"] = name;
    objectJson["id"] = id;
    objectJson["is_editor"] = is_editor;
    objectJson["is_selected"] = is_selected;

    nlohmann::json transformJson;
    glm::vec3 front = atr_transform->transform->GetFront();
    transformJson["Front"] = { front.x, front.y, front.z };
    glm::vec3 right = atr_transform->transform->GetRight();
    transformJson["Right"] = { right.x, right.y, right.z };
    glm::vec3 up = atr_transform->transform->GetUp();
    transformJson["Up"] = { up.x, up.y, up.z };
    glm::vec3 pos = atr_transform->transform->Position();
    transformJson["position"] = { pos.x, pos.y, pos.z };
    glm::vec3 rot = atr_transform->transform->Rotation();
    transformJson["rotation"] = { rot.x, rot.y, rot.z };
    glm::vec3 scale = atr_transform->transform->Scale();
    transformJson["scale"] = { scale.x, scale.y, scale.z };

    objectJson["transform"] = transformJson;
}

void SceneObject::Load(const nlohmann::json& objectJson) {
    name = objectJson["name"];
    id = objectJson["id"];
    is_editor = objectJson["is_editor"];
    is_selected = objectJson["is_selected"];

    nlohmann::json transformJson = objectJson["transform"];
    atr_transform->transform->SetPosition(transformJson["position"][0], transformJson["position"][1], transformJson["position"][2]);
    atr_transform->transform->SetRotation(transformJson["rotation"][0], transformJson["rotation"][1], transformJson["rotation"][2]);
    atr_transform->transform->SetScale(transformJson["scale"][0], transformJson["scale"][1], transformJson["scale"][2]);
}

SceneModel::SceneModel(Model *_model, bool _is_editor) : model(_model)
{
    is_editor = _is_editor;
    for (int i = 0; i < _model->meshes.size(); i++)
    {
        Material* material;
        if (_model->meshes[i]->textures.size() > 0)
        {
            material = MaterialManager::CreateMaterialByType(PBR_MATERIAL);
            PBRMaterial* tmp = dynamic_cast<PBRMaterial*>(material);
            material->SetTexture(&tmp->albedo_map, _model->meshes[i]->textures[0]);
        }
        else
        {
            material = MaterialManager::CreateMaterialByType(PBR_MATERIAL);
        }
        MeshRenderer* _meshRenderer = new MeshRenderer(material, _model->meshes[i]);
        atr_meshRenderers.push_back(new ATR_MeshRenderer(_meshRenderer));
        meshRenderers.push_back(_meshRenderer);
    }
    _model->refSceneModels.AddRef(this);
}

SceneModel::SceneModel(Model *_model, std::string _name, bool _is_editor) : SceneModel(_model, _is_editor) { name = _name; }

void SceneModel::OnModelRemoved()
{
    model = nullptr;
    for (int i = 0; i < meshRenderers.size(); i++)
    {
        meshRenderers[i]->mesh = nullptr;
        atr_meshRenderers[i]->meshRenderer = meshRenderers[i];
    }
}

void SceneModel::RenderAttribute()
{
    atr_transform->UI_Implement();
    for (int i = 0; i < atr_meshRenderers.size(); i++)
    {
        atr_meshRenderers[i]->UI_Implement();
    }
}

void SceneModel::DrawSceneModel()
{
    for (int i = 0; i < meshRenderers.size(); i++)
    {
        meshRenderers[i]->Draw();
    }
}

void SceneModel::Save(nlohmann::json& objectJson) {
    SceneObject::Save(objectJson);

    nlohmann::json model_json;
    model_json["directory"] = model->directory;
    model_json["name"] = model->name;
    objectJson["model"] = model_json;

    nlohmann::json atr_mesh_render_json = nlohmann::json::array();
    for (int i = 0; i < atr_meshRenderers.size(); i++) {
        nlohmann::json render_json;
        atr_meshRenderers[i]->Save(render_json);
        atr_mesh_render_json.push_back(render_json);
    }
    objectJson["atr_mesh_renderers"] = atr_mesh_render_json;
}

void SceneModel::Load(const nlohmann::json& objectJson) {
    SceneObject::Load(objectJson);

    for (int i = 0; i < atr_meshRenderers.size(); i++) {
        atr_meshRenderers[i]->Load(objectJson["atr_mesh_renderers"][i].get<nlohmann::json>());
    }
}

SceneModel::~SceneModel() 
{
    if (model != nullptr)
    {
        model->refSceneModels.RemoveRef(this);
    }
    for (int i = 0; i < meshRenderers.size(); i++)
    {
        delete atr_meshRenderers[i];
        delete meshRenderers[i];
        atr_meshRenderers[i] = nullptr;
        meshRenderers[i] = nullptr;
    }
    delete atr_transform;
}

SceneLight::SceneLight(std::string _name, bool _is_editor) : SceneObject(_name, _is_editor) 
{
    // light = new ATR_DirLight(light_color);
    atr_lightRenderer = new ATR_LightRenderer(light_color);
    atr_transform->transform->SetPosition(0,10,0);
    atr_transform->transform->SetRotation(120, 0, 0);
}

glm::vec3 SceneLight::GetLightColor()
{
    return glm::vec3(light_color[0], light_color[1], light_color[2]);
}

void SceneLight::RenderAttribute()
{
    atr_transform->UI_Implement();
    atr_lightRenderer->UI_Implement();
}

void SceneLight::Save(nlohmann::json& objectJson) {
    SceneObject::Save(objectJson);

    nlohmann::json lightJson;
    lightJson["light_color"] = { light_color[0], light_color[1], light_color[2] };
    nlohmann::json atr_light_renderer;
    atr_lightRenderer->Save(atr_light_renderer);
    lightJson["atr_light_renderer"] = atr_light_renderer;

    objectJson["light"] = lightJson;
}

void SceneLight::Load(const nlohmann::json& objectJson) {
    SceneObject::Load(objectJson);
    atr_lightRenderer->Load(objectJson["light"]["atr_light_renderer"].get<nlohmann::json>());

    light_color[0] = objectJson["light"]["light_color"][0];
    light_color[1] = objectJson["light"]["light_color"][1];
    light_color[2] = objectJson["light"]["light_color"][2];
}

SceneLight::~SceneLight() {}