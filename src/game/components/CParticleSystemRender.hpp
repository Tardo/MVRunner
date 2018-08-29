/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_PARTICLE_SYSTEM_RENDER
#define H_GAME_COMPONENT_PARTICLE_SYSTEM_RENDER

#include <game/CComponent.hpp>

class CParticleSystemRender final : public CComponent
{
public:
	CParticleSystemRender(CGameClient *pGameClient) noexcept;
	~CParticleSystemRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

private:
	void renderParticles(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
};

#endif
