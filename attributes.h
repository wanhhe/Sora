#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <list>
#include "transform.h"

class MeshRenderer;
class SceneModel;
class Texture2D;
class Material;
class Shader;

/*****************************************************
* Attribute is a container to display content
* which is the feature of a SceneObject 
* and can be rendered on detail panel
*****************************************************/
class Attribute
{
public:
    virtual void UI_Implement() = 0;
    virtual ~Attribute() {};
};

class ATR_Transform : public Attribute
{
public:
    Transform* transform;

public:
    ATR_Transform();
    void UI_Implement() override;
 
    ~ATR_Transform()    override = default;
};

class MaterialTexture2D;
class ATR_MaterialTexture : Attribute
{
public:
    ATR_MaterialTexture( std::string _name, Material *_material, MaterialTexture2D *_texture );
    void UI_Implement()     override;
    ~ATR_MaterialTexture()  override = default;

    unsigned int id;
    std::string slot_name = "null";
    MaterialTexture2D *mat_tex;
    Material *material;


private:
    static unsigned int cur_id;
    float tilling[2];
    float offset[2];
};

class ATR_MaterialFloat : Attribute
{
public:
    ATR_MaterialFloat( std::string _name, Material *_material, float *_value );
    void UI_Implement()     override;
    ~ATR_MaterialFloat()    override = default;

    unsigned int id;
    std::string slot_name = "null";
    float drag_speed = 0.1;
    float *value;
    Material *material;

private:
    static unsigned int cur_id;
};

class ATR_MaterialInt : Attribute
{
public:
    ATR_MaterialInt( std::string _name, Material *_material, int *_value );
    void UI_Implement() override;
    ~ATR_MaterialInt()  override = default;

    unsigned int                    id;
    std::string                     slot_name = "null";
    float                           drag_speed = 1;
    int                             *value;
    Material                        *material;

private:
    static unsigned int             cur_id;
};

class ATR_MaterialColor : Attribute
{
public:
    ATR_MaterialColor(std::string _name, Material *_material, float *_value);
    void UI_Implement() override;
    ~ATR_MaterialColor() override = default;

    unsigned int                    id;
    std::string                     slot_name = "null";
    float                           drag_speed = 0.1;
    float                           *value;
    Material                        *material;

private:
    static unsigned int             cur_id;
};

class ATR_Material : public Attribute
{
public:
    ATR_Material(Material *_material);
    void UI_Implement()     override;
    ~ATR_Material()    override = default;

    Material *material;
    unsigned int id;

private:
    // attributes need to draw:
    std::vector<ATR_MaterialTexture*> atr_textures;
    std::vector<ATR_MaterialInt*> atr_ints;
    std::vector<ATR_MaterialFloat*> atr_floats;
    std::vector<float*> atr_vec3s;
    std::vector<ATR_MaterialColor *> atr_colors;
    int cull_current = 0;
    static unsigned int cur_id;
};

// render a mesh's all attributes
class ATR_MeshRenderer : public Attribute
{
public:
    ATR_MeshRenderer(MeshRenderer *_meshRenderer);
    void UI_Implement() override;
    ~ATR_MeshRenderer() override = default;


    MeshRenderer* meshRenderer;
    unsigned int id;

private:
    ATR_Material* atr_material;
    int prev_mat = 1;
    int cur_mat = 1;
    static unsigned int cur_id;
};

class ATR_Light;

class ATR_LightRenderer : public Attribute {
public:
    ATR_LightRenderer(float* _color);
    void UI_Implement() override;
    ~ATR_LightRenderer() override = default;

    unsigned int id;

private:
    ATR_Light* atr_light;
    int prev_light = 0; // 之前的灯光
    int cur_light = 0; // 现在的灯光
    static unsigned int cur_id;
};

class ATR_Light : Attribute
{
public:
    ATR_Light(float *_value);
    void UI_Implement() override;
    ~ATR_Light() override = default;
    virtual float GetConstant() { return 1; };
    virtual float GetLinear() { return 0.7; };
    virtual float GetQuadratic() { return 1.8; };

    int light_type = 0; // 0是dir，1是point，2是spot
    float drag_speed = 0.1;
    float* color;
};

class ATR_DirLight;
class ATR_PointLight;
class ATR_SpotLight;

class ATR_DirLight : public ATR_Light {
public:
    ATR_DirLight(float* _value);
    void UI_Implement() override;
    ~ATR_DirLight() override = default;

    ATR_PointLight* TransformPointLight();
    ATR_SpotLight* TransformSpotLight();
};

class ATR_PointLight : public ATR_Light {
public:
    ATR_PointLight(float* _value);
    ATR_PointLight(float* _value, float _constant, float _linear, float _quadratic);
    void UI_Implement() override;
    ~ATR_PointLight() override = default;

    ATR_DirLight* TransformDirLight();
    ATR_SpotLight* TransformSpotLight();
    float GetConstant() override { return constant; }
    float GetLinear() override { return linear; }
    float GetQuadratic() override { return quadratic; }

    float constant;
    float linear;
    float quadratic;
};

class ATR_SpotLight: public ATR_Light {
public:
    ATR_SpotLight(float* _value);
    ATR_SpotLight(float* _value, float _constant, float _linear, float _quadratic);
    void UI_Implement() override;
    ~ATR_SpotLight() override = default;

    ATR_DirLight* TransformDirLight();
    ATR_PointLight* TransformPointLight();
    float GetConstant() override { return constant; }
    float GetLinear() override { return linear; }
    float GetQuadratic() override { return quadratic; }

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;
};

class PostProcess;
class ATR_PostProcessNode : public Attribute
{
public:
    unsigned int id;
    ATR_PostProcessNode(PostProcess* _postprocess);
    void UI_Implement()         override;
    ~ATR_PostProcessNode()      override;

protected:
    static unsigned int cur_id;
    PostProcess* postprocess;
};

class ATR_BloomProcessNode : public ATR_PostProcessNode
{
public:
    ATR_BloomProcessNode(PostProcess* _postprocess);
    virtual void UI_Implement()         override;
    virtual ~ATR_BloomProcessNode()      override;
};

class PostProcessManager;
class ATR_PostProcessManager : public Attribute
{
public:
    ATR_PostProcessManager(PostProcessManager* manager);
    virtual void UI_Implement()         override;
    virtual ~ATR_PostProcessManager()   override;
    void RefreshAllNode();

private:
    PostProcessManager* ppm;
    std::vector<ATR_PostProcessNode*> atr_pps;
};

