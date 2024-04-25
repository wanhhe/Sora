#include "skeleton.h"
#include <iostream>

Skeleton::Skeleton() { }

Skeleton::Skeleton(const Pose& rest, const Pose& bind, const std::vector<std::string>& names) {
	Set(rest, bind, names);
}

void Skeleton::Set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names) {
	mRestPose = rest;
	mBindPose = bind;
	mJointNames = names;
	UpdateInverseBindPose();
}

void Skeleton::UpdateInverseBindPose() {
	std::cout << "time " << std::endl;
	unsigned int size = mBindPose.Size();
	mInvBindPose.resize(size);

	for (unsigned int i = 0; i < size; i++) {
		Transform world = mBindPose.GetGlobalTransform(i); // 获得每个关节的变换
		mInvBindPose[i] = glm::inverse(world.GetTransformMatrix()); // 求逆得到反向绑定姿势矩阵
		 //std::cout << "ii" << std::endl;
		 //std::cout << mInvBindPose[i][0][0] << " " << mInvBindPose[i][0][1] << " " << mInvBindPose[i][0][2] << " " << mInvBindPose[i][0][3] << std::endl;
	}
}

Pose& Skeleton::GetBindPose() {
	return mBindPose;
}

Pose& Skeleton::GetRestPose() {
	return mRestPose;
}

std::vector<std::string>& Skeleton::GetJointNames() {
	return mJointNames;
}

std::string& Skeleton::GetJointName(unsigned int index) {
	return mJointNames[index];
}

std::vector<glm::mat4>& Skeleton::GetInvBindPose() {
	return mInvBindPose;
}