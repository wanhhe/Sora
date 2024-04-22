#include "attributes.h"
#include "scene_object.h"
#include "texture.h"
#include "model.h"
#include "shader.h"
#include "imgui.h"
#include "postprocess.h"

unsigned int ATR_Material::cur_id           = 10000;
unsigned int ATR_MaterialTexture::cur_id    = 20000;
unsigned int ATR_MaterialFloat::cur_id      = 30000;
unsigned int ATR_MaterialInt::cur_id        = 40000;
unsigned int ATR_MaterialColor::cur_id      = 50000;
unsigned int ATR_PostProcessNode::cur_id    = 60000;
unsigned int ATR_MeshRenderer::cur_id        = 70000;
unsigned int ATR_LightRenderer::cur_id = 80000;

ATR_Transform::ATR_Transform()
{
    transform = new Transform();
}



void ATR_Transform::UI_Implement()
{
    // position
    static float tmp_position[3];
    tmp_position[0] = transform->Position().r;
    tmp_position[1] = transform->Position().g;
    tmp_position[2] = transform->Position().b;
    ImGui::DragFloat3("Position", tmp_position, 0.1f);
    transform->SetPosition(tmp_position[0], tmp_position[1], tmp_position[2]);

    // rotation
    static float tmp_rotation[3];
    tmp_rotation[0] = transform->Rotation().r;
    tmp_rotation[1] = transform->Rotation().g;
    tmp_rotation[2] = transform->Rotation().b;
    ImGui::DragFloat3("Rotation", tmp_rotation, 0.1f);
    transform->SetRotation(tmp_rotation[0], tmp_rotation[1], tmp_rotation[2]);

    // scale
    static float tmp_scale[3];
    tmp_scale[0] = transform->Scale().r;
    tmp_scale[1] = transform->Scale().g;
    tmp_scale[2] = transform->Scale().b;
    ImGui::DragFloat3("Scale", tmp_scale, 0.1f);
    transform->SetScale(tmp_scale[0], tmp_scale[1], tmp_scale[2]);
}

ATR_MaterialTexture::ATR_MaterialTexture(std::string _name, Material *_material, MaterialTexture2D *_texture) : material(_material),
                                                                                                         mat_tex(_texture),
                                                                                                         slot_name(_name)
{
    id = cur_id++;
}

void ATR_MaterialTexture::UI_Implement()
{
    std::string item_name = "null";
    std::string tex_type = "null";
    if (mat_tex->texture != nullptr)
    {
        const char* types[]= {"RED", "RGB", "RGBA", "SRGB", "SRGBA", "Skybox"};
        item_name = (*mat_tex->texture)->name;
        tex_type = types[(*mat_tex->texture)->tex_type];
    }
    std::string material_id = std::to_string(material->id);
    std::string atrtex_id = std::to_string(id);

    ImGui::BeginChild(("child##" + material_id + atrtex_id).c_str(), ImVec2(200, 65), ImGuiChildFlags_Border);
    ImGui::Text(slot_name.c_str());
    ImGui::Text(("name:" + item_name).c_str());
    ImGui::Text(("type:" + tex_type).c_str());
    ImGui::EndChild();
    ImGui::SameLine();
    if (mat_tex->texture != nullptr)
    {
        float width = 32;
        float height = 32;
        ImVec2 uv_min = ImVec2(0.0f, 0.0f); // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f); // Lower-right
        ImVec4 bg_col = ImVec4(0, 0, 0, 0);
        ImVec4 tint_col = ImVec4(1, 1, 1, 1);

        std::vector<std::string> tex_names;
        auto tmp = Texture2D::LoadedTextures;
        for (std::map<string, Texture2D *>::iterator it = tmp.begin(); it != tmp.end(); it++)
        {
            tex_names.push_back(it->first);
        }

        if (ImGui::ImageButton(("texture##" + material_id + atrtex_id).c_str(), (GLuint *)(*mat_tex->texture)->id, ImVec2(width, height), uv_min, uv_max, bg_col, tint_col))
        {
            ImGui::OpenPopup(("popup##" + material_id + atrtex_id).c_str());
        }

        if (ImGui::BeginPopup(("popup##" + material_id + atrtex_id).c_str()))
        {
            for (int n = 0; n < tex_names.size(); n++)
            {
                if (ImGui::Selectable((tex_names[n] + "##" + material_id + atrtex_id + std::to_string(n)).c_str()))
                {
                    material->SetTexture(mat_tex->texture, Texture2D::LoadedTextures[tex_names[n]]);
                }
                ImGui::SameLine();
                ImGui::Image((GLuint *)Texture2D::LoadedTextures[tex_names[n]]->id, ImVec2(16, 16), uv_min, uv_max);
            }
            ImGui::EndPopup();
        }
    }
    tilling[0] = mat_tex->tilling.r; tilling[1] = mat_tex->tilling.g;
    offset[0] = mat_tex->offset.r;   offset[1] = mat_tex->offset.g;
    ImGui::DragFloat2(("tilling##" + material_id + atrtex_id).c_str(), tilling, 0.1f);
    ImGui::DragFloat2(("offset##" + material_id + atrtex_id).c_str(), offset, 0.01f);
    mat_tex->tilling.r = tilling[0]; mat_tex->tilling.g = tilling[1];
    mat_tex->offset.r = offset[0];   mat_tex->offset.g = offset[1];
}

