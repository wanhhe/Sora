#include "renderer_window.h"
#include "scene_object.h"
#include "camera.h"
#include "model.h"
#include "scene.h"
#include "shader.h"

Scene::Scene(RendererWindow *_window) : window(_window), render_pipeline(RenderPipeline(_window))
{
    // Create a default light
    RegisterGlobalLight(new SceneLight("Global Light", true));
}

Scene::~Scene() {}
void Scene::RegisterSceneObject(SceneObject *object) { scene_object_list.push_back(object); }
void Scene::RenderScene() { render_pipeline.Render(); }

void Scene::RegisterGlobalLight( SceneLight *light)
{
    RegisterSceneObject(light);
    render_pipeline.global_light = light;
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

    //// 添加渲染管线的状态
    //nlohmann::json renderPipelineJson;
    //renderPipelineJson["shadow_map_size"] = render_pipeline.shadow_map_setting.shadow_map_size;
    //renderPipelineJson["shadow_distance"] = render_pipeline.shadow_map_setting.shadow_distance;
    //// 添加更多的渲染管线状态...

    //sceneJson["render_pipeline"] = renderPipelineJson;

    // 将JSON对象保存到文件
    // std::ofstream file(FileSystem::FileSystem::GetContentPath() / "Scene/Scene.scene");
    std::ofstream file(_path);
    file << sceneJson.dump(4); // 以漂亮的格式输出，缩进为4
    file.close();
}

void Scene::LoadScene(const std::string _path) {
    // 从文件中读取JSON数据
    std::ifstream file(_path);
    nlohmann::json sceneJson;
    file >> sceneJson;
    file.close();

    // 清除当前场景对象列表
    scene_object_list.clear();

    // 从JSON中重建场景对象列表
    for (auto& objectJson : sceneJson["scene_objects"])
    {
        // 根据对象类型创建对象实例
        SceneObject* obj;
        if (objectJson.contains("light")) {
            obj = new SceneLight(objectJson["name"], objectJson["is_editor"]);
            obj->Load(objectJson);
        }
        else {
            obj = new SceneObject();
            obj->Load(objectJson);
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

        // 将对象添加到场景对象列表
        scene_object_list.push_back(obj);
    }

    // 重建渲染管线的状态
    //if (sceneJson.contains("render_pipeline"))
    //{
    //    json renderPipelineJson = sceneJson["render_pipeline"];
    //    render_pipeline.shadow_map_setting.shadow_map_size = renderPipelineJson["shadow_map_size"];
    //    render_pipeline.shadow_map_setting.shadow_distance = renderPipelineJson["shadow_distance"];
    //    // 重建更多的渲染管线状态...
    //}

    // 重建其他必要的场景状态...
}