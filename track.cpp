#include "track.h"

// 显式声明
template Track<float, 1>;
template Track<glm::vec3, 3>;
template Track<glm::quat, 4>;

template FastTrack<float, 1>;
template FastTrack<glm::vec3, 3>;
template FastTrack<glm::quat, 4>;

namespace TrackHelpers {
	inline float Interpolate(float a, float b, float t) {
		return a + (b - a) * t;
	}

	inline glm::vec3 Interpolate(const glm::vec3& a, const glm::vec3& b, float t) {
		return glm::lerp(a, b, t);
	}

	inline glm::quat Interpolate(const glm::quat& a, const glm::quat& b, float t) {
		glm::quat result = mix(a, b, t);
		if (dot(a, b) < 0) {
			result = mix(a, -b, t);
		}
		return glm::normalize(result); // nlerp
	}

	inline float AdjustHermiteResult(float f) {
		return f;
	}

	inline glm::vec3 AdjustHermiteResult(glm::vec3 v) {
		return v;
	}

	inline glm::quat AdjustHermiteResult(glm::quat q) {
		return normalize(q);
	}

	inline void Neighborhood(const float& a, float& b) {}
	inline void Neighborhood(const glm::vec3& a, glm::vec3& b) {}
	inline void Neighborhood(const glm::quat& a, glm::quat& b) {
		if (dot(a, b) < 0) {
			b = -b;
		}
	}
};

template<typename T, int N>
Track<T, N>::Track() {
	mInterpolation = Interpolation::Linear;
}

template<typename T, int N>
float Track<T, N>::GetStartTime() {
	return mFrames[0].mTime;
}

template<typename T, int N>
float Track<T, N>::GetEndTime() {
	return mFrames[mFrames.size() - 1].mTime;
}

template<typename T, int N>
T Track<T, N>::Sample(float time, bool looping) {
	if (mInterpolation == Interpolation::Constant) {
		return SampleConstant(time, looping);
	}
	else if (mInterpolation == Interpolation::Linear) {
		return SampleLinear(time, looping);
	}
	return SampleCubic(time, looping);
}

template<typename T, int N>
Frame<N>& Track<T, N>::operator[](unsigned int index) {
	return mFrames[index];
}

template<typename T, int N>
void Track<T, N>::Resize(unsigned int size) {
	mFrames.resize(size); // 若变长，用0填充
}

template<typename T, int N>
unsigned int Track<T, N>::Size() {
	return mFrames.size();
}

template<typename T, int N>
Interpolation Track<T, N>::GetInterpolation() {
	return mInterpolation;
}

template<typename T, int N>
void Track<T, N>::SetInterpolation(Interpolation interpolation) {
	mInterpolation = interpolation;
}

template<typename T, int N>
T Track<T, N>::Hermite(float time, const T& point1, const T& slope1, const T& _point2, const T& slope2) {
	float tt = time * time;
	float ttt = tt * time;

	T point2 = _point2;
	TrackHelpers::Neighborhood(point1, point2);
	float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
	float h2 = 2.0f * ttt + 3.0f * tt;
	float h3 = ttt - 2.0f * tt + time;
	float h4 = ttt - tt;
	T result = point1 * h1 + point2 * h2 + slope1 * h3 + slope2 * h4;
	return TrackHelpers::AdjustHermiteResult(result);
}