void ATR_MaterialTexture::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["slot_name"] = slot_name;
    objectJson["cur_id"] = cur_id;
    objectJson["tilling"] = { tilling[0], tilling[1] };
    objectJson["offset"] = { offset[0], offset[1] };

    nlohmann::json material_json;
    material->Save(material_json);
    objectJson["material"] = material_json;

    nlohmann::json mat_tex_json;
    mat_tex_json["name"] = (*mat_tex->texture)->name;
    mat_tex_json["tilling"] = { mat_tex->tilling[0], mat_tex->tilling[1] };
    mat_tex_json["offset"] = { mat_tex->offset[0], mat_tex->offset[1] };
    objectJson["mat_tex"] = mat_tex_json;
}

void ATR_MaterialTexture::Load(const nlohmann::json& objectJson) {
    id = objectJson["id"];
    slot_name = objectJson["slot_name"];
    cur_id = objectJson["cur_id"];
    tilling[0] = objectJson["tilling"][0];
    tilling[1] = objectJson["tilling"][1];
    offset[0] = objectJson["offset"][0];
    offset[1] = objectJson["offset"][1];

    nlohmann::json material_json = objectJson["material"];
    material->Load(material_json);

    nlohmann::json mat_tex_json = objectJson["mat_tex"];
    mat_tex->tilling[0] = mat_tex_json["tilling"][0];
    mat_tex->tilling[1] = mat_tex_json["tilling"][1];
    mat_tex->offset[0] = mat_tex_json["offset"][0];
    mat_tex->offset[1] = mat_tex_json["offset"][1];
    material->SetTexture(mat_tex->texture, Texture2D::LoadedTextures[mat_tex_json["name"]]);
}

ATR_MaterialFloat::ATR_MaterialFloat(std::string _name, Material *_material, float *_value) :   slot_name(_name), 
                                                                                                material(_material), 
                                                                                                value(_value)
{
    id = cur_id++;
}

void ATR_MaterialFloat::UI_Implement()
{
    std::string item = slot_name + "##" + std::to_string(material->id) + std::to_string(id);
    ImGui::DragFloat(item.c_str(), value, drag_speed);
}

void ATR_MaterialFloat::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["slot_name"] = slot_name;
    objectJson["drag_speed"] = drag_speed;
    objectJson["cur_id"] = cur_id;
    objectJson["value"] = *value;
    nlohmann::json material_json;
    material->Save(material_json);
    objectJson["material"] = material_json;
}

void ATR_MaterialFloat::Load(const nlohmann::json& objectJson) {
    id = objectJson["id"];
    slot_name = objectJson["slot_name"];
    drag_speed = objectJson["drag_speed"];
    cur_id = objectJson["cur_id"];
    *value = objectJson["value"];

    nlohmann::json material_json = objectJson["material"];
    material->Load(material_json);
}

ATR_MaterialInt::ATR_MaterialInt(std::string _name, Material *_material, int *_value) :     slot_name(_name), 
                                                                                            material(_material), 
                                                                                            value(_value)
{
    id = cur_id++;
}

