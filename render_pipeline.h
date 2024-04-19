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
class IrradianceTexture;
class SignleCubeMapTexture;
class OtherFrameBufferAndRenderBufferTexture2D;
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

    struct SkyboxCubeMapSetting
    {
        float skybox_cube_map_width = 2048;
        float skybox_cube_map_height = 2048;
    } skybox_cube_map_setting;

    struct IrradianceCubeMapSetting
    {
        float irradiance_cube_map_width = 64;
        float irraidance_cube_map_height = 64;
    } irradiance_cube_map_setting;

    struct PrefilterCubeMapSetting
    {
        float prefilter_cube_map_width = 256;
        float prefilter_cube_map_height = 256;
    } prefilter_cube_map_setting;

    float *clear_color;
    SceneLight* global_light;
    std::vector<SceneLight*> lights;
    PostProcessManager *postprocess_manager = nullptr;
    static RenderTexture* normal_texture;
    static RenderTexture* fragpos_texture;
    static DepthTexture* depth_texture;
    static DepthTexture* shadow_map;
    static SkyboxTexture* skybox_cubemap;
    static IrradianceTexture* irradiance_cubemap;
    static SignleCubeMapTexture* prefilter_cubemap;
    static OtherFrameBufferAndRenderBufferTexture2D* brdf_lut_texture;

private:
    std::map<unsigned int, SceneModel *> ModelQueueForRender;
    RendererWindow *window;
    Shader* skybox_shader;
    Shader* depth_shader;
    Shader* grid_shader;
    Shader* normal_shader;
    Shader* fragpos_shader;
    Shader* cubemap_shader;
    Shader* irradiance_convolution_shader;
    Shader* prefilter_shader;
    Shader* brdf_shader;

    void ProcessCubeMapPass();
    void ProcessIrradianceCubemap();
    void ProcessZPrePass();
    void ProcessFragposPass();
    void ProcessShadowPass();
    void ProcessNormalPass();
    void ProcessColorPass();
    void ProcessSpecularIBLPass();
    void RenderGizmos();
    void RenderSkybox();
    void RenderCube();
    void RenderQuad();

    int GetPointLightNum();
    int GetSpotLightNum();
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[6] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
};