template<typename T, int N>
int Track<T, N>::FrameIndex(float time, bool looping) {
	unsigned int size = (unsigned int)mFrames.size();
	// 若轨迹中帧数小于2则为非法
	if (size <= 1) return -1;

	// 若轨迹是循环的，则需调整时间到划定的时间内
	if (looping) {
		float startTime = mFrames[0].mTime;
		float endTime = mFrames[size - 1].mTime;
		float duration = endTime - startTime;
		// 将时间修正到合适的范围内
		time = fmodf(time - startTime, duration); // 浮点数求模。第一个参数 % 第二个参数
		if (time < 0) { // 在开始时间前发生
			time += duration;
		}
		time = time + startTime;
	}
	else {
		// 如果不是循环的，则小于第一帧开始时间的都要钳制到第一帧
		if (time <= mFrames[0].mTime) {
			return 0;
		}
		// 大于倒数第二帧的时间都要钳制到倒数第二帧，因为要获得的是指定时间前的一帧索引
		if (time >= mFrames[size - 2].mTime) {
			return (int)size - 2;
		}
	}

	for (int i = (int)size - 1; i >= 0; i--) {
		if (time >= mFrames[i].mTime) return i;
	}

	// unreachable
	return -1;
}

template<typename T, int N>
float Track<T, N>::AdjustTimeToFitTrack(float time, bool looping) {
	unsigned int size = (unsigned int)mFrames.size();
	if (size <= 1) return 0.0f; // 非法的时间

	float startTime = mFrames[0].mTime;
	float endTime = mFrames[size - 1].mTime;
	float duration = endTime - startTime;
	if (duration <= 0.0f) return 0.0f; // 时间间隔不能小于0

	if (looping) {
		time = fmodf(time - startTime, duration);
		if (time < 0.0f) time += duration;
		time = time + startTime;
	}
	else {
		if (time <= mFrames[0].mTime) time = startTime;
		if (time >= mFrames[size - 1].mTime) time = endTime;
	}

	// unreachable
	return time;
}

template<> // 特化
float Track<float, 1>::Cast(float* value) {
	return value[0];
}

template<>
glm::vec3 Track<glm::vec3, 3>::Cast(float* value) {
	return glm::vec3(value[0], value[1], value[2]);
}

template<>
glm::quat Track<glm::quat, 4>::Cast(float* value) {
	glm::quat q = glm::quat(value[0], value[1], value[2], value[3]);
	return normalize(q);
}

template<typename T, int N>
T Track<T, N>::SampleConstant(float time, bool looping) {
	int frame = FrameIndex(time, looping); // 找到该时间点前的一帧
	if (frame < 0 || frame >= (int)mFrames.size()) { // 非法索引
		return T();
	}

	return Cast(&mFrames[frame].mValue[0]);
}

template<typename T, int N>
T Track<T, N>::SampleLinear(float time, bool looping) {
	int thisFrame = FrameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= mFrames.size() - 1) return T(); // 非法索引
	int nextFrame = thisFrame + 1;
	float trackTime = AdjustTimeToFitTrack(time, looping); // 修正时间
	float thisTime = mFrames[thisFrame].mTime;
	float frameDelta = mFrames[nextFrame].mTime - thisTime; // 两帧之间的时间差
	if (frameDelta <= 0.0f) return T();

	float t = (trackTime - thisTime) / frameDelta; // 插值的权重
	T start = Cast(&mFrames[thisFrame].mValue[0]);
	T end = Cast(&mFrames[nextFrame].mValue[0]);

	return TrackHelpers::Interpolate(start, end, t);
}

template<typename T, int N>
T Track<T, N>::SampleCubic(float time, bool looping) {
	int thisFrame = FrameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= mFrames.size() - 1) return T(); // 非法索引
	int nextFrame = thisFrame + 1;
	float trackTime = AdjustTimeToFitTrack(time, looping); // 修正时间
	float thisTime = mFrames[thisFrame].mTime;
	float frameDelta = mFrames[nextFrame].mTime - thisTime; // 两帧之间的时间差
	if (frameDelta <= 0.0f) return T();

	float t = (trackTime - thisTime) / frameDelta; // 插值的权重

	T point1 = Cast(&mFrames[thisFrame].mValue[0]);
	T slope1;
	memcpy(&slope1, mFrames[thisFrame].mOut, N * sizeof(float));
	slope1 = slope1 * frameDelta; // 要缩放 ?

	T point2 = Cast(&mFrames[nextFrame].mValue[0]);
	T slope2;
	memcpy(&slope2, mFrames[nextFrame].mIn, N * sizeof(float));
	slope2 = slope2 * frameDelta;

	return Hermite(t, point1, slope1, point2, slope2);
}

