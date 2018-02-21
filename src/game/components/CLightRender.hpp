/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_LIGHT_RENDER
#define H_GAME_COMPONENT_LIGHT_RENDER

#include <game/CComponent.hpp>
#include <game/entities/CLight.hpp>

class CLightRender final : public CComponent
{
public:
	CLightRender(CGameClient *pGameClient, int render) noexcept;
	~CLightRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

protected:
	int m_Render;

private:
	void renderLight(sf::RenderTarget& target, sf::RenderStates states, CLight *pLight) const noexcept;
};

#endif
