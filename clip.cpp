#include "clip.h"

template TClip<TransformTrack>;
template TClip<FastTransformTrack>;

template <typename TRACK>
TClip<TRACK>::TClip() {
	mName = "default";
	mStartTime = 0.0f;
	mEndTime = 0.0f;
	mLooping = true;
}

template <typename TRACK>
float TClip<TRACK>::Sample(Pose& outPose, float time) {
	// 首先判断该Clip是否合法
	if (GetDuration() <= 0.0f) return 0.0f;
	// 矫正时间
	time = AdjustTimeToFitRange(time);

	unsigned int size = mTracks.size();
	for (unsigned int i = 0; i < size; i++) {
		unsigned int joint = mTracks[i].GetId();
		// 如果变换的组件没有设置动画，则使用引用的Pose的默认值
		Transform local = outPose.GetLocalTransform(joint);
		Transform animated = mTracks[i].Sample(local, time, mLooping);
		outPose.SetLocalTransform(joint, animated);
	}
	return time;
}

template <typename TRACK>
void TClip<TRACK>::RecalculateDuration() {
	mStartTime = 0.0f;
	mEndTime = 0.0f;
	bool startSet = false;
	bool endSet = false;
	unsigned int size = mTracks.size();

	for (unsigned int i = 0; i < size; i++) {
		if (mTracks[i].IsValid()) {
			float startTime = mTracks[i].GetStartTime();
			float endTime = mTracks[i].GetEndTime();
			if (startTime < mStartTime || !startSet) {
				mStartTime = startTime;
				startSet = true;
			}
			if (endTime > mEndTime || !endSet) {
				mEndTime = endTime;
				endSet = true;
			}
		}
	}
}

template <typename TRACK>
float TClip<TRACK>::AdjustTimeToFitRange(float time) {
	if (mLooping) {
		float duration = mEndTime - mStartTime;
		if (duration <= 0.0f) return 0.0f;
		time = fmodf(time - mStartTime, duration);
		if (time <= 0.0f) {
			time += duration;
		}
		time += mStartTime;
	}
	else
	{
		if (time < mStartTime) time = mStartTime;
		if (time > mEndTime) time = mEndTime;
	}
	return time;
}

template <typename TRACK>
TRACK& TClip<TRACK>::operator[](unsigned int joint) {
	unsigned int size = mTracks.size();
	for (unsigned int i = 0; i < size; i++) {
		if (mTracks[i].GetId() == joint) {
			return mTracks[i];
		}
	}

	// 如果没有该轨迹，创建一个并返回
	mTracks.push_back(TRACK());
	mTracks[mTracks.size() - 1].SetId(joint);
	return mTracks[mTracks.size() - 1];
}

template <typename TRACK>
unsigned int TClip<TRACK>::GetIdAtIndex(unsigned int index) {
	return mTracks[index].GetId();
}

template <typename TRACK>
void TClip<TRACK>::SetIdAtIndex(unsigned int index, unsigned int id) {
	mTracks[index].SetId(id);
}

template <typename TRACK>
unsigned int TClip<TRACK>::Size() {
	return (unsigned int)mTracks.size();
}

template <typename TRACK>
std::string& TClip<TRACK>::GetName() {
	return mName;
}

template <typename TRACK>
void TClip<TRACK>::SetName(const std::string& newName) {
	mName = newName;
}

template <typename TRACK>
float TClip<TRACK>::GetStartTime() {
	return mStartTime;
}

template <typename TRACK>
float TClip<TRACK>::GetEndTime() {
	return mEndTime;
}

template <typename TRACK>
float TClip<TRACK>::GetDuration() {
	return mEndTime - mStartTime;
}

template <typename TRACK>
bool TClip<TRACK>::GetLooping() {
	return mLooping;
}

template <typename TRACK>
void TClip<TRACK>::SetLooping(bool looping) {
	mLooping = looping;
}

FastClip OptimizeClip(Clip& input) {
	FastClip result;

	result.SetName(input.GetName());
	result.SetLooping(input.GetLooping());
	unsigned int size = input.Size();
	for (unsigned int i = 0; i < size; i++) {
		unsigned int joint = input.GetIdAtIndex(i);
		result[joint] = OptimizeTransformTrack(input[joint]);
	}

	// 计算时间
	result.RecalculateDuration();

	return result;
}