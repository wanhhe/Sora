#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "mesh.h"
#include "shader.h"
#include "skeleton.h"
#include "clip.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include "editor_resource.h"
using namespace std;

class SceneModel;

struct AnimationInstance {
    Pose mAnimatedPose;
    std::vector<mat4> mPosePalette;
    unsigned int mClip;
    float mPlayback;
    Transform mModel;

    inline AnimationInstance() : mClip(0), mPlayback(0.0f) {}
};

class Model
{
public:
    // model data 
    vector<Texture2D*> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh*>    meshes;
    string directory;
    bool gammaCorrection;
    string name;
    static map<string, Model*> LoadedModel;
    EditorResource<SceneModel*> refSceneModels;

    // constructor, expects a filepath to a 3D model.
    Model(string const &path, bool gamma = false);
    Model(std::filesystem::path path, bool gamma = false);
    ~Model();
    
private:
    Skeleton mSkeleton; // 模型的骨骼
    std::vector<FastClip> mClips;
    AnimationInstance mAnimInfo; // 动画信息

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(const aiScene* scene, string const& path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene);

    Mesh* processMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture2D*> loadMaterialTextures(aiMaterial *mat, aiTextureType type);

    void loadSkeleton(const aiScene* scene);
    std::vector<std::string> loadJointNames(const aiScene* scene);
    std::vector<Clip> loadAnimationClips(const aiScene* scene);
    Pose loadRestPose(const aiScene* scene);
    Pose loadBindPose(const aiScene* scene);
    void loadAnimation(const aiScene* scene);

};