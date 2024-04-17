#include "scene_object.h"
#include "model.h"
#include "renderer_console.h"
#include "transform.h"
#include "rearrange_bones.h"

map<string, Model*> Model::LoadedModel;

// constructor, expects a filepath to a 3D model.
Model::Model(string const& path, bool gamma) : gammaCorrection(gamma)           { 
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        RendererConsole::GetInstance()->AddError("[error] ASSIMP: %s", importer.GetErrorString());
        return;
    }
    loadModel(scene, path);
    loadSkeleton(scene);
    loadAnimation(scene);
}

Model::Model(std::filesystem::path path, bool gamma) : gammaCorrection(gamma)   { 
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        RendererConsole::GetInstance()->AddError("[error] ASSIMP: %s", importer.GetErrorString());
        return;
    }
    loadModel(scene, path.string().c_str());
    loadSkeleton(scene);
    loadAnimation(scene);
}

Model::~Model()
{
    RendererConsole::GetInstance()->AddLog("Delete Model: %s", directory); 
    for (auto it : refSceneModels.references)
    {
        it->OnModelRemoved();
    }
    LoadedModel.erase(name);
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::loadModel(const aiScene* scene, string const& path)
{
    // retrieve the directory path of the filepath
    string path_s = path;
    std::replace(path_s.begin(), path_s.end(), '\\', '/');
    directory = path_s.substr(0, path_s.find_last_of('/'));
    name = path_s.substr(path_s.find_last_of('/') + 1, path_s.size());
    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
    RendererConsole::GetInstance()->AddNote("Load Model From %s", path.c_str());
    LoadedModel.insert(map<string, Model*>::value_type(name, this));
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }

}

Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture2D*> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    vector<Texture2D*> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture2D*> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture2D*> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture2D*> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // return a mesh object created from the extracted mesh data
    return new Mesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<Texture2D*> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    vector<Texture2D*> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j]->path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            string filename = string(str.C_Str());
            filename = this->directory + '/' + filename;
            Texture2D* tex = new Texture2D(filename.c_str());
            tex->path = filename.c_str();
            textures.push_back(tex);
            textures_loaded.push_back(tex);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
    }
    return textures;
}

void Model::loadSkeleton(const aiScene* scene) {
    if (!scene->hasSkeletons()) return;
    mSkeleton = Skeleton(loadRestPose(scene), loadBindPose(scene), loadJointNames(scene));
    std::cout << "load success ths number is " << mSkeleton.GetJointNames().size() << std::endl;
}

std::vector<std::string> Model::loadJointNames(const aiScene* scene) {
    aiSkeleton* skeleton = scene->mSkeletons[0];
    unsigned int boneCount = skeleton->mNumBones;
    std::vector<std::string> result(boneCount, "Not Set");

    for (unsigned int i = 0; i < boneCount; i++) {
        aiSkeletonBone* bone = skeleton->mBones[i];

        if (bone->mNode->mName.C_Str() == "") { // 没有名字，指针为空
            result[i] = "EMPTY NODE";
        }
        else {
            result[i] = bone->mNode->mName.C_Str();
        }
    }

    return result;
}

