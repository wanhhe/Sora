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

    //// �����Ⱦ���ߵ�״̬
    //nlohmann::json renderPipelineJson;
    //renderPipelineJson["shadow_map_size"] = render_pipeline.shadow_map_setting.shadow_map_size;
    //renderPipelineJson["shadow_distance"] = render_pipeline.shadow_map_setting.shadow_distance;
    //// ��Ӹ������Ⱦ����״̬...

    //sceneJson["render_pipeline"] = renderPipelineJson;

    // ��JSON���󱣴浽�ļ�
    // std::ofstream file(FileSystem::FileSystem::GetContentPath() / "Scene/Scene.scene");
    std::ofstream file(_path);
    file << sceneJson.dump(4); // ��Ư���ĸ�ʽ���������Ϊ4
    file.close();
}

void Scene::LoadScene(const std::string _path) {
    // ���ļ��ж�ȡJSON����
    std::ifstream file(_path);
    nlohmann::json sceneJson;
    file >> sceneJson;
    file.close();

    // �����ǰ���������б�
    scene_object_list.clear();

    // ��JSON���ؽ����������б�
    for (auto& objectJson : sceneJson["scene_objects"])
    {
        // ���ݶ������ʹ�������ʵ��
        SceneObject* obj;
        if (objectJson.contains("light")) {
            obj = new SceneLight(objectJson["name"], objectJson["is_editor"]);
            obj->Load(objectJson);
        }
        else {
            obj = new SceneObject();
            obj->Load(objectJson);
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

        // ��������ӵ����������б�
        scene_object_list.push_back(obj);
    }

    // �ؽ���Ⱦ���ߵ�״̬
    //if (sceneJson.contains("render_pipeline"))
    //{
    //    json renderPipelineJson = sceneJson["render_pipeline"];
    //    render_pipeline.shadow_map_setting.shadow_map_size = renderPipelineJson["shadow_map_size"];
    //    render_pipeline.shadow_map_setting.shadow_distance = renderPipelineJson["shadow_distance"];
    //    // �ؽ��������Ⱦ����״̬...
    //}

    // �ؽ�������Ҫ�ĳ���״̬...
}