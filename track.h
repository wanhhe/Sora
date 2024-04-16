#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Frame.h"
#include "Interpolation.h"

template<typename T, int N>
class Track {
protected:
	// 只需要帧信息的集合和插值类型
	std::vector<Frame<N>> mFrames;
	Interpolation mInterpolation;

	T SampleConstant(float time, bool looping);
	T SampleLinear(float time, bool looping);
	T SampleCubic(float time, bool looping);
	T Hermite(float time, const T& point1, const T& slope1, const T& _point2, const T& slope2);
	virtual int FrameIndex(float time, bool looping); // 获得指定时间前的一帧的索引
	float AdjustTimeToFitTrack(float time, bool looping); // 将track范围外的时间调整到track内
	T Cast(float* value); // 用于将Frame中存储的浮点数组转化为对应的类型

public:
	Track();
	void Resize(unsigned int size);
	unsigned int Size();
	Interpolation GetInterpolation();
	void SetInterpolation(Interpolation interpolation);
	float GetStartTime();
	float GetEndTime();
	T Sample(float time, bool looping); // 根据时间和动画是否循环进行采样
	Frame<N>& operator[](unsigned int index);
};

typedef Track<float, 1> ScalarTrack;
typedef Track<glm::vec3, 3> VectorTrack;
typedef Track<glm::quat, 4> QuaternionTrack;


template<typename T, int N>
class FastTrack : public Track<T, N> {
protected:
	std::vector<unsigned int> mSampledFrames; // 记录每个采样的时间点前的帧的索引
	virtual int FrameIndex(float time, bool looping);

public:
	void UpdateIndexLookupTable();
};

typedef FastTrack<float, 1> FastScalarTrack;
typedef FastTrack<glm::vec3, 3> FastVectorTrack;
typedef FastTrack<glm::quat, 4> FastQuaternionTrack;

template<typename T, int N>
FastTrack<T, N> OptimizeTrack(Track<T, N>& input); // 将Track转为FastTrack