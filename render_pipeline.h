#pragma once

#include <map>
#include "renderer_window.h"

class SceneModel;
class SceneLight;
class Camera;
class Shader;
class DepthTexture;
class RenderTexture;
class SkyboxTexture;
class RendererWindow;
class PostProcessManager;

class RenderPipeline : public IOnWindowSizeChanged
{
public:
    RenderPipeline(RendererWindow* _window);
    ~RenderPipeline();
    void EnqueueRenderQueue(SceneModel *model);
    void RemoveFromRenderQueue(SceneModel *model);
    void RemoveFromRenderQueue(unsigned int id);
    SceneModel* GetRenderModel(unsigned int id);
    void Render();
    void OnWindowSizeChanged(int width, int height) override;

    struct ShadowMapSetting
    {
        float shadow_map_size = 4096;
        float shadow_distance = 50;
    } shadow_map_setting;

    struct CubeMapSetting
    {
        float cube_map_width = 1024;
        float cube_map_height = 1024;
    } cube_map_setting;

    float *clear_color;
    SceneLight* global_light;
    std::vector<SceneLight*> lights;
    PostProcessManager *postprocess_manager = nullptr;
    static RenderTexture* normal_texture;
    static RenderTexture* fragpos_texture;
    static DepthTexture* depth_texture;
    static DepthTexture* shadow_map;
    static SkyboxTexture* skybox_cubemap;

private:
    std::map<unsigned int, SceneModel *> ModelQueueForRender;
    RendererWindow *window;
    Shader* skybox_shader;
    Shader* depth_shader;
    Shader* grid_shader;
    Shader* normal_shader;
    Shader* fragpos_shader;
    Shader* cubemap_shader;

    void ProcessCubeMapPass();
    void ProcessZPrePass();
    void ProcessFragposPass();
    void ProcessShadowPass();
    void ProcessNormalPass();
    void ProcessColorPass();
    void RenderGizmos();
    void RenderSkybox();

};