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
    // 创建一个JSON对象
    nlohmann::json sceneJson;

    // 将场景对象列表添加到JSON中
    nlohmann::json sceneObjectsJson = nlohmann::json::array();
    for (auto& obj : scene_object_list)
    {
        nlohmann::json objectJson;
        obj->Save(objectJson);

        // 如果有子对象，递归地添加它们
        if (!obj->children.empty())
        {
            nlohmann::json childrenJson = nlohmann::json::array();
            for (auto& child : obj->children)
            {
                // 递归调用，这里只是示意，具体实现需要根据实际情况编写
                // childrenJson.push_back(/* child的JSON表示 */);
            }
            objectJson["children"] = childrenJson;
        }

        sceneObjectsJson.push_back(objectJson);
    }
    sceneJson["scene_objects"] = sceneObjectsJson;

    // 添加渲染设置的状态
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

    // 添加渲染管线的状态
    nlohmann::json renderPipelineJson;
    //renderPipelineJson["shadow_map_size"] = render_pipeline.shadow_map_setting.shadow_map_size;
    //renderPipelineJson["shadow_distance"] = render_pipeline.shadow_map_setting.shadow_distance;
    // 添加更多的渲染管线状态...

    //sceneJson["render_pipeline"] = renderPipelineJson;


    // 保存模型
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

    // 保存贴图
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


    // 将JSON对象保存到文件
    // std::ofstream file(FileSystem::FileSystem::GetContentPath() / "Scene/Scene.scene");
    std::ofstream file(_path);
    file << sceneJson.dump(4); // 缩进为4
    file.close();
}

void Scene::LoadScene(const std::string _path) {
    // 从文件中读取JSON数据
    std::ifstream file(_path);
    nlohmann::json sceneJson;
    file >> sceneJson;
    file.close();


    // 清除当前场景对象列表
    int list_size = scene_object_list.size();
    int flag = 0;
    for (int i = 0; i < list_size; i++) {
        if (i != 1 && i != 0) {
            RemoveSceneObjectAtIndex(i-flag);
            flag++;
        }
    }

    std::cout << scene_object_list.size() << std::endl;

    // 重建模型
    for (auto& objectJson : sceneJson["models"])
    {
        auto mmp = Model::LoadedModel;
        auto it = mmp.find(objectJson["name"]);
        if (it == mmp.end()) {
            new Model(objectJson["directory"].get<std::string>() + "/" + objectJson["name"].get<std::string>());
        }
    }

    // 重建贴图
    for (auto& objectJson : sceneJson["textures"])
    {
        auto textures = Texture2D::LoadedTextures;
        auto it = textures.find(objectJson["name"]);
        if (it == textures.end()) {
            new Texture2D(objectJson["path"].get<std::string>(), (ETexType)objectJson["tex_type"], (bool)objectJson["is_editor"]);
        }
    }

    // 从JSON中重建场景对象列表
    for (auto& objectJson : sceneJson["scene_objects"])
    {
        // 根据对象类型创建对象实例
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
        // 重建更多的属性...

        // 如果有子对象，递归地重建它们
        if (objectJson.contains("children"))
        {
            // 递归调用，这里只是示意，具体实现需要根据实际情况编写
            for (auto& childJson : objectJson["children"])
            {
                // 递归重建子对象...
            }
        }
    }

    // 重建渲染管线的状态
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
    // 重建更多的渲染管线状态...

    // 重建其他必要的场景状态...
}