void ATR_MaterialInt::UI_Implement()
{
    std::string item = slot_name + "##" + std::to_string(material->id) + std::to_string(id);
    ImGui::DragInt(item.c_str(), value, drag_speed);
}

void ATR_MaterialInt::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["slot_name"] = slot_name;
    objectJson["drag_speed"] = drag_speed;
    objectJson["cur_id"] = cur_id;
    objectJson["value"] = *value;

    nlohmann::json material_json;
    material->Save(material_json);
    objectJson["material"] = material_json;
}

void ATR_MaterialInt::Load(const nlohmann::json& objectJson) {
    id = objectJson["id"];
    slot_name = objectJson["slot_name"];
    drag_speed = objectJson["drag_speed"];
    cur_id = objectJson["cur_id"];
    *value = objectJson["value"];

    nlohmann::json material_json = objectJson["material"];
    material->Load(material_json);
}

ATR_MaterialColor::ATR_MaterialColor(std::string _name, Material *_material, float *_value) :   slot_name(_name), 
                                                                                                material(_material), 
                                                                                                value(_value)
{
    id = cur_id++;
}

void ATR_MaterialColor::UI_Implement()
{
    std::string item = slot_name + "##" + std::to_string(material->id) + std::to_string(id);
    ImGui::ColorEdit3(item.c_str(), value);
}

void ATR_MaterialColor::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["slot_name"] = slot_name;
    objectJson["drag_speed"] = drag_speed;
    objectJson["cur_id"] = cur_id;
    objectJson["value"] = { value[0], value[1], value[2] };

    nlohmann::json material_json;
    material->Save(material_json);
    objectJson["material"] = material_json;
}

void ATR_MaterialColor::Load(const nlohmann::json& objectJson) {
    id = objectJson["id"];
    slot_name = objectJson["slot_name"];
    drag_speed = objectJson["drag_speed"];
    cur_id = objectJson["cur_id"];
    value[0] = objectJson["value"][0];
    value[1] = objectJson["value"][1];
    value[2] = objectJson["value"][2];

    nlohmann::json material_json = objectJson["material"];
    material->Load(material_json);
}


ATR_Material::ATR_Material(Material* _material) : material(_material)
{
    for (int i = 0; i < _material->material_variables.allTextures.size(); i++)
    {
        atr_textures.push_back(new ATR_MaterialTexture(_material->material_variables.allTextures[i]->slot_name,
                                                       _material,
                                                       &_material->material_variables.allTextures[i]->variable));
    }

    for (int i = 0; i < _material->material_variables.allInt.size(); i++)
    {
        atr_ints.push_back(new ATR_MaterialInt(_material->material_variables.allInt[i]->slot_name,
                                               _material,
                                               _material->material_variables.allInt[i]->variable));
    }

    for (int i = 0; i < _material->material_variables.allFloat.size(); i++)
    {
        atr_floats.push_back(new ATR_MaterialFloat(_material->material_variables.allFloat[i]->slot_name,
                                                   _material,
                                                   _material->material_variables.allFloat[i]->variable));
    }

    for (int i = 0; i < _material->material_variables.allVec3.size(); i++)
    {
        atr_vec3s.push_back(_material->material_variables.allVec3[i]->variable);
    }

    for (int i = 0; i < _material->material_variables.allColor.size(); i++)
    {
        atr_colors.push_back(new ATR_MaterialColor(_material->material_variables.allColor[i]->slot_name,
                                                   _material,
                                                   _material->material_variables.allColor[i]->variable));
    }
    id = cur_id++;
}


void ATR_Material::UI_Implement()
{
    ImGui::Text("Material Info:");
    const char *items[] = {"Cull off", "Cull front", "Cull back"};
    std::string item = "cull face##" + std::to_string(material->id);
    ImGui::Combo(item.c_str(), &cull_current, items, IM_ARRAYSIZE(items));
    material->cullface = (E_CULL_FACE)cull_current;

    for (int i = 0; i < atr_textures.size(); i++)
    {
        atr_textures[i]->UI_Implement();
    }

    for (int i = 0; i < atr_ints.size(); i++)
    {
        atr_ints[i]->UI_Implement();
    }

    for (int i = 0; i < atr_floats.size(); i++)
    {
        atr_floats[i]->UI_Implement();
    }

    for (int i = 0; i < atr_vec3s.size(); i++)
    {
        std::string item = "vec##" + std::to_string(material->id) + std::to_string(i);
        ImGui::DragFloat3(item.c_str(), atr_vec3s[i], 0.1f);
    }

    for (int i = 0; i < atr_colors.size(); i++)
    {
        atr_colors[i]->UI_Implement();
    }
}

