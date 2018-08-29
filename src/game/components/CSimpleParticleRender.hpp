/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_PARTICLE_RENDER
#define H_GAME_COMPONENT_PARTICLE_RENDER

#include <game/CComponent.hpp>
#include "../entities/CSimpleParticle.hpp"


class CSimpleParticleRender final : public CComponent
{
public:
	CSimpleParticleRender(CGameClient *pGameClient, int render) noexcept;
	~CSimpleParticleRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

protected:
	int m_Render;

private:
	void renderSimpleParticle(sf::RenderTarget& target, sf::RenderStates states, CSimpleParticle *pSimpleParticle) const noexcept;
};

#endif
