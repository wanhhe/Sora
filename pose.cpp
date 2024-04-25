#include "pose.h"
#include <iostream>

Pose::Pose() {}

Pose::Pose(unsigned int numJoints) {
	Resize(numJoints);
}

Pose::Pose(const Pose& p) {
	*this = p;
}

Pose& Pose::operator=(const Pose& p) {
	if (&p == this) return *this;

	if (mParents.size() != p.mParents.size()) mParents.resize(p.mParents.size());
	if (mJoints.size() != p.mJoints.size()) mJoints.resize(p.mJoints.size());

	// 快速复制值
	if (mParents.size() != 0) memcpy(&mParents[0], &p.mParents[0], sizeof(int) * mParents.size());
	if (mJoints.size() != 0) memcpy(&mJoints[0], &p.mJoints[0], sizeof(Transform) * mJoints.size());

	return *this;
}

void Pose::Resize(unsigned int size) {
	mParents.resize(size);
	mJoints.resize(size);
}

unsigned int Pose::Size() {
	return mJoints.size();
}

Transform Pose::GetLocalTransform(unsigned int index) {
	return mJoints[index];
}

void Pose::SetLocalTransform(unsigned int index, const Transform& transform) {
	mJoints[index] = transform;
}

Transform Pose::GetGlobalTransform(unsigned int index) {
	Transform result = mJoints[index];

	//glm::mat4 mInvBindPose;
	//mInvBindPose = result.GetTransformMatrix();
	//std::cout << "ee" << std::endl;
	//std::cout << mInvBindPose[0][0] << " " << mInvBindPose[0][1] << " " << mInvBindPose[0][2] << " " << mInvBindPose[0][3] << std::endl;

	// 遍历到根节点为止
	for (int parent = mParents[index]; parent >= 0; parent = mParents[parent]) {
		result = Transform::combine(mJoints[parent], result);
	}

	return result;
}

Transform Pose::operator[](unsigned int index) {
	Transform result = mJoints[index];
	// 遍历到根节点为止
	for (int parent = mParents[index]; parent >= 0; parent = mParents[parent]) {
		result = Transform::combine(mJoints[parent], result);
	}

	return result;
}

#if 1
void Pose::GetMatrixPalette(std::vector<glm::mat4>& out) {
	int size = (int)Size();
	if ((int)out.size() != size) {
		out.resize(size);
	}

	int i = 0;
	for (; i < size; i++) {
		int parent = mParents[i];
		if (parent > i) { // 如果是无序的，退化成原始版本
			break;
		}

		glm::mat4 global = mJoints[i].GetTransformMatrix(); // 获得局部变换
		if (parent >= 0) {
			global = out[parent] * global; // 由于是从小到大的，out[parent]已经是被处理过的了
		}
		out[i] = global;

		//std::cout << "i" << std::endl;
		//glm::mat4 mInvBindPose;
		//std::cout << global[0][0] << " " << global[0][1] << " " << global[0][2] << " " << global[0][3] << std::endl;
	}

	for (; i < size; ++i) {
		Transform t = GetGlobalTransform(i);
		out[i] = t.GetTransformMatrix();
	}
}
#else
void Pose::GetMatrixPalette(std::vector<mat4>& out) {
	unsigned int size = mJoints.size();
	if (out.size() != size) {
		out.resize(size);
	}

	for (unsigned int i = 0; i < size; i++) {
		Transform t = GetGlobalTransform(i); // 找了大量重复节点，很耗时
		out[i] = transformToMat4(t);
	}
}
#endif

int Pose::GetParent(unsigned int index) {
	return mParents[index];
}

void Pose::SetParent(unsigned int index, int parent) {
	mParents[index] = parent;
}

bool Pose::operator==(const Pose& other) {
	// 如果两种容器长度不等则判断为不等
	if (mJoints.size() != other.mJoints.size()) return false; // protected声明的变量可以被类、子类中的函数、友元函数访问
	if (mParents.size() != other.mParents.size()) return false;

	unsigned int size = (unsigned int)mJoints.size();
	for (unsigned int i = 0; i < size; i++) {
		Transform thisLocal = mJoints[i];
		Transform otherLocal = other.mJoints[i];
		int thisParent = mParents[i];
		int otherParent = other.mParents[i];
		// 如果有关节的父节点不同则不相等
		if (thisParent != otherParent) return false;
		// 如果有Transform不相同则不相同
		if (thisLocal != otherLocal) return false;
	}

	return true;
}

bool Pose::operator!=(const Pose& other) {
	return !(*this == other);
}