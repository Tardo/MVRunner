/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CAssetManager.hpp>
#include <engine/CGame.hpp>
#include <game/CGameClient.hpp>
#include "CParticleSystemRender.hpp"


CParticleSystemRender::CParticleSystemRender(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{ }
CParticleSystemRender::~CParticleSystemRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CParticleRender", "Deleted");
	#endif
}

void CParticleSystemRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	target.setView(Client()->Camera());

	if (Client()->getRenderMode() == RENDER_MODE_LIQUID)
		renderParticles(target, states);
}

void CParticleSystemRender::renderParticles(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CSystemBox2D *pSystemBox2D = Client()->getSystem<CSystemBox2D>();
	for (b2ParticleSystem *pParticleSystem = pSystemBox2D->getWorld()->GetParticleSystemList(); pParticleSystem; pParticleSystem = pParticleSystem->GetNext())
	{
		b2Vec2 *pPos = pParticleSystem->GetPositionBuffer();
		b2ParticleColor *pColor = pParticleSystem->GetColorBuffer();
		const float particleSize = CSystemBox2D::b2ToSf(pParticleSystem->GetRadius());
		sf::CircleShape Shape;
		for (int i=0; i<pParticleSystem->GetParticleCount(); ++i)
		{
			Shape.setPosition(CSystemBox2D::b2ToSf(*(pPos+i)));
			Shape.setRadius(particleSize);
			Shape.setOrigin(particleSize/2.0f, particleSize/2.0f);
			Shape.setFillColor(sf::Color((pColor+i)->r, (pColor+i)->g, (pColor+i)->b, (pColor+i)->a));
			target.draw(Shape, states);
		}
	}
}
