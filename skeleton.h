#pragma once

#include <string>
#include <vector>
#include "mat4.h"
#include "pose.h"

// 保存角色间一些共用的信息，如restPose bindPose等。当然角色也可以不共用这些信息，只是会消耗额外的内存
class Skeleton {
protected:
	Pose mRestPose;
	Pose mBindPose;
	std::vector<glm::mat4> mInvBindPose; // 反向绑定姿势矩阵。用于从模型空间转到蒙皮空间
	std::vector<std::string> mJointNames; // 关节名称

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