#include "renderer_window.h"
#include "scene_object.h"
#include "camera.h"
#include "model.h"
#include "scene.h"
#include "shader.h"
#include "postprocess.h"

Scene::Scene(RendererWindow *_window) : window(_window), render_pipeline(RenderPipeline(_window))
{
    // Create a default light
    RegisterGlobalLight(new SceneLight("Global Light", true));
}

Scene::~Scene() {}
void Scene::RegisterSceneObject(SceneObject *object) { scene_object_list.push_back(object); }
void Scene::RenderScene(float deltaTime) { render_pipeline.Render(deltaTime); }

void Scene::RegisterGlobalLight( SceneLight *light)
{
    RegisterSceneObject(light);
    render_pipeline.global_light = light;
}

void Scene::RegisterOtherLight(SceneLight* light) {
    RegisterSceneObject(light);
    render_pipeline.lights.push_back(light);
}

void Scene::InstanceFromModel(Model *model, std::string name)
{
    SceneModel *scene_model = new SceneModel(model, name);
    RegisterSceneObject(scene_model);
    render_pipeline.EnqueueRenderQueue(scene_model);
}

void Scene::RemoveSceneObjectAtIndex(int index)
{
    if (index >= scene_object_list.size())
    {
        return;
    }
    auto it = scene_object_list.begin() + index;
    SceneObject* target_so = *it;
    render_pipeline.RemoveFromRenderQueue(target_so->id);
    scene_object_list.erase(it);
    delete target_so;
}

void Scene::SaveScene(const std::string _path) {
    // ����һ��JSON����
    nlohmann::json sceneJson;

    // �����������б���ӵ�JSON��
    nlohmann::json sceneObjectsJson = nlohmann::json::array();
    for (auto& obj : scene_object_list)
    {
        nlohmann::json objectJson;
        obj->Save(objectJson);

        // ������Ӷ��󣬵ݹ���������
        if (!obj->children.empty())
        {
            nlohmann::json childrenJson = nlohmann::json::array();
            for (auto& child : obj->children)
            {
                // �ݹ���ã�����ֻ��ʾ�⣬����ʵ����Ҫ����ʵ�������д
                // childrenJson.push_back(/* child��JSON��ʾ */);
            }
            objectJson["children"] = childrenJson;
        }

        sceneObjectsJson.push_back(objectJson);
    }
    sceneJson["scene_objects"] = sceneObjectsJson;

    // �����Ⱦ���õ�״̬
    nlohmann::json editor_settings_json;
    editor_settings_json["use_polygon_mode"] = EditorSettings::UsePolygonMode;
    editor_settings_json["use_post_process"] = EditorSettings::UsePostProcess;
    editor_settings_json["draw_gizmos"] = EditorSettings::DrawGizmos;
    editor_settings_json["use_ibl"] = EditorSettings::UseIBL;
    editor_settings_json["use_skybox"] = EditorSettings::UseSkybox;
    if (EditorSettings::UseSkybox) {
        editor_settings_json["name"] = EditorSettings::SkyboxTexture->name;
    }
    if (EditorSettings::UseSkybox) {
        editor_settings_json["need_update_skybox"] = true;
    }
    else {
        editor_settings_json["need_update_skybox"] = false;
    }

    sceneJson["editor_settings_json"] = editor_settings_json;

    // �����Ⱦ���ߵ�״̬
    nlohmann::json renderPipelineJson;
    //renderPipelineJson["shadow_map_size"] = render_pipeline.shadow_map_setting.shadow_map_size;
    //renderPipelineJson["shadow_distance"] = render_pipeline.shadow_map_setting.shadow_distance;
    // ��Ӹ������Ⱦ����״̬...

    //sceneJson["render_pipeline"] = renderPipelineJson;


    // ����ģ��
    nlohmann::json model_json = nlohmann::json::array();
    auto mmp = Model::LoadedModel;
    for (std::map<string, Model*>::iterator it = mmp.begin(); it != mmp.end(); it++)
    {
        nlohmann::json objectJson;
        objectJson["name"] = it->first;
        objectJson["directory"] = it->second->directory;
        model_json.push_back(objectJson);
    }

    sceneJson["models"] = model_json;

    // ������ͼ
    nlohmann::json texture_json = nlohmann::json::array();
    auto textures = Texture2D::LoadedTextures;
    for (std::map<string, Texture2D*>::iterator it = textures.begin(); it != textures.end(); it++) {
        nlohmann::json objectJson;
        objectJson["id"] = it->second->id;
        objectJson["is_editor"] = it->second->is_editor;
        objectJson["name"] = it->first;
        objectJson["path"] = it->second->path;
        objectJson["tex_type"] = it->second->tex_type;

        texture_json.push_back(objectJson);
    }

    sceneJson["textures"] = texture_json;


    // ��JSON���󱣴浽�ļ�
    // std::ofstream file(FileSystem::FileSystem::GetContentPath() / "Scene/Scene.scene");
    std::ofstream file(_path);
    file << sceneJson.dump(4); // ����Ϊ4
    file.close();
}

