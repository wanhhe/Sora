#pragma once

#include "track.h"
#include "transform.h"

template <typename VTRACK, typename QTRACK>
class TTransformTrack {
protected:
	unsigned int mId; // 对应的要变换的骨骼的id
	VTRACK mPosition;
	QTRACK mRotation;
	VTRACK mScale;

public:
	TTransformTrack();
	unsigned int GetId();
	void SetId(unsigned int id);
	VTRACK& GetPositionTrack();
	QTRACK& GetRotationTrack();
	VTRACK& GetScaleTrack();
	float GetStartTime(); // 返回合法轨迹中最早的开始时间
	float GetEndTime(); // 返回合法轨迹中最晚的开始时间
	bool IsValid(); // 判断是是否有一个轨迹是合法的
	Transform Sample(const Transform& ref, float time, bool looping);
};

typedef TTransformTrack<VectorTrack, QuaternionTrack> TransformTrack;
typedef TTransformTrack<FastVectorTrack, FastQuaternionTrack> FastTransformTrack;

FastTransformTrack OptimizeTransformTrack(TransformTrack& input);