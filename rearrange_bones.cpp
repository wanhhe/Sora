#include "rearrange_bones.h"
#include <list>

BoneMap RearrangeSkeleton(Skeleton& skeleton) {
	Pose& restPose = skeleton.GetRestPose();
	Pose& bindPose = skeleton.GetBindPose();
	unsigned int size = restPose.Size();
	if (size == 0) { // 如果骨骼是空的话，返回一个空映射
		return BoneMap();
	}

	std::vector<std::vector<int>> hierarchy(size); // 外层每个元素表示一个节点，内层表示该节点的所有子节点
	std::list<int> process; // 双向链表
	for (unsigned int i = 0; i < size; i++) {
		int parent = restPose.GetParent(i);
		if (parent >= 0) { // 如果是某个节点的子节点，则推入该节点中
			hierarchy[parent].push_back(i);
		}
		else { // 根节点
			process.push_back(i);
		}
	}

	BoneMap mapForward; // key是新的顺序索引，值是原来的顺序索引
	BoneMap mapBackward; // key是原来的顺序索引，值是新的顺序索引
	int index = 0;
	// 层次遍历处理每个节点
	while (process.size() > 0) {
		int current = *process.begin();
		process.pop_front();
		std::vector<int>& children = hierarchy[current]; // 所有孩子节点
		unsigned int numChildren = (unsigned int)children.size();
		for (unsigned int i = 0; i < numChildren; i++) {
			process.push_back(children[i]);
		}

		// 设置映射
		mapForward[index] = current;
		mapBackward[current] = index;
		index++;
	}
	// 空结点
	mapForward[-1] = -1;
	mapBackward[-1] = -1;

	// 建立新的有序的姿态
	Pose newRestPose(size);
	Pose newBindPose(size);
	std::vector<std::string> newNames(size);
	for (unsigned int i = 0; i < size; i++) {
		int thisBone = mapForward[i]; // 该骨骼原来的索引
		newRestPose.SetLocalTransform(i, restPose.GetLocalTransform(thisBone)); // 按新的索引顺序设置骨骼
		newBindPose.SetLocalTransform(i, bindPose.GetLocalTransform(thisBone));
		newNames[i] = skeleton.GetJointName(thisBone);

		// 由原来值的顺序索引获得新的顺序索引
		int parent = mapBackward[bindPose.GetParent(thisBone)];
		newRestPose.SetParent(i, parent);
		newBindPose.SetParent(i, parent);
	}

	skeleton.Set(newRestPose, newBindPose, newNames);
	return mapBackward;
}

// 由于骨骼id变了，TransformTrack中的id也就变了，需要调整
void RearrangeClip(Clip& clip, BoneMap& boneMap) {
	unsigned int size = clip.Size();

	for (unsigned int i = 0; i < size; i++) {
		int joint = clip.GetIdAtIndex(i); // 获得骨骼的原id
		unsigned int newJoint = (unsigned int)boneMap[joint];
		clip.SetIdAtIndex(i, newJoint);
	}
}

void RearrangeFastClip(FastClip& fastClip, BoneMap& boneMap) {
	unsigned int size = fastClip.Size();

	for (unsigned int i = 0; i < size; i++) {
		int joint = fastClip.GetIdAtIndex(i); // 获得骨骼的原id
		unsigned int newJoint = (unsigned int)boneMap[joint];
		fastClip.SetIdAtIndex(i, newJoint);
	}
}

void RearrangeMesh(Mesh& mesh, BoneMap& boneMap) {
	//std::vector<ivec4>& influences = mesh.GetInfluences();
	//unsigned int size = influences.size();
	//for (unsigned int i = 0; i < size; i++) {
	//	// 修改对该mesh产生影响的骨骼的位置(索引)
	//	influences[i].x = boneMap[influences[i].x];
	//	influences[i].y = boneMap[influences[i].y];
	//	influences[i].z = boneMap[influences[i].z];
	//	influences[i].w = boneMap[influences[i].w];
	//}

	//mesh.UpdateOpenGLBuffers();
}