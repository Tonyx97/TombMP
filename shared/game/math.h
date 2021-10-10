#pragma once

#include <stdint.h>
#include <numbers>
#include <cmath>

template <class T = int>
struct vec3
{
	T x = {},
	  y = {},
	  z = {};

	vec3() {}
	vec3(T x, T y, T z) : x(x), y(y), z(z) {}

	bool operator != (const vec3<T>& v) { return x != v.x || y != v.y || z != v.z; }

	vec3 operator + (const vec3& v)		{ return vec3(x + v.x, y + v.y, z + v.z); }
	vec3 operator - (const vec3& v)		{ return vec3(x - v.x, y - v.y, z - v.z); }
	vec3 operator += (T v)				{ return vec3(x + v, y + v, z + v); }
	vec3 operator -= (T v)				{ return vec3(x - v, y - v, z - v); }

	T distance(const vec3& v) { return static_cast<T>(std::sqrtf(float((v.x - x) * (v.x - x) + (v.y - y) * (v.y - y) + (v.z - z) * (v.z - z)))); }

	template <typename Tx = T, std::enable_if_t<sizeof(Tx) == 4>* = nullptr>
	vec3 interpolate(const vec3& v, float factor)
	{
		x = static_cast<T>(std::lerp(float(x), float(v.x), factor));
		y = static_cast<T>(std::lerp(float(y), float(v.y), factor));
		z = static_cast<T>(std::lerp(float(z), float(v.z), factor));

		return *this;
	}

	template <typename Tx = T, std::enable_if_t<sizeof(Tx) == 2>* = nullptr>
	vec3 interpolate(const vec3& v, float factor)
	{
		const T x_diff = x - v.x,
				y_diff = y - v.y,
				z_diff = z - v.z;

		x = static_cast<T>(std::lerp(std::fabs(float(x_diff)) > 32768.f / 2.f ? float(-x) : float(x), float(v.x), factor));
		y = static_cast<T>(std::lerp(std::fabs(float(y_diff)) > 32768.f / 2.f ? float(-y) : float(y), float(v.y), factor));
		z = static_cast<T>(std::lerp(std::fabs(float(z_diff)) > 32768.f / 2.f ? float(-z) : float(z), float(v.z), factor));

		return *this;
	}
};

template <class T>
struct quat
{
	T x = {},
	  y = {},
	  z = {},
	  w = {};

	quat() {}
	quat(T yaw, T pitch, T roll)
	{
		double cy = std::cos(yaw * 0.5);
		double sy = std::sin(yaw * 0.5);
		double cp = std::cos(pitch * 0.5);
		double sp = std::sin(pitch * 0.5);
		double cr = std::cos(roll * 0.5);
		double sr = std::sin(roll * 0.5);

		w = cr * cp * cy + sr * sp * sy;
		x = sr * cp * cy - cr * sp * sy;
		y = cr * sp * cy + sr * cp * sy;
		z = cr * cp * sy - sr * sp * cy;
	}

	~quat() {}

	inline double length()
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	void normalize()
	{
		const auto norm = length();

		x /= norm;
		y /= norm;
		z /= norm;
	}

	void slerp(const quat& target, double factor)
	{
		float dotproduct = x * target.x + y * target.y + z * target.z + w * target.w;
		float theta, st, sut, sout, coeff1, coeff2;

		// algorithm adapted from Shoemake's paper
		factor = factor / 2.0;

		theta = (float)std::acos(dotproduct);
		if (theta < 0.0) theta = -theta;

		st = (float)std::sin(theta);
		sut = (float)std::sin(factor * theta);
		sout = (float)std::sin((1 - factor) * theta);
		coeff1 = sout / st;
		coeff2 = sut / st;

		x = coeff1 * x + coeff2 * target.x;
		y = coeff1 * y + coeff2 * target.y;
		z = coeff1 * z + coeff2 * target.z;
		w = coeff1 * w + coeff2 * target.w;

		normalize();
	}

	vec3<double> to_euler()
	{
		vec3<double> angles;

		// roll (x-axis rotation)

		double sinr_cosp = 2 * (w * x + y * z);
		double cosr_cosp = 1 - 2 * (x * x + y * y);

		angles.z = std::atan2(sinr_cosp, cosr_cosp);

		// pitch (y-axis rotation)

		double sinp = 2 * (w * y - z * x);

		if (std::abs(sinp) >= 1)
			angles.x = std::copysign(std::numbers::pi / 2, sinp); // use 90 degrees if out of range
		else angles.x = std::asin(sinp);

		// yaw (z-axis rotation)

		double siny_cosp = 2 * (w * z + x * y);
		double cosy_cosp = 1 - 2 * (y * y + z * z);

		angles.y = std::atan2(siny_cosp, cosy_cosp);

		return angles;
	}
};

using int_vec3 = vec3<>;
using short_vec3 = vec3<int16_t>;
using u8_vec3 = vec3<uint8_t>;
using double_quat = quat<double>;

struct vec3d
{
	int_vec3 pos;
	short_vec3 rot;
};

struct game_vec3d
{
	int_vec3 pos;
	short_vec3 rot;
	int16_t room, box;
};