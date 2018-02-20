/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_ITEM_RENDER
#define H_GAME_COMPONENT_ITEM_RENDER

#include <game/CComponent.hpp>

class CItemRender final : public CComponent
{
public:
	CItemRender(CGameClient *pGameClient) noexcept;
	~CItemRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

private:
	void renderHitBox(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept;
	void renderProjectile(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept;
	void renderPrimitive(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept;
};

#endif
