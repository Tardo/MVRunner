/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <game/CGameClient.hpp>
#include "CParticleRender.hpp"

CParticleRender::CParticleRender(int render) noexcept
: CComponent()
{
	m_Render = render;
}
CParticleRender::~CParticleRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CParticleRender", "Deleted");
	#endif
}

void CParticleRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	std::vector<CParticle*> &lParticles = Client()->Controller()->Context()->getAllParticles();
    std::vector<CParticle*>::const_iterator itPar = lParticles.cbegin();
	while (itPar != lParticles.cend())
	{
		if ((*itPar)->m_Render == m_Render)
			target.draw(*reinterpret_cast<sf::Drawable*>((*itPar)), states);
		++itPar;
	}
}