void ATR_Material::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["cur_id"] = cur_id;
    objectJson["cull_current"] = cull_current;

    nlohmann::json atr_vec3s_json = nlohmann::json::array();
    for (int i = 0; i < atr_vec3s.size(); i++) {
        nlohmann::json vec3_json;
        vec3_json["atr_vec3"] = { atr_vec3s[i][0], atr_vec3s[i][1], atr_vec3s[i][2]};
        atr_vec3s_json.push_back(vec3_json);
    }
    objectJson["atr_vec3s"] = atr_vec3s_json;

    nlohmann::json atr_textures_json = nlohmann::json::array();
    for (int i = 0; i < atr_textures.size(); i++) {
        nlohmann::json texture_json;
        atr_textures[i]->Save(texture_json);
        atr_textures_json.push_back(texture_json);
    }
    objectJson["atr_textures"] = atr_textures_json;

    nlohmann::json atr_ints_json = nlohmann::json::array();
    for (int i = 0; i < atr_ints.size(); i++) {
        nlohmann::json int_json;
        atr_ints[i]->Save(int_json);
        atr_ints_json.push_back(int_json);
    }
    objectJson["atr_ints"] = atr_ints_json;

    nlohmann::json atr_floats_json = nlohmann::json::array();
    for (int i = 0; i < atr_floats.size(); i++) {
        nlohmann::json float_json;
        atr_floats[i]->Save(float_json);
        atr_floats_json.push_back(float_json);
    }
    objectJson["atr_floats"] = atr_floats_json;

    nlohmann::json atr_colors_json = nlohmann::json::array();
    for (int i = 0; i < atr_colors.size(); i++) {
        nlohmann::json color_json;
        atr_colors[i]->Save(color_json);
        atr_colors_json.push_back(color_json);
    }
    objectJson["atr_colors"] = atr_colors_json;

    nlohmann::json material_json;
    material->Save(material_json);
    objectJson["material"] = material_json;
}

void ATR_Material::Load(const nlohmann::json& objectJson) {
    id = objectJson["id"];
    cur_id = objectJson["cur_id"];
    cull_current = objectJson["cull_current"];

    for (int i = 0; i < atr_ints.size(); i++) {
        atr_ints[i]->Load(objectJson["atr_ints"][i].get<nlohmann::json>());
    }

    for (int i = 0; i < atr_floats.size(); i++) {
        atr_floats[i]->Load(objectJson["atr_floats"][i].get<nlohmann::json>());
    }

    for (int i = 0; i < atr_colors.size(); i++) {
        atr_colors[i]->Load(objectJson["atr_colors"][i].get<nlohmann::json>());
    }

    for (int i = 0; i < atr_vec3s.size(); i++) {
        atr_vec3s[i][0] = objectJson["atr_vec3s"][i][0];
        atr_vec3s[i][1] = objectJson["atr_vec3s"][i][1];
        atr_vec3s[i][2] = objectJson["atr_vec3s"][i][2];
    }

    for (int i = 0; i < atr_textures.size(); i++) {
        atr_textures[i]->Load(objectJson["atr_textures"][i].get<nlohmann::json>());
    }

    material->Load(objectJson["material"]);
}

ATR_MeshRenderer::ATR_MeshRenderer(MeshRenderer *_meshRenderer) : meshRenderer(_meshRenderer)
{
    atr_material = new ATR_Material(_meshRenderer->material);
    id = cur_id++;
}

