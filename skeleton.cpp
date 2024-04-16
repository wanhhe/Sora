#include "skeleton.h"

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
	unsigned int size = mBindPose.Size();
	mInvBindPose.resize(size);

	for (unsigned int i = 0; i < size; i++) {
		Transform world = mBindPose.GetGlobalTransform(i); // ���ÿ���ؽڵı任
		mInvBindPose[i] = glm::inverse(world.GetTransformMatrix()); // ����õ���������ƾ���
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