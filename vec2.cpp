#include "vec2.h"

vec2 operator+(const vec2& l, const vec2& r) { 
	return vec2(l.x + r.x, l.y + r.y); 
}

vec2 operator-(const vec2& l, const vec2& r) { 
	return vec2(l.x - r.x, l.y - r.y); 
}

vec2 operator*(const vec2& v, float f) { 
	return vec2(v.x * f, v.y * f); 
}