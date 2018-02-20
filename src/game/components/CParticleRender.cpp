/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <game/CGameClient.hpp>
#include "CParticleRender.hpp"

CParticleRender::CParticleRender(CGameClient *pGameClient, int render) noexcept
: CComponent(pGameClient)
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
	if (Client()->getRenderMode() != CGameClient::RENDER_NORMAL)
		return;

	target.setView(Client()->Camera());

	std::vector<CParticle*> &lParticles = Client()->Controller()->Context()->getParticles();
    std::vector<CParticle*>::const_iterator itPar = lParticles.cbegin();
	while (itPar != lParticles.cend())
	{
		if ((*itPar)->m_Render == m_Render)
			target.draw(*reinterpret_cast<sf::Drawable*>((*itPar)), states);
		++itPar;
	}
}
