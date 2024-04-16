#pragma once

template<unsigned int N>
class Frame { // Hermite������ʱ�䣬һ��ֵ��һ��incoming���ߺ�һ��outcoming���߹���
public:
	float mValue[N];
	float mIn[N];
	float mOut[N];
	float mTime;
};

typedef Frame<1> ScalarFrame;
typedef Frame<3> VectorFrame;
typedef Frame<4> QuaternionFrame;