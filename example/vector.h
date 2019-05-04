#pragma once

#include <mylly/math/vector.h>

// -------------------------------------------------------------------------------------------------

class Vec2
{
public:
	Vec2(void) : m_vector(vec2(0.0f, 0.0f)) { }
	Vec2(float x, float y) : m_vector(vec2(x, y)) { }
	Vec2(const vec2_t &vec) : m_vector(vec) { }
	Vec2(const Vec2 &other) : m_vector(other.m_vector) { }

	// Getters/setters
	float x() const { return m_vector.x; }
	float y() const { return m_vector.y; }
	void  x(float x) { m_vector.x = x; }
	void  y(float y) { m_vector.y = y; }

	// Arithmetic operators
	Vec2 operator+(const Vec2& other) const { return vec2_add(m_vector, other.m_vector); }
	Vec2 operator-(const Vec2& other) const { return vec2_subtract(m_vector, other.m_vector); }
	Vec2 operator*(float value) const { return vec2_multiply(m_vector, value); }
	Vec2 operator/(float value) const { return vec2_multiply(m_vector, 1.0f / value); }

	Vec2 operator+=(const Vec2& other) { m_vector = vec2_add(m_vector, other.m_vector); return *this; }
	Vec2 operator-=(const Vec2& other) { m_vector = vec2_subtract(m_vector, other.m_vector); return *this; }
	Vec2 operator*=(float value) { m_vector = vec2_multiply(m_vector, value); return *this; }
	Vec2 operator/=(float value) { m_vector = vec2_multiply(m_vector, 1.0f / value); return *this; }

	float Normalize(void) { return vec2_normalize(&m_vector); }
	Vec2 Normalized(void) { return vec2_normalized(m_vector); }

	float Dot(const Vec2 &other) { return vec2_dot(m_vector, other.m_vector); }
	void Rotate(float radians) { m_vector = vec2_rotate(m_vector, radians); }

	// Cast operators
	explicit operator vec2_t() const { return m_vector; }
	vec2_t vec(void) const { return m_vector; }

private:
	vec2_t m_vector;
};

// -------------------------------------------------------------------------------------------------

class Vec3
{
public:
	Vec3(void) : m_vector(vec3(0.0f, 0.0f, 0.0f)) { }
	Vec3(float x, float y, float z) : m_vector(vec3(x, y, z)) { }
	Vec3(const vec3_t &vec) : m_vector(vec) { }
	Vec3(const Vec3 &other) : m_vector(other.m_vector) { }

	// Getters
	float x() const { return m_vector.x; }
	float y() const { return m_vector.y; }
	float z() const { return m_vector.z; }

	// Arithmetic operators
	Vec3 operator+(const Vec3& other) const { return vec3_add(m_vector, other.m_vector); }
	Vec3 operator-(const Vec3& other) const { return vec3_subtract(m_vector, other.m_vector); }
	Vec3 operator*(float value) const { return vec3_multiply(m_vector, value); }
	Vec3 operator/(float value) const { return vec3_multiply(m_vector, 1.0f / value); }

	Vec3 operator+=(const Vec3& other) { m_vector = vec3_add(m_vector, other.m_vector); return *this; }
	Vec3 operator-=(const Vec3& other) { m_vector = vec3_subtract(m_vector, other.m_vector); return *this; }
	Vec3 operator*=(float value) { m_vector = vec3_multiply(m_vector, value); return *this; }
	Vec3 operator/=(float value) { m_vector = vec3_multiply(m_vector, 1.0f / value); return *this; }

	float Normalize(void) { return vec3_normalize(&m_vector); }
	Vec3 Normalized(void) { return vec3_normalized(m_vector); }

	float Dot(const Vec3 &other) { return vec3_dot(m_vector, other.m_vector); }

	// Cast operators
	explicit operator vec3_t() const { return m_vector; }
	vec3_t vec(void) const { return m_vector; }

private:
	vec3_t m_vector;
};
