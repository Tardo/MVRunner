/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_BASE_SYSTEM
#define H_BASE_SYSTEM

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Config.hpp>

namespace ups
{
	sf::Color hsvToRgb(const sf::Vector3f &hsv) noexcept;
	float hueToRgb(float v1, float v2, float h) noexcept;
	sf::Color hslToRgb(const sf::Vector3f &hsl) noexcept;
	unsigned int colorToInt(const sf::Color &color) noexcept;
	sf::Color intToColor(unsigned int color) noexcept;

	sf::Int64 timeGet() noexcept;
	sf::Int64 timeFreq() noexcept;

	int strNCaseCmp(const char *a, const char *b, const int num) noexcept;
	int strCaseCmp(const char *a, const char *b) noexcept;
	char* strCopy(char *to, const char *from, unsigned int len) noexcept;

	void calcCubicBezier(
				const sf::Vector2f &start,
				const sf::Vector2f &end,
				const sf::Vector2f &startControl,
				const sf::Vector2f &endControl,
				sf::Vector2f *vpPoints,
				const unsigned int numSegments) noexcept;

	extern "C" void msgDebug(const char *sys, const char *fmt, ...);
}

#endif
