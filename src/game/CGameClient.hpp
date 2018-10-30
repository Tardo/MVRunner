/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_GAMECLIENT
#define H_GAME_GAMECLIENT

#include <SFML/Graphics.hpp>
#include <tmxparser/TmxMap.h>
#include <base/system.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CConfig.hpp>
#include <engine/ISystem.hpp>
#include <engine/CSystemSound.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/components/CMapRender.hpp>
#include <game/components/CMenus.hpp>
#include <game/components/CPlayerRender.hpp>
#include <game/components/CItemRender.hpp>
#include <game/components/CControls.hpp>
#include <game/components/CCamera.hpp>
#include <game/components/CDebuggerRender.hpp>
#include <game/components/CLightRender.hpp>
#include <game/components/CParticleSystemRender.hpp>
#include <game/components/CSimpleParticleRender.hpp>
#include <game/components/CHUD.hpp>
#include <game/CController.hpp>
#include <game/CEntity.hpp>
#include <game/CUI.hpp>
#include <Zpg/Zpg.hpp>
#include <cstdlib>
#include <list>
#include <deque>

#define SCREEN_MARGIN_DRAW			128.0f
#define SCREEN_MARGIN_DESTRUCTION	MARGIN_CREATE_OBJECTS + 250.0f
#define BROADCAST_MAX_LENGTH	64
#define HELP_TEXT_MAX_LENGTH	128

enum
{
	RENDER_MODE_NORMAL=0,
	RENDER_MODE_LIGHTING,
	RENDER_MODE_LIQUID,
	RENDER_MODE_NORMALMAP,
};

enum GameType {
	GAMETYPE_MENU=0,
	GAMETYPE_MAIN,
};

class CGameClient final : public sf::RenderWindow
{
public:
	CGameClient() noexcept;
	~CGameClient() noexcept;

	CController* Controller() const noexcept { return m_pGameController; }
	CAssetManager& Assets() noexcept { return m_AssetManager; }
	CMenus& Menus() noexcept { return m_Menus; }
	CControls& Controls() noexcept { return m_Controls; }
	CUI& UI() noexcept { return m_UI; }
	CCamera& Camera() noexcept { return m_Camera; }
	Zpg& Storage() noexcept { return m_Zpg; }
	CLightRender Lights() noexcept { return m_LightRender; }

	template<class T>
	T* getSystem() noexcept
	{
		std::deque<ISystem*>::iterator itEng = m_vpSystems.begin();
		while (itEng != m_vpSystems.end())
		{
			if (dynamic_cast<T*>((*itEng)))
				return reinterpret_cast<T*>((*itEng));
			++itEng;
		}
		return 0x0;
	}

	const int getNumComponents() const noexcept { return m_vpComponents.size(); }
	const int getNumSystems() const noexcept { return m_vpSystems.size(); }

	void reset() noexcept;
	void run() noexcept;
	bool initializeGameMode(GameType gametype) noexcept;

	void getViewportGlobalBounds(sf::FloatRect *pRect, const sf::View &view, float margin=0.0f) noexcept;
	sf::View& getHudView() noexcept { return m_ViewHud; }

	bool isClipped(const sf::Vector2f &worldPos, float margin=0.0f) noexcept;
	bool isClipped(const std::vector<sf::Vector2f> &points, float margin=0.0f) noexcept;
	bool isClipped(const sf::FloatRect &worldRect, float margin) noexcept;

	unsigned int getFPS() const noexcept { return m_FPS; }
	float getDeltaTime() const noexcept { return m_DeltaTime; }
	const float getElapsedTime() const noexcept { return (ups::timeGet()-m_TimerGame)/(float)ups::timeFreq(); }

	int getRenderMode() const noexcept { return m_RenderMode; }
	void setRenderMode(int mode) noexcept;

	bool canAdd100Hz() const noexcept { return m_Add100Hz; }
	bool canAdd50Hz() const noexcept { return m_Add50Hz; }

	void showBroadcastMessage(const char *pMsg, float duration) noexcept;
	void showHelpMessage(const char *pMsg) noexcept;

	void runPlayerTime(bool state) noexcept;
	bool isPlayerTimerRun() const noexcept { return m_TimerPlayerRun; }

	bool m_Debug;
	bool m_Paused;
	unsigned int m_FPS;
	unsigned int m_MinFPS;

	sf::Int64 m_TimerBroadcast;
	float m_BroadcastDuration;
	char m_aBroadcastMsg[BROADCAST_MAX_LENGTH];
	char m_aHelpMsg[HELP_TEXT_MAX_LENGTH];

	sf::Int64 m_TimerPlayerStart;
	sf::Int64 m_TimerPlayerEnd;

protected:
	void renderComponentsPhase(int mode) noexcept;
	void renderLoadAssets() noexcept;
	void doUpdate() noexcept;
	void doRender() noexcept;

private:
	bool init() noexcept;

	std::deque<CComponent*> m_vpComponents;
	std::deque<ISystem*> m_vpSystems;

	Zpg m_Zpg;

	sf::View m_ViewHud;
	CAssetManager m_AssetManager;
	CController *m_pGameController;

	CCamera m_Camera;
	CMapRender m_MapRenderBack;
	CMapRender m_MapRenderFront;
	CMenus m_Menus;
	CPlayerRender m_PlayerRender;
	CItemRender m_ItemRender;
	CSimpleParticleRender m_SimpleParticleRenderBack;
	CSimpleParticleRender m_SimpleParticleRenderFront;
	CSimpleParticleRender m_SimpleParticleRenderForeground;
	CParticleSystemRender m_SimpleParticleSystemRender;
	CControls m_Controls;
	CDebuggerRender m_DebuggerRender;
	CLightRender m_LightRender;
	CHUD m_HUD;

	CUI m_UI;

	CSystemBox2D m_SystemBox2D;
	CSystemSound m_SystemSound;

	sf::RenderTexture m_RenderPhaseTexture;
	sf::RenderTexture m_RenderPhaseTextureFinal;
	sf::RectangleShape m_RenderPhase;
	int m_RenderMode;

	sf::Int64 m_TimerGame;

	bool m_TimerPlayerRun;

	float m_DeltaTime;
	sf::Clock m_Timer100Hz;
	sf::Clock m_Timer50Hz;
	bool m_Add100Hz;
	bool m_Add50Hz;
};

#endif
