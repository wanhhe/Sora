#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>
#include "mat4.h"
#include "quat.h"

#define EPSILON 0.000001f


class Transform
{
public:
    Transform(glm::vec3 pos = glm::vec3(0,0,0), glm::vec3 rot = glm::vec3(0,0,0), glm::vec3 scal = glm::vec3(1,1,1)) : position(pos), scale(scal)
    {
        rotation = glm::quat(rot);
        UpdateVectors();
    }

    Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scal) : position(pos), rotation(rot), scale(scal)
    {
        rotation = glm::quat(rot);
        UpdateVectors();
    }

    ~Transform() {}

    Transform& operator=(const Transform& rhs)
    {
        this->position = rhs.position;
        this->rotation = rhs.rotation;
        this->scale = rhs.scale;
        this->UpdateVectors();
        return *this;
    }

    void SetPosition(float x, float y, float z)
    {
        position.r = x;
        position.g = y;
        position.b = z;
    }

    void SetPosition(glm::vec3 pos)
    {
        position = pos;
    }

    void SetRotation(float Pitch, float Yaw, float Roll)
    {
        glm::vec3 eularAngles(glm::radians(Pitch), glm::radians(Yaw), glm::radians(Roll));
        rotation = glm::quat(eularAngles);
        UpdateVectors();
    }

    void SetRotation(glm::quat rot)
    {
        rotation = rot;
        UpdateVectors();
    }

    void SetScale(float x, float y, float z)
    {
        scale.r = x;
        scale.g = y;
        scale.b = z;
    }

    void SetScale(glm::vec3 sca)
    {
        scale = sca;
    }

    const glm::vec3 Position()
    {
        return position;
    }

    const glm::vec3 Scale()
    {
        return scale;
    }

    const glm::vec3 Rotation()
    {
        return glm::degrees(glm::eulerAngles(rotation));
    }

    const glm::vec3 GetFront()
    {
        UpdateVectors();
        return Front;
    }

    const glm::vec3 GetRight()
    {
        UpdateVectors();
        return Right;
    }

    const glm::vec3 GetUp()
    {
        UpdateVectors();
        return Up;
    }

    void UpdateVectors()
    {
        // calculate the new Front vector
        glm::vec4 front(0,0,1,0);
        glm::vec4 up(0,1,0,0);
        glm::mat4 rot = glm::mat4_cast(rotation);
        Front = rot * front;
        Up = rot * up;
        Right = glm::normalize(glm::cross(Front, Up));
    }

    glm::mat4 GetTransformMatrix()
    {
        quat quat_trans(rotation.x, rotation.y, rotation.z, rotation.w);
        vec3 x = quat_trans * vec3(1, 0, 0);
        vec3 y = quat_trans * vec3(0, 1, 0);
        vec3 z = quat_trans * vec3(0, 0, 1);

        // Next, scale the basis vectors
        x = x * scale.x;
        y = y * scale.y;
        z = z * scale.z;

        // Create matrix
        return glm::mat4(
            x.x, x.y, x.z, 0, // X basis (& Scale)
            y.x, y.y, y.z, 0, // Y basis (& scale)
            z.x, z.y, z.z, 0, // Z basis (& scale)
            position.x, position.y, position.z, 1  // Position
        );
    }

    static Transform mat4ToTransform(const glm::mat4& m)
    {
        //Transform out;

        //// 提取位置信息
        //out.position = glm::vec3(m[3]); // m的第四列是位置信息

        //// 分解变换矩阵以获取变换组件
        //glm::quat rotationQuat;
        //glm::vec3 skew;
        //glm::vec4 perspective;

        //glm::decompose(m, out.scale, rotationQuat, out.position, skew, perspective);

        //// 将四元数转换为欧拉角
        //out.rotation = glm::eulerAngles(rotationQuat);
        //out.rotation = glm::degrees(out.rotation); // 将弧度转换为度

        //return out;

        Transform out;
        out.position = glm::vec3(m[3]); //  矩阵的最后一列是偏移量
        // out.rotation = glm::quat_cast(m);

        mat4 trans(
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]
            );
        
        quat quat_trans = mat4ToQuat(trans);
        out.rotation = glm::quat(quat_trans.w, quat_trans.x, quat_trans.y, quat_trans.z);

        mat4 rotScaleMat(
            trans.v[0], trans.v[1], trans.v[2], 0,
            trans.v[4], trans.v[5], trans.v[6], 0,
            trans.v[8], trans.v[9], trans.v[10], 0,
            0, 0, 0, 1
        );
        mat4 invRotMat = quatToMat4(::inverse(quat_trans));
        mat4 scaleSkewMat = rotScaleMat * invRotMat;

        out.scale = glm::vec3(
            scaleSkewMat.v[0],
            scaleSkewMat.v[5],
            scaleSkewMat.v[10]
        );

        //glm::mat4 rotScaleMat = glm::mat4(
        //    m[0][0], m[0][1], m[0][2], 0,
        //    m[1][0], m[1][1], m[1][2], 0,
        //    m[2][0], m[2][1], m[2][2], 0,
        //    0,0,0,1
        //);
        //// 消去旋转
        //glm::mat4 invRotMat = glm::mat4_cast(glm::inverse(out.rotation));
        //glm::mat4 scaleSkewMat = rotScaleMat * invRotMat;
        // 对角线是缩放信息
        //out.scale = glm::vec3(
        //    scaleSkewMat[0][0],
        //    scaleSkewMat[1][1],
        //    scaleSkewMat[2][2]
        //);
        //glm::vec3 skew;
        //glm::vec4 perspective;
        //glm::decompose(m, out.scale, out.rotation, out.position, skew, perspective);
        
        // out.rotation = glm::conjugate(out.rotation);
        //std::cout << "ii" << std::endl;
        // std::cout << out.position.x << " " << out.position.y << " " << out.position.z << std::endl;
        // std::cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << std::endl;
        return out;
    }

    static Transform combine(const Transform& a, const Transform& b) {
    //    //Transform out;

    //    //out.scale = a.scale * b.scale;
    //    //out.rotation = b.rotation * a.rotation;

    //    //out.position = a.rotation * (a.scale * b.position);
    //    //out.position = a.position + out.position;

    //    //return out;


    //    Transform out;
    //    out.scale = this->scale * b.scale;

    //    // 创建基于欧拉角的旋转矩阵
    //    glm::mat4 rotA = glm::eulerAngleYXZ(glm::radians(this->rotation.y), glm::radians(this->rotation.x), glm::radians(this->rotation.z));
    //    glm::mat4 rotB = glm::eulerAngleYXZ(glm::radians(b.rotation.y), glm::radians(b.rotation.x), glm::radians(b.rotation.z));

    //    // 将旋转矩阵相乘来组合旋转
    //    glm::mat4 combinedRot = rotB * rotA;

    //    // 将组合后的旋转矩阵转换为四元数
    //    glm::quat combinedQuat = glm::quat_cast(combinedRot);

    //    // 将四元数转换回欧拉角
    //    glm::vec3 combinedEulerAngles = glm::eulerAngles(combinedQuat);
    //    combinedEulerAngles = glm::degrees(combinedEulerAngles);

    //    out.rotation = combinedEulerAngles;

    //    // 应用旋转和缩放到位置
    //    out.position = glm::vec3(rotA * glm::vec4(this->scale * b.position, 1.0f));
    //    out.position += this->position;

    //    return out;


        Transform out;
        
        out.scale = a.scale * b.scale;
        
        //out.rotation = glm::quat(
        //    -a.rotation.x * b.rotation.x - a.rotation.y * b.rotation.y - a.rotation.z * b.rotation.z + a.rotation.w * b.rotation.w,
        //    a.rotation.x * b.rotation.w + a.rotation.y * b.rotation.z - a.rotation.z * b.rotation.y + a.rotation.w * b.rotation.x,
        //    -a.rotation.x * b.rotation.z + a.rotation.y * b.rotation.w + a.rotation.z * b.rotation.x + a.rotation.w * b.rotation.y,
        //    a.rotation.x * b.rotation.y - a.rotation.y * b.rotation.x + a.rotation.z * b.rotation.w + a.rotation.w * b.rotation.z
        //);
        // out.rotation = b.rotation * a.rotation;
        
        quat quat_trans_a(a.rotation.x, a.rotation.y, a.rotation.z, a.rotation.w);
        quat quat_trans_b(b.rotation.x, b.rotation.y, b.rotation.z, b.rotation.w);
        quat quat_trans_res = quat_trans_b * quat_trans_a;
        // out.rotation = a.rotation * b.rotation;
        out.rotation = glm::quat(quat_trans_res.w, quat_trans_res.x, quat_trans_res.y, quat_trans_res.z);
        // std::cout << out.rotation.x << " " << out.rotation.y << " " << out.rotation.z << " " << out.rotation.w << std::endl;

        out.position = a.rotation * (a.scale * b.position);
        
        out.position = a.position + out.position;
        // std::cout << out.position.x << " " << out.position.y << " " << out.position.z << std::endl;
        return out;
    }

    bool operator==(const Transform& b) {
        return this->position == b.position &&
            this->rotation == b.rotation &&
            this->scale == b.scale;
    }

    bool operator!=(const Transform& b) {
        return !(*this == b);
    }

    static Transform inverse(const Transform& t) {
    //    Transform inv;
    //    inv.rotation = -this->rotation;
    //    inv.scale.x = fabs(this->scale.x) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.x;
    //    inv.scale.y = fabs(this->scale.y) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.y;
    //    inv.scale.z = fabs(this->scale.z) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.z;

    //    glm::vec3 invTrans = this->position * -1.0f;
    //    inv.position = inv.rotation * (inv.scale * invTrans);

    //    return inv;

        Transform inv;
        //quat quat_trans(t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w);
        //quat_trans = ::inverse(quat_trans);
        //inv.rotation = glm::quat(quat_trans.w, quat_trans.x, quat_trans.y, quat_trans.z);
        inv.rotation = glm::inverse(t.rotation);
        inv.scale.x = fabs(t.scale.x) < EPSILON ? 0.0f : 1.0f / t.scale.x;
        inv.scale.y = fabs(t.scale.y) < EPSILON ? 0.0f : 1.0f / t.scale.y;
        inv.scale.z = fabs(t.scale.z) < EPSILON ? 0.0f : 1.0f / t.scale.z;

        glm::vec3 invTrans = t.position * -1.0f;
        inv.position = inv.rotation * (inv.scale * invTrans);

        return inv;
    }


    static Transform mix(const Transform& a, const Transform& b, float t) {
        // 四元数的插值要解决neibourhood问题
        glm::quat bRot = b.rotation;
        if (glm::dot(a.rotation, bRot) < 0.0f) {
            bRot = -bRot;
        }

        return Transform(
            glm::lerp(a.position, b.position, t),
            glm::slerp(a.rotation, bRot, t),
            glm::lerp(a.scale, b.scale, t)
        );
    }

    static glm::vec3 transformPoint(const Transform& a, const glm::vec3& b) {
        glm::vec3 out;
        out = a.rotation * (a.scale * b);
        out = a.position + out;
        return out;
    }
    static glm::vec3 transformVector(const Transform& a, const glm::vec3& b) {
        glm::vec3 out;
        out = a.rotation * (a.scale * b); //向量不存在偏移，所以不用加position
        return out;
    }

private:
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up;

    glm::vec3 position;
    glm::quat rotation;
    // glm::vec3 rotation; // Euler
    glm::vec3 scale;
    const glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

};