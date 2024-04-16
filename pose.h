#pragma once


#include <vector>
#include "transform.h"

class Pose { // 某一时刻骨骼的状态
protected:
	std::vector<Transform> mJoints; // 每个关节的变换
	std::vector<int> mParents; // 每个关节对应的父节点

public:
	Pose();
	Pose(unsigned int numJoints);
	Pose(const Pose& p);
	Pose& operator=(const Pose& p);
	void Resize(unsigned int size); // 会同时修改两个容器中的大小
	unsigned int Size();
	int GetParent(unsigned int index);
	void SetParent(unsigned int index, int parent);
	Transform GetLocalTransform(unsigned int index); // 获得该关节的变换
	void SetLocalTransform(unsigned int index, const Transform& transform);
	Transform GetGlobalTransform(unsigned int index); // 获得所有变换对该关节造成的整体变换
	Transform operator[](unsigned int index); // 效果同 GetGlobalTransform
	void GetMatrixPalette(std::vector<glm::mat4>& out); // 用于将Transform转为矩阵以便传给OpenGL

	bool operator==(const Pose& other);
	bool operator!=(const Pose& other);
};