#pragma once


#include <vector>
#include "transform.h"

class Pose { // ĳһʱ�̹�����״̬
protected:
	std::vector<Transform> mJoints; // ÿ���ؽڵı任
	std::vector<int> mParents; // ÿ���ؽڶ�Ӧ�ĸ��ڵ�

public:
	Pose();
	Pose(unsigned int numJoints);
	Pose(const Pose& p);
	Pose& operator=(const Pose& p);
	void Resize(unsigned int size); // ��ͬʱ�޸����������еĴ�С
	unsigned int Size();
	int GetParent(unsigned int index);
	void SetParent(unsigned int index, int parent);
	Transform GetLocalTransform(unsigned int index); // ��øùؽڵı任
	void SetLocalTransform(unsigned int index, const Transform& transform);
	Transform GetGlobalTransform(unsigned int index); // ������б任�Ըùؽ���ɵ�����任
	Transform operator[](unsigned int index); // Ч��ͬ GetGlobalTransform
	void GetMatrixPalette(std::vector<glm::mat4>& out); // ���ڽ�TransformתΪ�����Ա㴫��OpenGL

	bool operator==(const Pose& other);
	bool operator!=(const Pose& other);
};