/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_MENUS
#define H_GAME_COMPONENT_MENUS

#include <game/CComponent.hpp>

class CMenus final : public CComponent
{
public:
	enum
	{
		NONE=0,
		MAIN,
		CREDITS,
		CONTROLS,
		NUM_MENUS
	};


	CMenus(CGameClient *pGameClient) noexcept;
	~CMenus() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

	void setActive(int mid, CEntity *pEnt = nullptr) noexcept;
	void setActiveEntity(CEntity *pEnt) { m_pEntity = pEnt; }
	const int getActive() const noexcept { return m_ActiveMenu; }

private:
	int m_ActiveMenu;
	CEntity *m_pEntity;

	void renderMenuMain(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void renderMenuControls(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void renderMenuCredits(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void renderMenuIntro(sf::RenderTarget& target, sf::RenderStates states) const noexcept;

	void clipEnable(const sf::RenderTarget &target, const sf::FloatRect &area) const noexcept
	{
		clipEnable(target, area.left, area.top, area.width, area.height);
	}
	void clipEnable(const sf::RenderTarget &target, int x, int y, int w, int h) const noexcept;
	void clipDisable() const noexcept;
};

#endif
