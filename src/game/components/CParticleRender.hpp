/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_PARTICLE_RENDER
#define H_GAME_COMPONENT_PARTICLE_RENDER

#include <game/CComponent.hpp>

class CParticleRender final : public CComponent
{
public:
	CParticleRender(CGameClient *pGameClient, int render) noexcept;
	~CParticleRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

protected:
	int m_Render;
};

#endif
