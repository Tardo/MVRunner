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
		NUM_MENUS,

		MODAL_KEY_BIND=1,
		NUM_MODALS
	};


	CMenus(CGameClient *pGameClient) noexcept;
	~CMenus() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

	void setActive(int mid, CEntity *pEnt = nullptr) noexcept;
	void setActiveModal(int mid) noexcept;
	void setActiveEntity(CEntity *pEnt) { m_pEntity = pEnt; }
	const int getActive() const noexcept { return m_ActiveMenu; }
	const int getActiveModal() const noexcept { return m_ActiveModal; }

private:
	int m_ActiveMenu;
	int m_ActiveModal;
	CEntity *m_pEntity;

	void renderMenuMain(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void renderMenuControls(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void renderMenuCredits(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void renderMenuIntro(sf::RenderTarget& target, sf::RenderStates states) const noexcept;

	void renderModalKeyBind(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
};

#endif
