#pragma once
#include "imgui.h"
#include <filesystem>
#include "file_system.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class RendererWindow;
class SceneObject;
class Scene;

class renderer_ui
{
public:
    renderer_ui();
    ~renderer_ui();
    void setup(GLFWwindow *window);
    void RenderAll(RendererWindow *window, Scene *scene);
    void mainUI(RendererWindow *window, Scene* scene);
    void resourceUI(RendererWindow *window, Scene *scene);
    void sceneUI(RendererWindow *window, Scene* scene);
    void detailUI(RendererWindow *window, Scene *scene);
    void ImportModelPanel(RendererWindow *window);
    void ImportShaderPanel(RendererWindow *window);
    void ImportTexturePanel(RendererWindow *window);
    void SavePanel(RendererWindow* window, Scene* scene);
    void LoadPanel(RendererWindow* window, Scene* scene);
    void FileBrowser(RendererWindow *window, std::filesystem::path *_path);
    void shutdown();
    static bool isFocusOnUI();

private:
    SceneObject *selected;
    bool showImportModelPanel = false;
    bool showImportShaderPanel = false;
    bool showImportTexturePanel = false;
    bool showSavePanel = false;
    bool showLoadPanel = false;
    bool showFileBrowser = false;
    bool showConsole = false;
    std::filesystem::path *file_path;
    std::filesystem::path import_tex_path = FileSystem::GetContentPath();
    std::filesystem::path import_model_path = FileSystem::GetContentPath();
    std::filesystem::path import_shader_path = FileSystem::GetContentPath();
    std::filesystem::path scene_path = FileSystem::GetContentPath();
};