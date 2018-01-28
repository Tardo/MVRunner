/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_SYSTEM_WEATHER
#define H_ENGINE_SYSTEM_WEATHER

#include <SFML/Graphics.hpp>
#include <engine/CSystem.hpp>
#include <game/CEntity.hpp>
#include <list>
#include "CNoise.hpp"


class CSystemWeather final : public CSystem
{
public:
	enum
	{
		WEATHER_SUN=0,
		WEATHER_RAIN,
		NUM_WEATHERS
	};

	CSystemWeather() noexcept;
	virtual ~CSystemWeather() noexcept final;

	virtual bool init(class CGameClient *pGameClient) noexcept final;
	virtual void update(float deltaTime) noexcept final;

	const sf::Sprite& getWeatherMap(const sf::View &pCamera) noexcept;
	const int getWeather() const noexcept { return m_Weather; }
	void setWeather(int weather) noexcept { m_Weather = weather; }

private:
	int m_Weather;
	unsigned long m_WeatherTick;
	sf::Vector2f m_ScreenSize;
	sf::Vector2f m_CamOffset;

	CPerlinOctave *m_pNoise;
	sf::RenderTexture m_WeatherMapTexture;
	sf::RenderTexture m_WeatherMapTextureTemp;
	sf::Sprite m_WeatherMap;
	sf::Sprite m_WeatherMapTemp;

	void renderWeatherRain() noexcept;
	void renderPuddles() noexcept;

	bool isPuddleZone(const sf::Vector2i &mapPos) noexcept;
};

#endif
