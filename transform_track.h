#pragma once

#include "track.h"
#include "transform.h"

template <typename VTRACK, typename QTRACK>
class TTransformTrack {
protected:
	unsigned int mId; // ��Ӧ��Ҫ�任�Ĺ�����id
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
	float GetStartTime(); // ���غϷ��켣������Ŀ�ʼʱ��
	float GetEndTime(); // ���غϷ��켣������Ŀ�ʼʱ��
	bool IsValid(); // �ж����Ƿ���һ���켣�ǺϷ���
	Transform Sample(const Transform& ref, float time, bool looping);
};

typedef TTransformTrack<VectorTrack, QuaternionTrack> TransformTrack;
typedef TTransformTrack<FastVectorTrack, FastQuaternionTrack> FastTransformTrack;

FastTransformTrack OptimizeTransformTrack(TransformTrack& input);