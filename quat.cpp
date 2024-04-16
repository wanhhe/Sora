#include "quat.h"
#include <cmath>

quat angleAxis(float angle, const vec3& axis) {
	vec3 norm = normalized(axis);
	float s = sinf(angle * 0.5f); // angle����ת�Ƕȣ��� q = sin(theta/2)

	// ʵ����cos(theta/2)���鲿��sin(theta/2)u
	return quat(
		norm.x * s,
		norm.y * s,
		norm.z * s,
		cosf(angle * 0.5f)
	);
}

quat fromTo(const vec3& from, const vec3& to) {
	vec3 f = normalized(from);
	vec3 t = normalized(to);

	if (f == t) { // ��������ת
		return quat();
	}
	else if (f == t * -1.0f) { // ���Ҫת���෴�����ҵ��ֱ��ʸ��������Ԫ��
		vec3 ortho = vec3(1.0, 0.0, 0.0);
		if (fabsf(f.y) < fabsf(f.x)) {
			ortho = vec3(0.0, 1.0, 0.0);
		}
		if (fabsf(f.z) < fabsf(f.y) && fabsf(f.z) < fabsf(f.x)) {
			ortho = vec3(0.0, 0.0, 1.0);
		}
		vec3 axis = normalized(cross(f, ortho));
		return quat(axis.x, axis.y, axis.z, 0); // theta/2 = 90, cos90=0 sin90=1
	}

	vec3 half = normalized(f + t);
	vec3 axis = cross(f, half); // ������תtheta�ȣ�����Ԫ��Ϊtheta/2��f x half = axis * sin(theta/2)����ʵ������Ԫ�����鲿�������þ���axis*sin(theta/2)
	return quat(
		axis.x,
		axis.y,
		axis.z,
		dot(f, half) // cos(theta/2)
	);
}

vec3 getAxis(const quat& quat) {
	// �鲿������ sin(theta/2)u������sin(theta/2)��һ�����������׼�����ɵõ�u
	return normalized(vec3(quat.x, quat.y, quat.z));
}

float getAngle(const quat& quat) {
	// ʵ�������� cos(theta/2)
	return 2.0f * acosf(quat.w);
}

quat operator+(const quat& a, const quat& b) {
	return quat(
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	);
}

quat operator-(const quat& a, const quat& b) {
	return quat(
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w
	);
}

quat operator-(const quat& a) {
	return quat(
		-a.x,
		-a.y,
		-a.z,
		-a.w
	);
}

quat operator*(const quat& a, float b) {
	return quat(
		a.x * b,
		a.y * b,
		a.z * b,
		a.w * b
	);
}

bool operator==(const quat& left, const quat& right) {
	return (fabsf(left.x - right.x) <= QUAT_EPSILON &&
		fabsf(left.y - right.y) <= QUAT_EPSILON &&
		fabsf(left.z - right.z) <= QUAT_EPSILON &&
		fabsf(left.w - right.w) <= QUAT_EPSILON);
}

bool operator!=(const quat& left, const quat& right) {
	return !(left == right);
}

bool sameOrientation(const quat& l, const quat& r) {
	// ��Ԫ����˫���������ʵ�����Ҫ��������������ж��Ƿ��ʾ��ͬ�������ת��q��-q��ʾ����ͬһ��ת
	return (
		fabsf(l.x - r.x) <= QUAT_EPSILON && // ��ת������ͬ�����
		fabsf(l.y - r.y) <= QUAT_EPSILON &&
		fabsf(l.z - r.z) <= QUAT_EPSILON &&
		fabsf(l.w - r.w) <= QUAT_EPSILON ||
		fabsf(l.x + r.x) <= QUAT_EPSILON && // ��ת�����෴�����
		fabsf(l.y + r.y) <= QUAT_EPSILON &&
		fabsf(l.z + r.z) <= QUAT_EPSILON &&
		fabsf(l.w + r.w) <= QUAT_EPSILON
		);
}

float dot(const quat& a, const quat& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w + b.w;
}

float lenSq(const quat& q) {
	return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

float len(const quat& q) {
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON) {
		return 0.0f;
	}
	return sqrtf(lenSq);
}

void normalize(quat& q) {
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON) {
		return;
	}

	float inverse_len = 1.0f / sqrtf(lenSq);
	q.x *= inverse_len;
	q.y *= inverse_len;
	q.z *= inverse_len;
	q.w *= inverse_len;
}

quat normalized(const quat& q) {
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON) {
		return quat();
	}
	float inverse_len = 1.0f / sqrtf(lenSq);
	return quat(q.x * inverse_len, q.y * inverse_len, q.z * inverse_len, q.w * inverse_len);
}

quat conjugate(const quat& q) {
	return quat(
		-q.x,
		-q.y,
		-q.z,
		q.w
	);
}

quat inverse(const quat& q) {
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON) {
		return quat();
	}
	float recip = 1.0f / lenSq;
	// qq* = ģ����ƽ�� 
	return quat(
		-q.x * recip,
		-q.y * recip,
		-q.z * recip,
		q.w * recip
	);
}

