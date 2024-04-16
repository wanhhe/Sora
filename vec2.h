#ifndef _H_VEC2_
#define _H_VEC2_

// 由于既可能使用int型又可能使用float型，故使用模板
template<typename T>
struct TVec2
{
	union {
		struct
		{
			T x;
			T y;
		};
		T v[2];
	};

	inline TVec2() : x(T(0)), y(T(0)) {} // 用T进行强制类型转换
	inline TVec2(T _x, T _y): x(_x), y(_y) {}
	inline TVec2(T* fv): x(fv[0]), y(fv[1]) {}
};

// 重命名
typedef TVec2<float> vec2;
typedef TVec2<int> ivec2;

vec2 operator+(const vec2& l, const vec2& r);
vec2 operator-(const vec2& l, const vec2& r);
vec2 operator*(const vec2& v, float f);

#endif // _H_VEC2_

