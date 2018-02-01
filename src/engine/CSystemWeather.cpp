/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemWeather.hpp>
#include <game/CGameClient.hpp>
#include "CGame.hpp"
#include <cstring>

#define PUDDLE_SIZE 24.0f

CSystemWeather::CSystemWeather() noexcept
{
	m_pNoise = nullptr;
	m_WeatherTick = 0L;
	m_Weather = WEATHER_SUN;
}
CSystemWeather::~CSystemWeather() noexcept
{
	if (m_pNoise)
		delete m_pNoise;
	m_pNoise = nullptr;

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSystemWeather", "Deleted");
	#endif
}

bool CSystemWeather::init(class CGameClient *pGameClient) noexcept
{
	CSystem::init(pGameClient);

	m_pNoise = new CPerlinOctave(7, 1234);
	m_WeatherTick = ups::timeGet();

	if (!m_WeatherMapTexture.create(g_Config.m_ScreenWidth, g_Config.m_ScreenHeight)
			|| !m_WeatherMapTextureTemp.create(g_Config.m_ScreenWidth, g_Config.m_ScreenHeight))
		return false;
	m_WeatherMap.setTexture(m_WeatherMapTexture.getTexture());
	m_WeatherMapTemp.setTexture(m_WeatherMapTextureTemp.getTexture());

	return true;
}

void CSystemWeather::renderWeatherRain() noexcept
{
	CSystemFx *pFxEngine = Client()->getSystem<CSystemFx>();

	sf::FloatRect screenArea;
	Client()->getViewportGlobalBounds(&screenArea, Client()->Camera());
	const int startX = screenArea.left;
	const int endX = screenArea.width;
	const int startY = screenArea.top;
	const int endY = screenArea.height;

	// Rain & Splash
	const sf::Vector2f pos = sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY));
	pFxEngine->createRain(pos, 30.0f);
}

void CSystemWeather::update(float deltaTime) noexcept
{
	if (!Client()->Controller() || !Client()->Controller()->Context())
		return;

	if (m_Weather == WEATHER_RAIN)
		renderWeatherRain();
}

const sf::Sprite& CSystemWeather::getWeatherMap(const sf::View &camera) noexcept
{
	m_WeatherMapTexture.setView(camera);
	m_WeatherMapTextureTemp.setView(m_pGameClient->getDefaultView());
	if (m_Weather == WEATHER_RAIN)
	{
		m_WeatherMapTexture.clear(sf::Color::Black);
	}

	m_WeatherMapTexture.display();

	return m_WeatherMap;
}
