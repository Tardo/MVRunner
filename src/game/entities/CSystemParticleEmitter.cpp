/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/CGameClient.hpp>
#include "CSystemParticleEmitter.hpp"

/** PARTICLES **/
CSystemParticleEmitter::CSystemParticleEmitter(const sf::Vector2f &worldPos, const sf::Vector2f &force, const sf::Color &color, const sf::Vector2f &size, float lifetime, float delay) noexcept
: CEntity(CEntity::SYSTEM_PARTICLE_EMITTER)
{
    m_Pos = worldPos;
    m_Force = force;
    m_Color = color;
    m_Lifetime = lifetime;
    m_Delay = delay;
    m_Timer = ups::timeGet();

    m_Shape.SetAsBox(CSystemBox2D::sfToB2(size.x), CSystemBox2D::sfToB2(size.y));
}
CSystemParticleEmitter::~CSystemParticleEmitter() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSystemParticleEmitter", "Deleted");
	#endif
}

void CSystemParticleEmitter::tick() noexcept
{
	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->isClipped(m_Pos, SCREEN_MARGIN_DESTRUCTION))
		destroy();

	CSystemBox2D *pSystemBox2D = pGame->Client()->getSystem<CSystemBox2D>();
	if (ups::timeGet() - m_Timer > m_Delay*ups::timeFreq())
	{
		pSystemBox2D->createLiquidFunParticleGroup(m_Pos, &m_Shape, b2ParticleColor(m_Color.r, m_Color.g, m_Color.b, m_Color.a), m_Lifetime, b2_waterParticle | b2_colorMixingParticle, CSystemBox2D::sfToB2(m_Force));

		m_Timer = ups::timeGet();
	}
}
