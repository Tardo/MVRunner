/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "system.hpp"
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cmath>
#if defined(SFML_SYSTEM_LINUX)
	#include <sys/time.h>
#elif defined(SFML_SYSTEM_WINDOWS)
    #include <windows.h>
#endif

namespace ups
{
	/* RENDER TOOLS */
	// Original Source: https://github.com/SFML/SFML/wiki/Source%3A-cubic-bezier-curve
	void calcCubicBezier(
				const sf::Vector2f &start,
				const sf::Vector2f &end,
				const sf::Vector2f &startControl,
				const sf::Vector2f &endControl,
				sf::Vector2f *vpPoints,
				const unsigned int numSegments) noexcept
	{
		float p = 1.f / (numSegments-2);
		const float q = p;
		vpPoints[0] = start;
		vpPoints[numSegments-1] = end;
		for (size_t i = 1; i < numSegments-1; ++i, p += q)
			vpPoints[i] = p * p * p * (end + 3.f * (startControl - endControl) - start) +
						  3.f * p * p * (start - 2.f * startControl + endControl) +
						  3.f * p * (startControl - start) + start;
	}

	sf::Color hsvToRgb(const sf::Vector3f &hsv) noexcept
	{
		sf::Color color;
		const int i = static_cast<const int>(std::floor(hsv.x * 6.0f));
		const sf::Uint8 f = static_cast<const sf::Uint8>(hsv.x * 6.0f - i);
		const sf::Uint8 p = static_cast<const sf::Uint8>(hsv.z * (1.0f - hsv.y));
		const sf::Uint8 q = static_cast<const sf::Uint8>(hsv.z * (1.0f - f * hsv.y));
		const sf::Uint8 t = static_cast<const sf::Uint8>(hsv.z * (1.0f - (1.0f - f) * hsv.y));

		switch (i % 6)
		{
			case 0: color = sf::Color(hsv.z, t, p); break;
			case 1: color = sf::Color(q, hsv.z, p); break;
			case 2: color = sf::Color(p, hsv.z, t); break;
			case 3: color = sf::Color(p, q, hsv.z); break;
			case 4: color = sf::Color(t, p, hsv.z); break;
			case 5: color = sf::Color(hsv.z, p, q); break;
		}

		return color;
	}

	float hueToRgb(float v1, float v2, float h) noexcept
	{
		if(h < 0.0f) h += 1;
		if(h > 1.0f) h -= 1;
		if((6.0f * h) < 1.0f) return v1 + (v2 - v1) * 6.0f * h;
		if((2.0f * h) < 1.0f) return v2;
		if((3.0f * h) < 2.0f) return v1 + (v2 - v1) * ((2.0f/3.0f) - h) * 6.0f;
		return v1;
	}

	sf::Color hslToRgb(const sf::Vector3f &hsl) noexcept
	{
		if(hsl.y == 0.0f)
			return sf::Color(hsl.z*255.0f, hsl.z*255.0f, hsl.z*255.0f);
		else
		{
			float v2 = hsl.z < 0.5f ? hsl.z * (1.0f + hsl.y) : (hsl.z+hsl.y) - (hsl.y*hsl.z);
			float v1 = 2.0f * hsl.z - v2;

			return sf::Color(hueToRgb(v1, v2, hsl.x + (1.0f/3.0f))*255.0f, hueToRgb(v1, v2, hsl.x)*255.0f, hueToRgb(v1, v2, hsl.x - (1.0f/3.0f))*255.0f);
		}
	}

	/** TIME **/
	/* Original code from teeworlds source */
	sf::Int64 timeGet() noexcept
	{
	#if defined(SFML_SYSTEM_LINUX)
		struct timeval val;
		gettimeofday(&val, NULL);
		return (sf::Int64)val.tv_sec*(sf::Int64)1000000+(sf::Int64)val.tv_usec;
	#elif defined(SFML_SYSTEM_WINDOWS)
		static sf::Int64 last = 0;
		sf::Int64 t;
		QueryPerformanceCounter((PLARGE_INTEGER)&t);
		if(t<last) /* for some reason, QPC can return values in the past */
			return last;
		last = t;
		return t;
	#else
		#error not implemented
	#endif
	}

	sf::Int64 timeFreq() noexcept
	{
	#if defined(SFML_SYSTEM_LINUX)
		return 1000000;
	#elif defined(SFML_SYSTEM_WINDOWS)
		sf::Int64 t;
		QueryPerformanceFrequency((PLARGE_INTEGER)&t);
		return t;
	#else
		#error not implemented
	#endif
	}

	/** LOGGING **/
	/* Original code from teeworlds source */
	extern "C" void msgDebug(const char *sys, const char *fmt, ...)
	{
		va_list args;
		char str[1024*4];
		char *msg;
		int len;

		snprintf(str, sizeof(str), "[%08x][%s]: ", (int)time(0), sys);
		len = strlen(str);
		msg = (char *)str + len;

		va_start(args, fmt);
	#if defined(SFML_SYSTEM_LINUX)
		vsnprintf(msg, sizeof(str)-len, fmt, args);
	#else
		_vsnprintf(msg, sizeof(str)-len, fmt, args);
	#endif
		va_end(args);

		printf("%s\n", str);
	}
}
