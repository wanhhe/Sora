#pragma once

#include "vec3.h"
#include "mat4.h"

#define QUAT_EPSILON 0.000001f

struct quat
{
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		struct {
			vec3 vector;
			float scalar;  // ���ｫʵ���������
		};
		float v[4];
	};

	inline quat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	inline quat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

quat angleAxis(float angle, const vec3& axis); // ������ͽǶȴ���һ����תq
quat fromTo(const vec3& from, const vec3& to); // ������ʼ������������������һ����תq
vec3 getAxis(const quat& quat);
float getAngle(const quat& quat);
quat operator+(const quat& a, const quat& b);
quat operator-(const quat& a, const quat& b);
quat operator-(const quat& a);
quat operator*(const quat& a, float b);
bool operator==(const quat& left, const quat& right); // ��������Ԫ���ĸ���������������Ϊ����ȵ�
bool operator!=(const quat& left, const quat& right);
bool sameOrientation(const quat& l, const quat& r); // �ж�������Ԫ���Ƿ��ʾ��ͬ�������ת
float dot(const quat& a, const quat& b);
float lenSq(const quat& q);
float len(const quat& q);
void normalize(quat& q);
quat normalized(const quat& q);
quat conjugate(const quat& q); // ����
quat inverse(const quat& q);
quat operator*(const quat& Q1, const quat& Q2); // ��Ԫ���˷� Q2 * Q1
vec3 operator*(const quat& q, const vec3& v); // ��Ԫ�����������(������������ת)
quat mix(const quat& from, const quat& to, float t);
quat nlerp(const quat& from, const quat& to, float t);
quat operator^(const quat& q, float f);
quat slerp(const quat& start, const quat& end, float t);
quat lookRotation(const vec3& direction, const vec3& up);
mat4 quatToMat4(const quat& q);
quat mat4ToQuat(const mat4& m);