void ATR_MeshRenderer::UI_Implement()
{
    std::string title = "Mesh Renderer";
    std::string meshInfo = "vertices: null";
    if (meshRenderer->mesh != nullptr)
    {
        title = "Mesh Renderer##" + std::to_string(meshRenderer->mesh->VAO);
        meshInfo = "vertices: " + std::to_string(meshRenderer->mesh->vertices.size());
    }
    if (ImGui::CollapsingHeader(title.c_str(), true))
    {
        ImGui::Text(meshInfo.c_str());
        ImGui::SeparatorText("Setting");
        ImGui::Checkbox(("cast shadow##"+std::to_string(meshRenderer->mesh->VAO)).c_str(), &meshRenderer->cast_shadow);
        ImGui::SeparatorText("Material");

        const char* material_types[4] = {"Model", "PBR", "Unlit", "NPR"};
        std::string item = "material##" + std::to_string(id);
        ImGui::Combo(item.c_str(), &cur_mat, material_types, IM_ARRAYSIZE(material_types));
        if (prev_mat != cur_mat)
        {
            meshRenderer->SetMaterial((EMaterialType)cur_mat);
            delete atr_material;
            atr_material = new ATR_Material(meshRenderer->material);
        }
        prev_mat = cur_mat;

        atr_material->UI_Implement();
    }
}

void ATR_MeshRenderer::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["cur_id"] = cur_id;
    objectJson["prev_mat"] = prev_mat;
    objectJson["cur_mat"] = cur_mat;

    nlohmann::json atr_material_json;
    atr_material->Save(atr_material_json);
    objectJson["atr_material"] = atr_material_json;
}

void ATR_MeshRenderer::Load(const nlohmann::json& objectJson) {
    id = objectJson["id"];
    cur_id = objectJson["cur_id"];
    cur_mat = objectJson["cur_mat"];
    if (prev_mat != cur_mat) {
        meshRenderer->SetMaterial((EMaterialType)cur_mat);
        std::cout << cur_mat << std::endl;
        delete atr_material;
        atr_material = new ATR_Material(meshRenderer->material);
    }
    prev_mat = cur_mat;

    atr_material->Load(objectJson["atr_material"].get<nlohmann::json>());
}

ATR_LightRenderer::ATR_LightRenderer(float* _color)
{
    atr_light = new ATR_DirLight(_color);
    id = cur_id++;
}

void ATR_LightRenderer::UI_Implement()
{
    std::string title = "Light Setting";
    if (ImGui::CollapsingHeader(title.c_str(), true))
    {
        ImGui::SeparatorText("Light Type");

        const char* light_types[3] = { "Dir", "Point", "Spot" };
        std::string item = "Light Type##" + std::to_string(id);
        ImGui::Combo(item.c_str(), &cur_light, light_types, IM_ARRAYSIZE(light_types));
        if (prev_light != cur_light)
        {
            float* color = atr_light->color;
            if (cur_light == 1) {
                if (prev_light == 0) {
                    delete atr_light;
                    atr_light = new ATR_PointLight(color);
                }
                else if (prev_light == 2) {
                    float constant = atr_light->GetConstant();
                    float linear = atr_light->GetLinear();
                    float quadratic = atr_light->GetQuadratic();
                    delete atr_light;
                    atr_light = new ATR_PointLight(color, constant, linear, quadratic);
                }
            }
            else if (cur_light == 2) {
                if (prev_light == 0) {
                    delete atr_light;
                    atr_light = new ATR_SpotLight(color);
                }
                else if (prev_light == 1) { // 点光源
                    float constant = atr_light->GetConstant();
                    float linear = atr_light->GetLinear();
                    float quadratic = atr_light->GetQuadratic();
                    delete atr_light;
                    atr_light = new ATR_SpotLight(color, constant, linear, quadratic);
                }
            }
            else { // cur_light == 0
                delete atr_light;
                atr_light = new ATR_DirLight(color);
            }
        }
        prev_light = cur_light;

        atr_light->UI_Implement();
    }
}

void ATR_LightRenderer::Save(nlohmann::json& objectJson) {
    objectJson["id"] = id;
    objectJson["prev_light"] = prev_light;
    objectJson["cur_light"] = cur_light;
    objectJson["cur_id"] = cur_id;
    nlohmann::json atr_light_json;
    atr_light->Save(atr_light_json);
    objectJson["atr_light"] = atr_light_json;
}

