/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_FLUID_RENDER
#define H_GAME_COMPONENT_FLUID_RENDER

#include <game/CComponent.hpp>

class CFluidRender final : public CComponent
{
public:
	CFluidRender(CGameClient *pGameClient) noexcept;
	~CFluidRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

private:
	void renderWater(sf::RenderTarget& target, sf::RenderStates states, CEntity *pEntity) const noexcept;
};

#endif