quat operator*(const quat& Q1, const quat& Q2) {
	return quat(
		Q2.x * Q1.w + Q2.y * Q1.z - Q2.z * Q1.y + Q2.w * Q1.x,
		-Q2.x * Q1.z + Q2.y * Q1.w + Q2.z * Q1.x + Q2.w * Q1.y,
		Q2.x * Q1.y - Q2.y * Q1.x + Q2.z * Q1.w + Q2.w * Q1.z,
		-Q2.x * Q1.x - Q2.y * Q1.y - Q2.z * Q1.z + Q2.w * Q1.w
	);

	// �ڶ���ʵ�֣������ڵ�һ�ֲ��õ��ú����������ܸ���һ��
	//quat result;
	//result.scalar = Q2.scalar * Q1.scalar - dot(Q2.vector, Q1.vector);
	//result.vector = (Q1.vector * Q2.scalar) + (Q2.vector * Q1.scalar) + cross(Q2.vector, Q1.vector);
	//return result;
}

vec3 operator*(const quat& q, const vec3& v) {
	// Ҫ�������v�Ѿ��ǵ�λ����
	return    q.vector * 2.0f * dot(q.vector, v) +
		v * (q.scalar * q.scalar - dot(q.vector, q.vector)) +
		cross(q.vector, v) * 2.0f * q.scalar;
}

quat mix(const quat& from, const quat& to, float t) {
	// Ҫ�������from��to�Ѿ������ڽ���Ԫ��
	return from * (1.0f - t) + to * t;
}

quat nlerp(const quat& from, const quat& to, float t) {
	// Ҫ�������from��to�Ѿ������ڽ���Ԫ��
	return normalized(from * (1.0f - t) + to * t);
}

quat operator^(const quat& q, float f) {
	// ʵ�� = cos(theta/2);    theta/2 = acos(ʵ��) 
	float angle = 2.0f * acosf(q.scalar);
	vec3 axis = normalized(q.vector);
	float halfCos = cosf(f * angle * 0.5f); // cos(t*theta/2)
	float halfSin = sinf(f * angle * 0.5f); // sin(t*theta/2)
	return quat(
		axis.x * halfSin,
		axis.y * halfSin,
		axis.z * halfSin,
		halfCos
	);
}

quat slerp(const quat& start, const quat& end, float t) {
	// // Ҫ�������start��end�Ѿ������ڽ���Ԫ����(��Ӧ���ǵ�λ������Ԫ����)

	// ���start��end�ܿ������˻���nlerp
	if (fabsf(dot(start, end)) > 1.0f - QUAT_EPSILON) {
		return nlerp(start, end, t);
	}

	// quat delta = end * inverse(start); // ���ﲻһ����!!!!!!
	// һ��Ҫ�ǵ�λ����!!!! ��������ź������
	quat delta = end * conjugate(start); // ���ﲻһ����!!!!!!
	return normalized((delta ^ t) * start);

	// �����Ǻ����Ĺ�ʽ
	//if (fabsf(dot(start, end)) > 1.0f - QUAT_EPSILON) {
	//	return nlerp(start, end, t);
	//}
	//float angle = acosf(dot(start, end));
	//float inverse_sin = 1.0f / sin(angle);
	//float q0 = sin((1.0f - t) * angle) * inverse_sin;
	//float q1 = sin(t * angle) * inverse_sin;
	//return quat(
	//	q0 * start.x + q1 * end.x,
	//	q0 * start.y + q1 * end.y,
	//	q0 * start.z + q1 * end.z,
	//	q0 * start.w + q1 * end.w
	//);
}

quat lookRotation(const vec3& direction, const vec3& up) {
	// up������ռ������ϵ�����
	vec3 f = normalized(direction); // Object Forward �������ķ���
	vec3 u = normalized(up); // Desired Up������ռ������ϵ�����
	vec3 r = cross(u, f); // Object Right����������ҷ��� ͨ����˻����������ҷ���

	u = cross(f, r); // Object Up���������ռ������ϵķ��򡣸о�������ϵ�Ļ�Ӧ����cross(r, f)?????
	// From world forward to object forward
	quat worldToObject = fromTo(vec3(0, 0, 1), f); // �����������z��ת�������z��
	// what direction is the new object up?
	vec3 objectUp = worldToObject * vec3(0, 1, 0);
	// From object up to desired up
	quat u2u = fromTo(objectUp, u);
	// Rotate to forward direction first
	// then twist to correct up
	quat result = worldToObject * u2u;
	// ���Ҫ��λ��
	return normalized(result);
}

mat4 quatToMat4(const quat& q) {
	vec3 r = q * vec3(1, 0, 0);
	vec3 u = q * vec3(0, 1, 0);
	vec3 f = q * vec3(0, 0, 1);
	return mat4(
		r.x, r.y, r.z, 0,
		u.x, u.y, u.z, 0,
		f.x, f.y, f.z, 0,
		0, 0, 0, 1
	);
}

quat mat4ToQuat(const mat4& m) {
	vec3 up = normalized(vec3(m.up.x, m.up.y, m.up.z));
	vec3 forward = normalized(vec3(m.forward.x, m.forward.y, m.forward.z));
	vec3 right = cross(up, forward);
	up = cross(forward, right);

	return lookRotation(forward, up);
}