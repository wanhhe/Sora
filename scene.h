#pragma once
#include <vector>
#include <string>
#include "render_pipeline.h"
class SceneLight;
class SceneObject;
class RendererWindow;
class Shader;
class Camera;
class Model;
class Scene
{
public:
    std::vector<SceneObject*>  scene_object_list;
    RenderPipeline render_pipeline;

public:
    Scene(RendererWindow* window);
    ~Scene();
    void RegisterSceneObject(SceneObject* object);
    void RegisterGlobalLight(SceneLight* light);
    void RegisterOtherLight(SceneLight* light);
    void InstanceFromModel(Model *model, std::string name);
    void RemoveSceneObjectAtIndex(int index);
    void RenderScene();

    void SaveScene(const std::string _path);
    void LoadScene(const std::string _path);

    RendererWindow *window;
};