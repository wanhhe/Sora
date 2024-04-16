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
	float Sample(Pose& outPose, float time); // ��Clip���������������һ��Pose
	TRACK& operator[](unsigned int joint); // ���ݹ���id���
	void RecalculateDuration(); // ����Clip��ʱ����

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