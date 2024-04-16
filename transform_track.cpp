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
	// 只要有一个合法就可以了
	return mPosition.Size() > 1 || mRotation.Size() > 1 || mScale.Size() > 1;
}

template <typename VTRACK, typename QTRACK>
float TTransformTrack<VTRACK, QTRACK>::GetStartTime() {
	float result = 0.0f;
	bool isSet = false; // 是否已经有合法的轨迹

	if (mPosition.Size() > 1) {
		result = mPosition.GetStartTime();
		isSet = true; // 有合法的帧
	}
	if (mRotation.Size() > 1) {
		float rotationStart = mRotation.GetStartTime();
		if (rotationStart < result || !isSet) { // 如果比现在的开始时间早或还没有合法帧，则赋值
			result = rotationStart;
			isSet = true;
		}
	}
	if (mScale.Size() > 1) {
		float scalenStart = mScale.GetStartTime();
		if (scalenStart < result || !isSet) { // 如果比现在的开始时间早或还没有合法帧，则赋值
			result = scalenStart;
			isSet = true;
		}
	}

	return result;
}

template <typename VTRACK, typename QTRACK>
float TTransformTrack<VTRACK, QTRACK>::GetEndTime() {
	float result = 0.0f;
	bool isSet = false; // 是否已经有合法的轨迹

	if (mPosition.Size() > 1) {
		result = mPosition.GetEndTime();
		isSet = true; // 有合法的帧
	}
	if (mRotation.Size() > 1) {
		float rotationEnd = mRotation.GetEndTime();
		if (rotationEnd > result || !isSet) { // 如果比现在的开始时间早或还没有合法帧，则赋值
			result = rotationEnd;
			isSet = true;
		}
	}
	if (mScale.Size() > 1) {
		float scalenEnd = mScale.GetEndTime();
		if (scalenEnd > result || !isSet) { // 如果比现在的开始时间早或还没有合法帧，则赋值
			result = scalenEnd;
			isSet = true;
		}
	}

	return result;
}

template <typename VTRACK, typename QTRACK>
Transform TTransformTrack<VTRACK, QTRACK>::Sample(const Transform& ref, float time, bool looping) {
	Transform result = ref; // 默认的变换
	if (mPosition.Size() > 1) { // 合法轨迹才进行变换
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