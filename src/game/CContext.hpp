/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_CONTEXT
#define H_GAME_CONTEXT

#include <game/CPlayer.hpp>
#include "CMap.hpp"
#include <vector>

enum
{
	WEATHER_NORMAL=0,
	WEATHER_RAIN,
	WEATHER_SNOW,
};

class CContext final
{
public:
	CContext() noexcept;
	~CContext() noexcept;

	CMap& Map() { return m_Map; }

	int addEntity(CEntity *pEntity) noexcept;
	CEntity* getEntity(sf::Uint64 id) const noexcept
	{
		std::vector<CEntity*>::const_iterator cit = m_vpEntities.cbegin();
		while (cit != m_vpEntities.cend())
		{
			if ((*cit)->getID() == id)
				return (*cit);
			++cit;
		}
		return nullptr;
	}
	std::vector<CEntity*>& getEntities() noexcept { return m_vpEntities; }

	void tick() noexcept;
	void clearTrash() noexcept;

	void setWeather(int weather) { m_Weather = weather; }
	int getWeather() const { return m_Weather; }

	sf::Color getColorClear() const noexcept { return m_ColorClear; }
	sf::Color getColorShadow() const noexcept { return m_ColorShadow; }

	CPlayer* getPlayer() noexcept { return m_pPlayer; }

	sf::Int64 getTimerStorm() const { return m_TimerStorm; }
	void setTimerStorm(sf::Int64 time) { m_TimerStorm = time; }

protected:
	sf::Uint64 m_GID;

private:
	std::vector<CEntity*> m_vpEntities;

	CPlayer *m_pPlayer;
	CMap m_Map;
	int m_Weather;
	sf::Int64 m_TimerStorm;

	sf::Color m_ColorClear;
	sf::Color m_ColorShadow;
};

#endif