std::vector<Clip> Model::loadAnimationClips(const aiScene* scene) {
    std::vector<Clip> result;
    //if (!scene->HasAnimations()) return result;
    //unsigned int numClips = scene->mNumAnimations;
    //// unsigned int numNodes = (unsigned int)data->nodes_count; // 动画中结点的个数

    //result.resize(numClips);

    //aiAnimation** animations = scene->mAnimations;

    //for (unsigned int i = 0; i < numClips; i++) {
    //    // 为每个片段设置名称
    //    result[i].SetName(animations[i]->mName.C_Str());

    //    unsigned int numChannels = animations[i]->mNumChannels;
    //    for (unsigned int j = 0; j < numChannels; j++) { // 通道定义了动画的目标节点和属性（如平移、旋转或缩放)
    //        aiNodeAnim* channel = animations[i]->mChannels[j];
    //        
    //        cgltf_node* target = channel.target_node;
    //        int nodeId = GLTFHelpers::GetNodeIndex(target, data->nodes, numNodes);
    //        // 根据类型决定采用TransformTrack中的哪个成员
    //        if (channel.target_path == cgltf_animation_path_type_translation) {
    //            VectorTrack& track = result[i][nodeId].GetPositionTrack();
    //            GLTFHelpers::TrackFromChannel<vec3, 3>(track, channel);
    //        }
    //        else if (channel.target_path == cgltf_animation_path_type_scale) {
    //            VectorTrack& track = result[i][nodeId].GetScaleTrack();
    //            GLTFHelpers::TrackFromChannel<vec3, 3>(track, channel);
    //        }
    //        else if (channel.target_path == cgltf_animation_path_type_rotation) {
    //            QuaternionTrack& track = result[i][nodeId].GetRotationTrack();
    //            GLTFHelpers::TrackFromChannel<quat, 4>(track, channel);
    //        }

    //        // 使用Assimp查找目标节点
    //        aiNode* target = scene->mRootNode->FindNode(channel->mNodeName);
    //        
    //        int nodeId = AssimpHelpers::GetNodeIndex(target, scene->mRootNode, numNodes);

    //        // 根据类型决定采用TransformTrack中的哪个成员
    //        if (channel->mScalingKeys) {
    //            VectorTrack& track = result[i][nodeId].GetScaleTrack();
    //            AssimpHelpers::TrackFromScalingKeys(track, channel);
    //        }
    //        if (channel->mPositionKeys) {
    //            VectorTrack& track = result[i][nodeId].GetPositionTrack();
    //            AssimpHelpers::TrackFromPositionKeys(track, channel);
    //        }
    //        if (channel->mRotationKeys) {
    //            QuaternionTrack& track = result[i][nodeId].GetRotationTrack();
    //            AssimpHelpers::TrackFromRotationKeys(track, channel);
    //        }
    //    }

    //    result[i].RecalculateDuration();
    //}

    return result;
}

Pose Model::loadRestPose(const aiScene* scene) {
    aiSkeleton* skeleton = scene->mSkeletons[0];
    unsigned int boneCount = skeleton->mNumBones;
    
    Pose result(boneCount);

    for (unsigned int i = 0; i < boneCount; i++) {
        aiSkeletonBone* bone = skeleton->mBones[i];
        // cgltf_node* node = &(data->nodes[i]);

        // 设置骨骼的变换
        aiMatrix4x4 localMatrix = bone->mLocalMatrix;
        glm::mat4 mat;
        // Assimp矩阵是行主序的，而GLM默认是列主序的
        mat[0][0] = localMatrix.a1; mat[1][0] = localMatrix.a2;
        mat[2][0] = localMatrix.a3; mat[3][0] = localMatrix.a4;
        mat[0][1] = localMatrix.b1; mat[1][1] = localMatrix.b2;
        mat[2][1] = localMatrix.b3; mat[3][1] = localMatrix.b4;
        mat[0][2] = localMatrix.c1; mat[1][2] = localMatrix.c2;
        mat[2][2] = localMatrix.c3; mat[3][2] = localMatrix.c4;
        mat[0][3] = localMatrix.d1; mat[1][3] = localMatrix.d2;
        mat[2][3] = localMatrix.d3; mat[3][3] = localMatrix.d4;
        Transform transform = Transform::mat4ToTransform(mat);
        result.SetLocalTransform(i, transform);

        // 设置其父节点
        result.SetParent(i, bone->mParent);
    }

    return result;
}