void ATR_LightRenderer::Load(const nlohmann::json& objectJson, float* _color) {
    id = objectJson["id"];
    prev_light = objectJson["prev_light"];
    cur_light = objectJson["cur_light"];
    cur_id = objectJson["cur_id"];
    if (objectJson["atr_light"]["light_type"] == 0) { // 平行光
        delete atr_light;
        atr_light = new ATR_DirLight(_color);
    }
    else if (objectJson["atr_light"]["light_type"] == 1) { // 点光源
        delete atr_light;
        atr_light = new ATR_PointLight(_color);
    }
    else { // 聚光
        delete atr_light;
        atr_light = new ATR_SpotLight(_color);
    }
    atr_light->Load(objectJson["atr_light"]);
}

ATR_Light::ATR_Light(float* _color): color(_color) {}

void ATR_Light::UI_Implement()
{
    ImGui::ColorEdit4("Light Color", color);
}

void ATR_Light::Save(nlohmann::json& objectJson) {
    objectJson["light_type"] = light_type;
    objectJson["drag_speed"] = drag_speed;
    objectJson["color"] = { color[0], color[1], color[2] };
    objectJson["constant"] = GetConstant();
    objectJson["linear"] = GetLinear();
    objectJson["quadratic"] = GetQuadratic();
    objectJson["cut_off"] = GetCutOff();
    objectJson["outer_cut_off"] = GetOuterCutOff();
}
void ATR_Light::Load(const nlohmann::json& objectJson) {
    light_type = objectJson["light_type"];
    drag_speed = objectJson["drag_speed"];
}

ATR_DirLight::ATR_DirLight(float* _color) : ATR_Light(_color) { light_type = 0; }

void ATR_DirLight::UI_Implement() {
    ATR_Light::UI_Implement();
}

ATR_PointLight* ATR_DirLight::TransformPointLight() {
    return new ATR_PointLight(this->color);
}

ATR_SpotLight* ATR_DirLight::TransformSpotLight() {
    return new ATR_SpotLight(color);
}

ATR_PointLight::ATR_PointLight(float* _color) : ATR_Light(_color) {
    light_type = 1;
}

ATR_PointLight::ATR_PointLight(float* _color, float _constant, float _linear, float _quadratic): ATR_Light(_color) {
    light_type = 1;
    constant = _constant;
    linear = _linear;
    quadratic = _quadratic;
}

void ATR_PointLight::UI_Implement() {
    ATR_Light::UI_Implement();
    ImGui::SeparatorText("Parameters");
    ImGui::DragFloat("Constant", &constant, 0.05f, 1.0f, 3.0f);
    ImGui::DragFloat("Linear", &linear, 0.001f, 0.045f, 3.0f);
    ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.0075f, 3.0f);
}

void ATR_PointLight::Load(const nlohmann::json& objectJson) {
    ATR_Light::Load(objectJson);
    constant = objectJson["constant"];
    linear = objectJson["linear"];
    quadratic = objectJson["quadratic"];
}

ATR_DirLight* ATR_PointLight::TransformDirLight() {
    return new ATR_DirLight(color);
}

ATR_SpotLight* ATR_PointLight::TransformSpotLight() {
    return new ATR_SpotLight(color, constant, linear, quadratic);
}

ATR_SpotLight::ATR_SpotLight(float* _color) : ATR_Light(_color) {
    light_type = 2;
}

ATR_SpotLight::ATR_SpotLight(float* _color, float _constant, float _linear, float _quadratic) : ATR_Light(_color) {
    light_type = 2;

    cutOff = glm::cos(glm::radians(12.5f));
    outerCutOff = glm::cos(glm::radians(20.5f));

    constant = _constant;
    linear = _linear;
    quadratic = _quadratic;
}

void ATR_SpotLight::Load(const nlohmann::json& objectJson) {
    ATR_Light::Load(objectJson);
    constant = objectJson["constant"];
    linear = objectJson["linear"];
    quadratic = objectJson["quadratic"];
    cutOff = objectJson["cut_off"];
    outerCutOff = objectJson["outer_cut_off"];
}

ATR_DirLight* ATR_SpotLight::TransformDirLight() {
    return new ATR_DirLight(color);
}

ATR_PointLight* ATR_SpotLight::TransformPointLight() {
    return new ATR_PointLight(color, constant, linear, quadratic);
}

