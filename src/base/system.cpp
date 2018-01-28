/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "system.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <cmath>
#if defined(__unix__)
	#include <sys/time.h>
#else
    #include <windows.h>
#endif

namespace ups
{
	/* TOOL FUNCS */
	inline sf::Color hsvToRgb(float h, float s, float v) noexcept
	{
		sf::Color color;
		const int i = static_cast<const int>(std::floor(h * 6));
		const sf::Uint8 f = static_cast<const sf::Uint8>(h * 6.0f - i);
		const sf::Uint8 p = static_cast<const sf::Uint8>(v * (1.0f - s));
		const sf::Uint8 q = static_cast<const sf::Uint8>(v * (1.0f - f * s));
		const sf::Uint8 t = static_cast<const sf::Uint8>(v * (1.0f - (1.0f - f) * s));

		switch (i % 6)
		{
			case 0: color = sf::Color(v, t, p); break;
			case 1: color = sf::Color(q, v, p); break;
			case 2: color = sf::Color(p, v, t); break;
			case 3: color = sf::Color(p, q, v); break;
			case 4: color = sf::Color(t, p, v); break;
			case 5: color = sf::Color(v, p, q); break;
		}

		return color;
	}

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


	/** STRINGS **/
	int strNCaseCmp(const char *a, const char *b, const int num) noexcept
	{
	#if defined(SFML_SYSTEM_WINDOWS)
		return _strnicmp(a, b, num);
	#else
		return strncasecmp(a, b, num);
	#endif
	}

	int strCaseCmp(const char *a, const char *b) noexcept
	{
	#if defined(SFML_SYSTEM_WINDOWS)
		return _stricmp(a,b);
	#else
		return strcasecmp(a,b);
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
	#if defined(__unix__)
		vsnprintf(msg, sizeof(str)-len, fmt, args);
	#else
		_vsnprintf(msg, sizeof(str)-len, fmt, args);
	#endif
		va_end(args);

		printf("%s\n", str);
	}
}
