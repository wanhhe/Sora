#include "rearrange_bones.h"
#include <list>

BoneMap RearrangeSkeleton(Skeleton& skeleton) {
	Pose& restPose = skeleton.GetRestPose();
	Pose& bindPose = skeleton.GetBindPose();
	unsigned int size = restPose.Size();
	if (size == 0) { // ��������ǿյĻ�������һ����ӳ��
		return BoneMap();
	}

	std::vector<std::vector<int>> hierarchy(size); // ���ÿ��Ԫ�ر�ʾһ���ڵ㣬�ڲ��ʾ�ýڵ�������ӽڵ�
	std::list<int> process; // ˫������
	for (unsigned int i = 0; i < size; i++) {
		int parent = restPose.GetParent(i);
		if (parent >= 0) { // �����ĳ���ڵ���ӽڵ㣬������ýڵ���
			hierarchy[parent].push_back(i);
		}
		else { // ���ڵ�
			process.push_back(i);
		}
	}

	BoneMap mapForward; // key���µ�˳��������ֵ��ԭ����˳������
	BoneMap mapBackward; // key��ԭ����˳��������ֵ���µ�˳������
	int index = 0;
	// ��α�������ÿ���ڵ�
	while (process.size() > 0) {
		int current = *process.begin();
		process.pop_front();
		std::vector<int>& children = hierarchy[current]; // ���к��ӽڵ�
		unsigned int numChildren = (unsigned int)children.size();
		for (unsigned int i = 0; i < numChildren; i++) {
			process.push_back(children[i]);
		}

		// ����ӳ��
		mapForward[index] = current;
		mapBackward[current] = index;
		index++;
	}
	// �ս��
	mapForward[-1] = -1;
	mapBackward[-1] = -1;

	// �����µ��������̬
	Pose newRestPose(size);
	Pose newBindPose(size);
	std::vector<std::string> newNames(size);
	for (unsigned int i = 0; i < size; i++) {
		int thisBone = mapForward[i]; // �ù���ԭ��������
		newRestPose.SetLocalTransform(i, restPose.GetLocalTransform(thisBone)); // ���µ�����˳�����ù���
		newBindPose.SetLocalTransform(i, bindPose.GetLocalTransform(thisBone));
		newNames[i] = skeleton.GetJointName(thisBone);

		// ��ԭ��ֵ��˳����������µ�˳������
		int parent = mapBackward[bindPose.GetParent(thisBone)];
		newRestPose.SetParent(i, parent);
		newBindPose.SetParent(i, parent);
	}

	skeleton.Set(newRestPose, newBindPose, newNames);
	return mapBackward;
}

// ���ڹ���id���ˣ�TransformTrack�е�idҲ�ͱ��ˣ���Ҫ����
void RearrangeClip(Clip& clip, BoneMap& boneMap) {
	unsigned int size = clip.Size();

	for (unsigned int i = 0; i < size; i++) {
		int joint = clip.GetIdAtIndex(i); // ��ù�����ԭid
		unsigned int newJoint = (unsigned int)boneMap[joint];
		clip.SetIdAtIndex(i, newJoint);
	}
}

void RearrangeFastClip(FastClip& fastClip, BoneMap& boneMap) {
	unsigned int size = fastClip.Size();

	for (unsigned int i = 0; i < size; i++) {
		int joint = fastClip.GetIdAtIndex(i); // ��ù�����ԭid
		unsigned int newJoint = (unsigned int)boneMap[joint];
		fastClip.SetIdAtIndex(i, newJoint);
	}
}

void RearrangeMesh(Mesh& mesh, BoneMap& boneMap) {
	//std::vector<ivec4>& influences = mesh.GetInfluences();
	//unsigned int size = influences.size();
	//for (unsigned int i = 0; i < size; i++) {
	//	// �޸ĶԸ�mesh����Ӱ��Ĺ�����λ��(����)
	//	influences[i].x = boneMap[influences[i].x];
	//	influences[i].y = boneMap[influences[i].y];
	//	influences[i].z = boneMap[influences[i].z];
	//	influences[i].w = boneMap[influences[i].w];
	//}

	//mesh.UpdateOpenGLBuffers();
}