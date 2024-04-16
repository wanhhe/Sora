#include "transform_track.h"

template TTransformTrack<VectorTrack, QuaternionTrack>;
template TTransformTrack<FastVectorTrack, FastQuaternionTrack>;

template <typename VTRACK, typename QTRACK>
TTransformTrack<VTRACK, QTRACK>::TTransformTrack() {
	mId = 0;
}

template <typename VTRACK, typename QTRACK>
unsigned int TTransformTrack<VTRACK, QTRACK>::GetId() {
	return mId;
}

template <typename VTRACK, typename QTRACK>
void TTransformTrack<VTRACK, QTRACK>::SetId(unsigned int id) {
	mId = id;
}

template <typename VTRACK, typename QTRACK>
VTRACK& TTransformTrack<VTRACK, QTRACK>::GetPositionTrack() {
	return mPosition;
}

template <typename VTRACK, typename QTRACK>
QTRACK& TTransformTrack<VTRACK, QTRACK>::GetRotationTrack() {
	return mRotation;
}

template <typename VTRACK, typename QTRACK>
VTRACK& TTransformTrack<VTRACK, QTRACK>::GetScaleTrack() {
	return mScale;
}

template <typename VTRACK, typename QTRACK>
bool TTransformTrack<VTRACK, QTRACK>::IsValid() {
	// ֻҪ��һ���Ϸ��Ϳ�����
	return mPosition.Size() > 1 || mRotation.Size() > 1 || mScale.Size() > 1;
}

template <typename VTRACK, typename QTRACK>
float TTransformTrack<VTRACK, QTRACK>::GetStartTime() {
	float result = 0.0f;
	bool isSet = false; // �Ƿ��Ѿ��кϷ��Ĺ켣

	if (mPosition.Size() > 1) {
		result = mPosition.GetStartTime();
		isSet = true; // �кϷ���֡
	}
	if (mRotation.Size() > 1) {
		float rotationStart = mRotation.GetStartTime();
		if (rotationStart < result || !isSet) { // ��������ڵĿ�ʼʱ�����û�кϷ�֡����ֵ
			result = rotationStart;
			isSet = true;
		}
	}
	if (mScale.Size() > 1) {
		float scalenStart = mScale.GetStartTime();
		if (scalenStart < result || !isSet) { // ��������ڵĿ�ʼʱ�����û�кϷ�֡����ֵ
			result = scalenStart;
			isSet = true;
		}
	}

	return result;
}

template <typename VTRACK, typename QTRACK>
float TTransformTrack<VTRACK, QTRACK>::GetEndTime() {
	float result = 0.0f;
	bool isSet = false; // �Ƿ��Ѿ��кϷ��Ĺ켣

	if (mPosition.Size() > 1) {
		result = mPosition.GetEndTime();
		isSet = true; // �кϷ���֡
	}
	if (mRotation.Size() > 1) {
		float rotationEnd = mRotation.GetEndTime();
		if (rotationEnd > result || !isSet) { // ��������ڵĿ�ʼʱ�����û�кϷ�֡����ֵ
			result = rotationEnd;
			isSet = true;
		}
	}
	if (mScale.Size() > 1) {
		float scalenEnd = mScale.GetEndTime();
		if (scalenEnd > result || !isSet) { // ��������ڵĿ�ʼʱ�����û�кϷ�֡����ֵ
			result = scalenEnd;
			isSet = true;
		}
	}

	return result;
}

template <typename VTRACK, typename QTRACK>
Transform TTransformTrack<VTRACK, QTRACK>::Sample(const Transform& ref, float time, bool looping) {
	Transform result = ref; // Ĭ�ϵı任
	if (mPosition.Size() > 1) { // �Ϸ��켣�Ž��б任
		result.SetPosition(mPosition.Sample(time, looping));
	}
	if (mRotation.Size() > 1) {
		result.SetRotation(mRotation.Sample(time, looping));
	}
	if (mScale.Size() > 1) {
		result.SetScale(mScale.Sample(time, looping));
	}

	return result;
}

FastTransformTrack OptimizeTransformTrack(TransformTrack& input) {
	FastTransformTrack result;

	result.SetId(input.GetId());
	result.GetPositionTrack() = OptimizeTrack<glm::vec3, 3>(input.GetPositionTrack());
	result.GetRotationTrack() = OptimizeTrack<glm::quat, 4>(input.GetRotationTrack());
	result.GetScaleTrack() = OptimizeTrack<glm::vec3, 3>(input.GetScaleTrack());

	return result;
}