template<typename T, int N>
void FastTrack<T, N>::UpdateIndexLookupTable() {
	// 首先保证有合法的帧
	int numFrames = (int)this->mFrames.size();
	if (numFrames <= 1) return;

	float duration = this->GetEndTime() - this->GetStartTime();
	unsigned int numSamples = 60 + duration * 60.f; // 一秒取60个采样点  感觉应该加个数保证duration为0带来的结果为0
	mSampledFrames.resize(numSamples);

	for (unsigned int i = 0; i < numSamples; i++) {
		// 计算该采样点对应的时间
		float t = (float)i / (float)(numSamples - 1);
		float time = t * duration + this->GetStartTime();

		// 找该时间点之前的最后一帧
		unsigned int frameIndex = 0;
		for (int j = numFrames - 1; j >= 0; j--) {
			if (time >= this->mFrames[j].mTime) {
				frameIndex = (unsigned int)j;
				// 如果是该时间点之前的帧是最后一帧，令其为倒数第二帧
				if ((int)frameIndex >= numFrames - 2) {
					frameIndex = numFrames - 2;
				}
				break;
			}
		}
		mSampledFrames[i] = frameIndex;
	}
}

template<typename T, int N>
int FastTrack<T, N>::FrameIndex(float time, bool looping) {
	// 模板定义阶段：只对模板中和模板参数无关的名字进行查找，忽略那些有模板参数的部分。
	// 模板实例化阶段：对模板中和模板参数有关的名字进行查找，替换模板参数为实际类型。
	// mFrames是一个和模板参数有关的名字,其中N是模板参数。在模板定义阶段，编译器会忽略掉mFrames的存在。而在模板实例化阶段，编译器已经认定mFrames是一个非成员函数，不会去基类中查找它，所以就会报错找不到mFrames。
	// 为了解决这个问题，需要在mFrames前面加上this->或者SeqList<T>::，这样就可以告诉编译器，mFrames是一个成员变量，需要在基类中查找。这样，编译器就会在模板实例化阶段，正确地找到mFrames的定义，而不会报错。
	std::vector<Frame<N>>& frames = this->mFrames;
	unsigned int size = frames.size();
	if (size <= 1) return -1;

	// 调整时间
	if (looping) {
		float startTime = this->GetStartTime();
		float endTime = this->GetEndTime();
		float duration = endTime - startTime;
		time = fmodf(time - startTime, duration);
		if (time < 0.0f) {
			time += duration;
		}
		time += startTime;
	}
	else {
		if (time <= frames[0].mTime) return 0;
		if (time >= frames[size - 2].mTime) return (int)size - 2;
	}

	float duration = this->GetEndTime() - this->GetStartTime();
	float t = time / duration;
	unsigned int numSamples = 60 + (unsigned int)(duration * 60.0f);

	unsigned int index = t * numSamples;
	if (index >= mSampledFrames.size()) {
		return -1;
	}
	return (int)mSampledFrames[index];
}

template FastTrack<float, 1> OptimizeTrack(Track<float, 1>& input);
template FastTrack<glm::vec3, 3> OptimizeTrack(Track<glm::vec3, 3>& input);
template FastTrack<glm::quat, 4> OptimizeTrack(Track<glm::quat, 4>& input);

template<typename T, int N>
FastTrack<T, N> OptimizeTrack(Track<T, N>& input) {
	FastTrack<T, N> result;

	result.SetInterpolation(input.GetInterpolation());
	unsigned int size = input.Size();
	result.Resize(size);
	for (unsigned int i = 0; i < size; i++) {
		result[i] = input[i];
	}
	result.UpdateIndexLookupTable();

	return result;
}