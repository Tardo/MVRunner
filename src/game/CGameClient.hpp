/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_GAMECLIENT
#define H_GAME_GAMECLIENT

#include <SFML/Graphics.hpp>
#include <tmxparser/TmxMap.h>
#include <base/system.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CConfig.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemLight.hpp>
#include <engine/CSystemSound.hpp>
#include <engine/CSystemWeather.hpp>
#include <engine/CSystem.hpp>
#include <game/components/CMapRender.hpp>
#include <game/components/CMenus.hpp>
#include <game/components/CUI.hpp>
#include <game/CController.hpp>
#include <game/CEntity.hpp>
#include <Zpg/Zpg.hpp>
#include <cstdlib>
#include <list>
#include <deque>

#define SCREEN_MARGIN_SPAWN_BOT	255.0f

#define BROADCAST_MAX_LENGTH	64
#define HELP_TEXT_MAX_LENGTH	128

class CGameClient final : public sf::RenderWindow
{
public:
	CGameClient() noexcept;
	~CGameClient() noexcept;

	CController* Controller() const noexcept { return m_pGameController; }
	CMapRender& MapRender() noexcept { return m_MapRender; }
	CAssetManager& Assets() noexcept { return m_AssetManager; }
	CMenus& Menus() noexcept { return m_Menus; }
	CUI& UI() noexcept { return m_UI; }
	CCamera& Camera() noexcept { return m_Camera; }
	Zpg& Storage() noexcept { return m_Zpg; }

	template<class T>
	T* getSystem() noexcept
	{
		std::deque<CSystem*>::iterator itEng = m_vpSystems.begin();
		while (itEng != m_vpSystems.end())
		{
			if (dynamic_cast<T*>((*itEng)))
				return reinterpret_cast<T*>((*itEng));
			++itEng;
		}
		return 0x0;
	}

	void reset() noexcept;
	void run() noexcept;
	bool initializeGameMode(const char *pGameType) noexcept;

	void getViewportGlobalBounds(sf::FloatRect *pRect, const sf::View &view, float margin=0.0f) noexcept;
	sf::View& getHudView() noexcept { return m_ViewHud; }

	bool isClipped(const sf::Vector2f &worldPos, float margin=0.0f) noexcept;
	bool isClipped(const std::vector<sf::Vector2f> &points, float margin=0.0f) noexcept;
	bool isClipped(const sf::FloatRect &worldRect, float margin) noexcept;

	void showBroadcastMessage(const char *pMsg, float duration) noexcept;
	void showHelpMessage(const char *pMsg) noexcept;

	unsigned int getFPS() const noexcept { return m_FPS; }
	float getDeltaTime() const noexcept { return m_DeltaTime; }
	const float getElapsedTime() const noexcept { return (ups::timeGet()-m_TimerGame)/(float)ups::timeFreq(); }

	sf::Vector2i m_MousePosition;
	bool m_Debug;
	bool m_Paused;

private:
	bool init() noexcept;

	void drawHUD() noexcept;
	void drawCursor() noexcept;
	void drawDebugInfo() noexcept;

	std::deque<CComponent*> m_vpComponents;
	std::deque<CSystem*> m_vpSystems;

	Zpg m_Zpg;

	sf::View m_ViewHud;
	CAssetManager m_AssetManager;

	CController *m_pGameController;

	CMapRender m_MapRender;
	CMenus m_Menus;
	CUI m_UI;
	CCamera m_Camera;

	CSystemBox2D m_SystemBox2D;
	CSystemFx m_SystemFx;
	CSystemLight m_SystemLight;
	CSystemWeather m_SystemWeather;
	CSystemSound m_SystemSound;

	sf::Int64 m_TimerGame;

	sf::Int64 m_TimerBroadcast;
	float m_BroadcastDuration;
	char m_aBroadcastMsg[BROADCAST_MAX_LENGTH];
	char m_aHelpMsg[HELP_TEXT_MAX_LENGTH];

	unsigned int m_FPS;
	unsigned int m_MinFPS;
	float m_DeltaTime;
};

#endif