Pose Model::loadBindPose(const aiScene* scene) {
    // 先获得restPose作为默认值，避免该文件未设置bindPose而导致错误
    Pose restPose = loadRestPose(scene);
    //unsigned int numBones = restPose.Size();
    //std::vector<Transform> worldBindPose(numBones);
    //for (unsigned int i = 0; i < numBones; i++) {
    //    worldBindPose[i] = restPose.GetGlobalTransform(i); // 设置每个关节的默认值
    //}

    //unsigned int numSkins = data->skins_count;
    //for (unsigned int i = 0; i < numSkins; i++) { // 获得每个皮肤的逆绑定矩阵(全局矩阵)
    //    cgltf_skin* skin = &(data->skins[i]);
    //    std::vector<float> invBindAccessor;
    //    GLTFHelpers::GetScalarValues(invBindAccessor, 16, *skin->inverse_bind_matrices); // 一个元素由一个4*4的矩阵构成

    //    // 获得该蒙皮上每个关节的逆绑定矩阵
    //    unsigned int numJoints = skin->joints_count;
    //    for (unsigned int j = 0; j < numJoints; j++) {
    //        // 获得该关节的 inverse bind matrix
    //        float* matrix = &invBindAccessor[j * 16];
    //        mat4 invBindMatrix = mat4(matrix);
    //        // 获得绑定矩阵
    //        mat4 bindMatrix = inverse(invBindMatrix);
    //        Transform bindTransform = Transform::mat4ToTransform(bindMatrix);
    //        // 根据结点设置绑定变换
    //        cgltf_node* jointNode = skin->joints[j];
    //        int jointIndex = GLTFHelpers::GetNodeIndex(jointNode, data->nodes, numBones);
    //        worldBindPose[jointIndex] = bindTransform;
    //    }
    //}

    //// 将世界变换转为相对于父节点的局部变换
    //Pose bindPose = restPose;
    //for (unsigned int i = 0; i < numBones; i++) {
    //    Transform current = worldBindPose[i];
    //    int p = bindPose.GetParent(i);
    //    if (p >= 0) {
    //        Transform parent = worldBindPose[p];
    //        current = inverse(parent).combine(current); // 父节点的逆矩阵乘该结点的变换就可以取消父节点的变换
    //    }
    //    bindPose.SetLocalTransform(i, current);
    //}

    //return bindPose;



    //Pose restPose = loadRestPose(scene);
    //unsigned int numBones = restPose.Size();
    //std::vector<Transform> worldBindPose(numBones);

    //// 遍历所有的骨骼动画
    //for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    //    const aiMesh* mesh = scene->mMeshes[i];
    //    if (mesh->HasBones()) {
    //        for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
    //            const aiBone* bone = mesh->mBones[j];
    //            unsigned int boneIndex = restPose.GetBoneIndex(bone->mName.C_Str());
    //            if (boneIndex != -1) {
    //                aiMatrix4x4 bindMatrix = bone->mOffsetMatrix;
    //                Transform bindTransform = AssimpHelpers::ConvertMatrixToTransform(bindMatrix);
    //                worldBindPose[boneIndex] = bindTransform;
    //            }
    //        }
    //    }
    //}

    //// 将世界变换转换为局部变换
    //std::vector<Transform> localBindPose = convertWorldToLocal(worldBindPose);

    Pose bindPose = restPose;
    //for (unsigned int i = 0; i < numBones; ++i) {
    //    bindPose.SetLocalTransform(i, localBindPose[i]);
    //}

    return bindPose;
}

void Model::loadAnimation(const aiScene* scene) {
    //std::vector<Clip> clips = loadAnimationClips(scene);
    //mClips.resize(clips.size());
    //for (unsigned int i = 0; i < (unsigned int)clips.size(); i++) {
    //    mClips[i] = OptimizeClip(clips[i]);
    //}

    //// 调整骨骼顺序
    //BoneMap bones = RearrangeSkeleton(mSkeleton);
    //for (unsigned int i = 0; i < (unsigned int)meshes.size(); i++) {
    //    // 对每个mesh也要调整
    //    RearrangeMesh(meshes[i], bones);
    //}
    //for (unsigned int i = 0; i < mClips.size(); i++) {
    //    // 由于骨骼id变了，TransformTrack中的id也就变了，需要调整
    //    RearrangeFastClip(mClips[i], bones);
    //}

    //for (unsigned int i = 0, size = (unsigned int)mGPUMeshes.size(); i < size; ++i) {
    //    mGPUMeshes[i].UpdateOpenGLBuffers();
    //}

    //mStaticShader = new Shader("Shaders/static.vert", "Shaders/lit.frag");
    ////mSkinnedShader = new Shader("Shaders/skinned.vert", "Shaders/lit.frag");
    //mSkinnedShader = new Shader("Shaders/preskinned.vert", "Shaders/lit.frag");
    ////mSkinnedShader = new Shader("Shaders/static.vert", "Shaders/lit.frag");
    //mDiffuseTexture = new Texture("Assets/NRM_base.png");

    //mAnimInfo.mAnimatedPose = mSkeleton.GetRestPose();
    //mAnimInfo.mPosePalette.resize(mSkeleton.GetRestPose().Size());
    //mAnimInfo.mModel.position = vec3(-2, 0, 0);
    //mAnimInfo.mModel.scale = vec3(200, 200, 200);

    //std::cout << "pose: " << mSkeleton.GetRestPose().Size() << ";\n";

    //unsigned int numUIClips = (unsigned int)mClips.size();
    //for (unsigned int i = 0; i < numUIClips; i++) {
    //    if (mClips[i].GetName() == "Walking") {
    //        mCPUAnimInfo.mClip = i;
    //    }
    //    else if (mClips[i].GetName() == "Running") {
    //        mGPUAnimInfo.mClip = i;
    //    }
    //}
}