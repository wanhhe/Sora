#include "track.h"

// ��ʽ����
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
	mFrames.resize(size); // ���䳤����0���
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
	// ���켣��֡��С��2��Ϊ�Ƿ�
	if (size <= 1) return -1;

	// ���켣��ѭ���ģ��������ʱ�䵽������ʱ����
	if (looping) {
		float startTime = mFrames[0].mTime;
		float endTime = mFrames[size - 1].mTime;
		float duration = endTime - startTime;
		// ��ʱ�����������ʵķ�Χ��
		time = fmodf(time - startTime, duration); // ��������ģ����һ������ % �ڶ�������
		if (time < 0) { // �ڿ�ʼʱ��ǰ����
			time += duration;
		}
		time = time + startTime;
	}
	else {
		// �������ѭ���ģ���С�ڵ�һ֡��ʼʱ��Ķ�Ҫǯ�Ƶ���һ֡
		if (time <= mFrames[0].mTime) {
			return 0;
		}
		// ���ڵ����ڶ�֡��ʱ�䶼Ҫǯ�Ƶ������ڶ�֡����ΪҪ��õ���ָ��ʱ��ǰ��һ֡����
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
	if (size <= 1) return 0.0f; // �Ƿ���ʱ��

	float startTime = mFrames[0].mTime;
	float endTime = mFrames[size - 1].mTime;
	float duration = endTime - startTime;
	if (duration <= 0.0f) return 0.0f; // ʱ��������С��0

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

template<> // �ػ�
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
	int frame = FrameIndex(time, looping); // �ҵ���ʱ���ǰ��һ֡
	if (frame < 0 || frame >= (int)mFrames.size()) { // �Ƿ�����
		return T();
	}

	return Cast(&mFrames[frame].mValue[0]);
}

template<typename T, int N>
T Track<T, N>::SampleLinear(float time, bool looping) {
	int thisFrame = FrameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= mFrames.size() - 1) return T(); // �Ƿ�����
	int nextFrame = thisFrame + 1;
	float trackTime = AdjustTimeToFitTrack(time, looping); // ����ʱ��
	float thisTime = mFrames[thisFrame].mTime;
	float frameDelta = mFrames[nextFrame].mTime - thisTime; // ��֮֡���ʱ���
	if (frameDelta <= 0.0f) return T();

	float t = (trackTime - thisTime) / frameDelta; // ��ֵ��Ȩ��
	T start = Cast(&mFrames[thisFrame].mValue[0]);
	T end = Cast(&mFrames[nextFrame].mValue[0]);

	return TrackHelpers::Interpolate(start, end, t);
}

template<typename T, int N>
T Track<T, N>::SampleCubic(float time, bool looping) {
	int thisFrame = FrameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= mFrames.size() - 1) return T(); // �Ƿ�����
	int nextFrame = thisFrame + 1;
	float trackTime = AdjustTimeToFitTrack(time, looping); // ����ʱ��
	float thisTime = mFrames[thisFrame].mTime;
	float frameDelta = mFrames[nextFrame].mTime - thisTime; // ��֮֡���ʱ���
	if (frameDelta <= 0.0f) return T();

	float t = (trackTime - thisTime) / frameDelta; // ��ֵ��Ȩ��

	T point1 = Cast(&mFrames[thisFrame].mValue[0]);
	T slope1;
	memcpy(&slope1, mFrames[thisFrame].mOut, N * sizeof(float));
	slope1 = slope1 * frameDelta; // Ҫ���� ?

	T point2 = Cast(&mFrames[nextFrame].mValue[0]);
	T slope2;
	memcpy(&slope2, mFrames[nextFrame].mIn, N * sizeof(float));
	slope2 = slope2 * frameDelta;

	return Hermite(t, point1, slope1, point2, slope2);
}

template<typename T, int N>
void FastTrack<T, N>::UpdateIndexLookupTable() {
	// ���ȱ�֤�кϷ���֡
	int numFrames = (int)this->mFrames.size();
	if (numFrames <= 1) return;

	float duration = this->GetEndTime() - this->GetStartTime();
	unsigned int numSamples = 60 + duration * 60.f; // һ��ȡ60��������  �о�Ӧ�üӸ�����֤durationΪ0�����Ľ��Ϊ0
	mSampledFrames.resize(numSamples);

	for (unsigned int i = 0; i < numSamples; i++) {
		// ����ò������Ӧ��ʱ��
		float t = (float)i / (float)(numSamples - 1);
		float time = t * duration + this->GetStartTime();

		// �Ҹ�ʱ���֮ǰ�����һ֡
		unsigned int frameIndex = 0;
		for (int j = numFrames - 1; j >= 0; j--) {
			if (time >= this->mFrames[j].mTime) {
				frameIndex = (unsigned int)j;
				// ����Ǹ�ʱ���֮ǰ��֡�����һ֡������Ϊ�����ڶ�֡
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
	// ģ�嶨��׶Σ�ֻ��ģ���к�ģ������޹ص����ֽ��в��ң�������Щ��ģ������Ĳ��֡�
	// ģ��ʵ�����׶Σ���ģ���к�ģ������йص����ֽ��в��ң��滻ģ�����Ϊʵ�����͡�
	// mFrames��һ����ģ������йص�����,����N��ģ���������ģ�嶨��׶Σ�����������Ե�mFrames�Ĵ��ڡ�����ģ��ʵ�����׶Σ��������Ѿ��϶�mFrames��һ���ǳ�Ա����������ȥ�����в����������Ծͻᱨ���Ҳ���mFrames��
	// Ϊ�˽��������⣬��Ҫ��mFramesǰ�����this->����SeqList<T>::�������Ϳ��Ը��߱�������mFrames��һ����Ա��������Ҫ�ڻ����в��ҡ��������������ͻ���ģ��ʵ�����׶Σ���ȷ���ҵ�mFrames�Ķ��壬�����ᱨ��
	std::vector<Frame<N>>& frames = this->mFrames;
	unsigned int size = frames.size();
	if (size <= 1) return -1;

	// ����ʱ��
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