void Scene::LoadScene(const std::string _path) {
    // ���ļ��ж�ȡJSON����
    std::ifstream file(_path);
    nlohmann::json sceneJson;
    file >> sceneJson;
    file.close();


    // �����ǰ���������б�
    int list_size = scene_object_list.size();
    int flag = 0;
    for (int i = 0; i < list_size; i++) {
        if (i != 1 && i != 0) {
            RemoveSceneObjectAtIndex(i-flag);
            flag++;
        }
    }

    std::cout << scene_object_list.size() << std::endl;

    // �ؽ�ģ��
    for (auto& objectJson : sceneJson["models"])
    {
        auto mmp = Model::LoadedModel;
        auto it = mmp.find(objectJson["name"]);
        if (it == mmp.end()) {
            new Model(objectJson["directory"].get<std::string>() + "/" + objectJson["name"].get<std::string>());
        }
    }

    // �ؽ���ͼ
    for (auto& objectJson : sceneJson["textures"])
    {
        auto textures = Texture2D::LoadedTextures;
        auto it = textures.find(objectJson["name"]);
        if (it == textures.end()) {
            new Texture2D(objectJson["path"].get<std::string>(), (ETexType)objectJson["tex_type"], (bool)objectJson["is_editor"]);
        }
    }

    // ��JSON���ؽ����������б�
    for (auto& objectJson : sceneJson["scene_objects"])
    {
        // ���ݶ������ʹ�������ʵ��
        if (objectJson.contains("light")) {
            if (objectJson["name"].get<std::string>() == "Global Light") {
                render_pipeline.global_light->Load(objectJson);
            }
            else {
                SceneLight* obj = new SceneLight(objectJson["name"].get<std::string>(), (bool)objectJson["is_editor"]);
                obj->Load(objectJson);
                RegisterOtherLight(obj);
            }
            
        }
        else if (objectJson.contains("model")) {
            auto mmp = Model::LoadedModel;
            for (std::map<string, Model*>::iterator it = mmp.begin(); it != mmp.end(); it++)
            {
                if (it->first == objectJson["model"]["name"].get<std::string>()) {
                    SceneModel* obj = new SceneModel(it->second, objectJson["name"].get<std::string>(), (bool)objectJson["is_editor"]);
                    obj->Load(objectJson);
                    RegisterSceneObject(obj);
                    render_pipeline.EnqueueRenderQueue(obj);
                }
            }
        }
        // �ؽ����������...

        // ������Ӷ��󣬵ݹ���ؽ�����
        if (objectJson.contains("children"))
        {
            // �ݹ���ã�����ֻ��ʾ�⣬����ʵ����Ҫ����ʵ�������д
            for (auto& childJson : objectJson["children"])
            {
                // �ݹ��ؽ��Ӷ���...
            }
        }
    }

    // �ؽ���Ⱦ���ߵ�״̬
    // nlohmann::json renderPipelineJson = sceneJson["render_pipeline"];
    nlohmann::json editor_settings_json = sceneJson["editor_settings_json"];
    EditorSettings::DrawGizmos = editor_settings_json["draw_gizmos"];
    EditorSettings::UsePolygonMode = editor_settings_json["use_polygon_mode"];
    EditorSettings::UsePostProcess = editor_settings_json["use_post_process"];
    EditorSettings::UseSkybox = editor_settings_json["use_skybox"];
    if (EditorSettings::UseSkybox) {
        if (EditorSettings::SkyboxTexture != nullptr) {
            delete EditorSettings::SkyboxTexture;
        }
        EditorSettings::SkyboxTexture = Texture2D::LoadedTextures[editor_settings_json["name"].get<std::string>()];
        EditorSettings::NeedUpdateSkybox = true;
        EditorSettings::UseIBL = editor_settings_json["use_ibl"];
    }
    else {
        EditorSettings::NeedUpdateSkybox = false;
        EditorSettings::UseIBL = false;
    }
    // �ؽ��������Ⱦ����״̬...

    // �ؽ�������Ҫ�ĳ���״̬...
}