#pragma once

template<unsigned int N>
class Frame { // Hermite曲线由时间，一个值，一个incoming切线和一个outcoming切线构成
public:
	float mValue[N];
	float mIn[N];
	float mOut[N];
	float mTime;
};

typedef Frame<1> ScalarFrame;
typedef Frame<3> VectorFrame;
typedef Frame<4> QuaternionFrame;