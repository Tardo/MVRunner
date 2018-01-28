/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/* Based on Magnus Auvinen teeworlds source */
#ifndef BASE_MATH_H
#define BASE_MATH_H

#include <cstdlib>
#include <cmath>
#include <SFML/Graphics.hpp>

#define I_PI 			3.14f	// Imprecise PI
#define VECTOR_ZERO		sf::Vector2f(0.0f, 0.0f)

namespace upm
{
	/* HELPERS */
	template <typename T>
	inline T clamp(T val, T min, T max) noexcept
	{
		if(val < min)
			return min;
		if(val > max)
			return max;
		return val;
	}

	inline int roundToInt(float f) noexcept
	{
		if(f > 0)
			return (int)(f+0.5f);
		return (int)(f-0.5f);
	}

	template<typename T, typename TB>
	inline T mix(const T a, const T b, TB amount) noexcept
	{
		return a + (b-a)*amount;
	}

	template <typename T> inline T min(T a, T b) noexcept { return a<b?a:b; }
	template <typename T> inline T max(T a, T b) noexcept { return a>b?a:b; }

	inline double fade(double t) noexcept
	{
		return t * t * t * (t * (t * 6 - 15) + 10);
	}

	inline double lerp(double t, double a, double b) noexcept
	{
		return a + t * (b - a);
	}

	inline double grad(int hash, double x, double y, double z) noexcept
	{
		int h = hash & 15;
		double u = h < 8 ? x : y;
		double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	template <typename T>
	inline T calcPerc(T value, T max) noexcept { return (value * 100.0) / max; }

	template <typename T>
	inline T calcPercValue(T perc, T max, bool inverse) noexcept
	{
		const T res = perc * max * 0.01;
		return inverse?max - res:res;
	}

	/* RANDOM */
	inline int randInt(int low, int high) noexcept
	{
		return rand() % (high - low + 1) + low;
	}

	inline float floatRand(float low,float high) noexcept
	{
		return (rand()/(float)(RAND_MAX)) * (high - low) + low;
	}

	/* ANGLES */
	inline float normalizeAngle(float radians) noexcept
	{
		return std::fmod(I_PI*2.0f + std::fmod(radians, I_PI*2.0f), I_PI*2.0f);
	}

	inline float degToRad(float angle) noexcept
	{
		return angle * I_PI/180.0f;
	}

	inline float radToDeg(float radians) noexcept
	{
		return radians * 180.0f/I_PI;
	}

	inline sf::Vector2f degToDir(float angle) noexcept
	{
		angle = degToRad(angle);
		return sf::Vector2f(cosf(angle), sinf(angle));
	}

	inline bool isInsideAngle(float radians, float minRadians, float maxRadians) noexcept
	{
		radians = normalizeAngle(radians);
		minRadians = normalizeAngle(minRadians);
		maxRadians = normalizeAngle(maxRadians);

		if(minRadians < maxRadians)
			return minRadians <= radians && maxRadians >= radians;
		return minRadians <= radians || maxRadians >= radians;
	}


	/* VECTOR */
	// FIXME: La forma mas costosa... JAJA!
	inline float vectorLength(const sf::Vector2f& v) noexcept
	{
		return sqrtf(v.x*v.x + v.y*v.y);
	}

	inline float vectorLength(const sf::Vector2i& v) noexcept
	{
		return sqrtf(static_cast<float>(v.x*v.x + v.y*v.y));
	}

	inline sf::Vector2f vectorNormalize(const sf::Vector2f& source) noexcept
	{
		float length = vectorLength(source);
		if (length != 0.0f)
			return sf::Vector2f(source.x / length, source.y / length);
		else
			return source;
	}

	inline sf::Vector2i vectorNormalize(const sf::Vector2i& source) noexcept
	{
		const float length = vectorLength(source);
		if (length != 0.0f)
			return sf::Vector2i(static_cast<int>(source.x / length), static_cast<int>(source.y / length));
		else
			return source;
	}

	inline float vectorAngle(const sf::Vector2f& Dir) noexcept
	{
		return radToDeg(normalizeAngle(atan2(Dir.y,Dir.x)));
	}

	inline void vectorRotate(const sf::Vector2f &Center, sf::Vector2f *pPoint, float RotationRadians) noexcept
	{
		float x = pPoint->x - Center.x;
		float y = pPoint->y - Center.y;
		pPoint->x = x * cosf(RotationRadians) - y * sinf(RotationRadians) + Center.x;
		pPoint->y = x * sinf(RotationRadians) + y * cosf(RotationRadians) + Center.y;
	}

	inline float vectorDot(const sf::Vector2f &vectorA, const sf::Vector2f &vectorB) noexcept
	{
		return vectorA.x*vectorB.x + vectorA.y*vectorB.y;
	}

	inline float vectorCross(const sf::Vector2f &vectorA, const sf::Vector2f &vectorB) noexcept
	{
		return vectorA.x*vectorB.y - vectorB.x*vectorA.y;
	}
}

#endif // BASE_MATH_H