void ATR_SpotLight::UI_Implement() {
    ATR_Light::UI_Implement();
    ImGui::SeparatorText("Parameters");
    ImGui::DragFloat("Constant", &constant, 0.05f, 1.0f, 3.0f);
    ImGui::DragFloat("Linear", &linear, 0.001f, 0.045f, 3.0f);
    ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.0075f, 3.0f);

    static int spot_light_cutoff_angle = glm::degrees(glm::acos(cutOff));
    static int spot_light_outercutoff_angle = glm::degrees(glm::acos(outerCutOff));
    ImGui::DragInt("CutOff", &spot_light_cutoff_angle, 1.0f, 0.0f, spot_light_outercutoff_angle);
    cutOff = glm::cos(glm::radians((float)spot_light_cutoff_angle));
    ImGui::DragInt("OuterCutOff", &spot_light_outercutoff_angle, 1.0f, spot_light_cutoff_angle, 90.0f);
    outerCutOff = glm::cos(glm::radians((float)spot_light_outercutoff_angle));
}



ATR_PostProcessManager::ATR_PostProcessManager(PostProcessManager* manager) : ppm(manager) 
{
    RefreshAllNode();
}


ATR_PostProcessManager::~ATR_PostProcessManager() {}

void ATR_PostProcessManager::RefreshAllNode()
{
    atr_pps.clear();
    for (auto atr_p : ppm->postprocess_list)
    {
        atr_pps.push_back(atr_p->atr_ppn);
    }
}

void ATR_PostProcessManager::UI_Implement()
{
    ImGui::Text("post process manager");
    for (int i = 0; i < atr_pps.size(); i++)
    {
        atr_pps[i]->UI_Implement();
        if(ImGui::Button(("Move Up##" + std::to_string(atr_pps[i]->id)).c_str(), ImVec2(60,20)))
        {
            if (i > 0)
            {
                auto tmp = atr_pps[i-1];
                atr_pps[i-1] = atr_pps[i];
                atr_pps[i] = tmp;
                ppm->MoveUpPostProcessOnIndex(i);
            }
        }
        ImGui::SameLine();
        if(ImGui::Button(("Move Down##" + std::to_string(atr_pps[i]->id)).c_str(), ImVec2(80,20)))
        {
            if (i + 1 < atr_pps.size())
            {
                auto tmp = atr_pps[i+1];
                atr_pps[i+1] = atr_pps[i];
                atr_pps[i] = tmp;
                ppm->MoveDownPostProcessOnIndex(i);
            }
        }
    }
}

ATR_PostProcessNode::ATR_PostProcessNode(PostProcess* _postprocess) : postprocess(_postprocess)
{
    id = cur_id++;
}
ATR_PostProcessNode::~ATR_PostProcessNode() {}
void ATR_PostProcessNode::UI_Implement()
{
    ImGui::SeparatorText(postprocess->name.c_str());
    ImGui::Checkbox(("enabled##" + std::to_string(id)).c_str(), &postprocess->enabled);
}

ATR_BloomProcessNode::ATR_BloomProcessNode(PostProcess* _bloomprocess) : ATR_PostProcessNode(_bloomprocess) {}
ATR_BloomProcessNode::~ATR_BloomProcessNode() {}
void ATR_BloomProcessNode::UI_Implement()
{
    ATR_PostProcessNode::UI_Implement();
    ImGui::DragFloat("threshold", &dynamic_cast<BloomProcess*>(postprocess)->threshold, 0.05f);
    ImGui::DragFloat(("exposure##" + std::to_string(id)).c_str(), &dynamic_cast<BloomProcess*>(postprocess)->exposure, 0.05f);
}

ATR_HDRProcessNode::ATR_HDRProcessNode(PostProcess* _hdrprocess) : ATR_PostProcessNode(_hdrprocess) {}
ATR_HDRProcessNode::~ATR_HDRProcessNode() {}
void ATR_HDRProcessNode::UI_Implement()
{
    ATR_PostProcessNode::UI_Implement();
    ImGui::DragFloat(("exposure##" + std::to_string(id)).c_str(), &dynamic_cast<HDRProcess*>(postprocess)->exposure1, 0.05f);
}