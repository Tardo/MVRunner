/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_BASE_SYSTEM
#define H_BASE_SYSTEM

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Config.hpp>
#include <cstring>

namespace ups
{
	/* RENDER TOOLS */
	void calcCubicBezier(
			const sf::Vector2f &start,
			const sf::Vector2f &end,
			const sf::Vector2f &startControl,
			const sf::Vector2f &endControl,
			sf::Vector2f *vpPoints,
			const unsigned int numSegments) noexcept;
	sf::Color hsvToRgb(const sf::Vector3f &hsv) noexcept;
	float hueToRgb(float v1, float v2, float h) noexcept;
	sf::Color hslToRgb(const sf::Vector3f &hsl) noexcept;
	inline sf::Color intToColor(unsigned int color) noexcept
	{
		return sf::Color((color>>24) & 255, (color>>16) & 255, (color>>8) & 255, color & 255);
	}
	inline unsigned int colorToInt(const sf::Color &color) noexcept
	{
		return ((color.r&255)<<24) + ((color.g&255)<<16) + ((color.b&255)<<8) + (color.a&255);
	}

	/** TIME **/
	sf::Int64 timeGet() noexcept;
	sf::Int64 timeFreq() noexcept;

	/* STRING */
	inline int strNCaseCmp(const char *a, const char *b, unsigned long num) noexcept
	{
		#if defined(SFML_SYSTEM_LINUX)
			return strncasecmp(a, b, num);
		#elif defined(SFML_SYSTEM_WINDOWS)
			return _strnicmp(a, b, num);
		#else
			#error not implemented
		#endif
	}

	inline char* strCopy(char *to, const char *from, unsigned int size) noexcept
	{
	#if defined(SFML_SYSTEM_LINUX)
		return strncpy(to, from, size);
	#elif defined(SFML_SYSTEM_WINDOWS)
		return strncpy(to, from, size);
	#else
		#error not implemented
	#endif
	}

	inline int strLen(const char *str) noexcept
	{
	#if defined(SFML_SYSTEM_LINUX)
		return strlen(str);
	#elif defined(SFML_SYSTEM_WINDOWS)
		return strlen(str);
	#else
		#error not implemented
	#endif
	}

	inline char* strNCat(char *to, const char *from, unsigned int size) noexcept
	{
	#if defined(SFML_SYSTEM_LINUX)
		return strncat(to, from, size);
	#elif defined(SFML_SYSTEM_WINDOWS)
		return strncat(to, from, size);
	#else
		#error not implemented
	#endif
	}

	/** LOGGING **/
	extern "C" void msgDebug(const char *sys, const char *fmt, ...);
}

#endif
