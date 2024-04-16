#ifndef _H_VEC3_
#define _H_VEC3_

#define VEC3_EPSILON 0.000001f

struct vec3
{
	// 使用union关键字，既可视为数组访问，也可通过结构体单独访问其中的变量
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		float v[3];
	};
	
	inline vec3() : x(0.0f), y(0.0f), z(0.0f) {}
	inline vec3(float _num) : x(_num), y(_num), z(_num) {}
	inline vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	inline vec3(float* fv) : x(fv[0]), y(fv[1]), z(fv[2]) {}
};

vec3 operator+(const vec3& l, const vec3& r);
vec3 operator-(const vec3& l, const vec3& r);
vec3 operator*(const vec3& v, float f);
vec3 operator*(const vec3& l, const vec3& r);
float dot(const vec3& l, const vec3& r);
float lenSq(const vec3& v); // 平方空间中的长度
float len(const vec3& v);
void normalize(vec3& v);
vec3 normalized(const vec3& v);
float angle(const vec3& l, const vec3& r);
vec3 project(const vec3& a, const vec3& b); // 获得A平行于B的分量
vec3 reject(const vec3& a, const vec3& b); // 获得A垂直于B的分量
vec3 reflect(const vec3& a, const vec3& b);
vec3 cross(const vec3& l, const vec3& r);
vec3 lerp(const vec3& s, const vec3& e, float t); // 线性插值
vec3 slerp(const vec3& s, const vec3& e, float t); // 弧线(球线)上线性插值
vec3 nlerp(const vec3& s, const vec3& e, float t); // 顺便对线性插值结果进行标准化
bool operator==(const vec3& l, const vec3& r);
bool operator!=(const vec3& l, const vec3& r);

#endif
