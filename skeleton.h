#pragma once

#include <string>
#include <vector>
#include "mat4.h"
#include "pose.h"

// �����ɫ��һЩ���õ���Ϣ����restPose bindPose�ȡ���Ȼ��ɫҲ���Բ�������Щ��Ϣ��ֻ�ǻ����Ķ�����ڴ�
class Skeleton {
protected:
	Pose mRestPose;
	Pose mBindPose;
	std::vector<glm::mat4> mInvBindPose; // ��������ƾ������ڴ�ģ�Ϳռ�ת����Ƥ�ռ�
	std::vector<std::string> mJointNames; // �ؽ�����

	void UpdateInverseBindPose();

public:
	Skeleton();
	Skeleton(const Pose& rest, const Pose& bind, const std::vector<std::string>& names);

	void Set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names);
	Pose& GetBindPose();
	Pose& GetRestPose();
	std::vector<std::string>& GetJointNames();
	std::string& GetJointName(unsigned int index);
	std::vector<glm::mat4>& GetInvBindPose();
};