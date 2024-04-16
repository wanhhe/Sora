#pragma once

#include <vector>
#include <string>
#include "transform_track.h"
#include "pose.h"

template <typename TRACK>
class TClip {
protected:
	std::vector<TRACK> mTracks;
	std::string mName;
	float mStartTime;
	float mEndTime;
	bool mLooping;

	float AdjustTimeToFitRange(float time);

public:
	TClip();

	unsigned int GetIdAtIndex(unsigned int index);
	void SetIdAtIndex(unsigned int index, unsigned int id);
	unsigned int Size();
	float Sample(Pose& outPose, float time); // 对Clip采样并将结果赋给一个Pose
	TRACK& operator[](unsigned int joint); // 根据骨骼id获得
	void RecalculateDuration(); // 计算Clip的时间间隔

	std::string& GetName();
	void SetName(const std::string& newName);
	float GetStartTime();
	float GetEndTime();
	float GetDuration();
	bool GetLooping();
	void SetLooping(bool looping);
};

typedef TClip<TransformTrack> Clip;
typedef TClip<FastTransformTrack> FastClip;

FastClip OptimizeClip(Clip& input);