/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/CGameClient.hpp>
#include "CSimpleParticle.hpp"

/** PARTICLES **/
CSimpleParticle::CSimpleParticle(sf::BlendMode blendMode, int render, bool luminance, int shader) noexcept
: CEntity(CEntity::SIMPLE_PARTICLE)
{
    m_Pos = m_Dir = m_SizeInit = m_SizeEnd = { 0.0f, 0.0f };
    m_ColorInit = m_ColorEnd = { 255, 255, 255, 255 };
    m_Vel = 0.0f;
    m_VelRot = 0.0f;
    m_Render = render;
    m_ModeText = false;
    m_DoMove = true;
    m_BlendMode = blendMode;
    m_Timer = ups::timeGet();
    m_Disp = VECTOR_ZERO;
    m_Offset = VECTOR_ZERO;
    m_Duration = 1.0f;
    m_Luminance = luminance;
    m_pTarget = nullptr;
    m_Animated = false;
    m_AnimSize = { 0, 0 };
    m_FixedPos = false;
    m_VelType = VEL_INCREASE;
    m_Rotation = 0.0f;
    m_TextId = -1;
    m_ApplyForces = false;
    m_Collide = false;
    m_isCollide = false;
}
CSimpleParticle::~CSimpleParticle() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSimpleParticle", "Deleted");
	#endif
}

void CSimpleParticle::tick() noexcept
{
	CGame *pGame = CGame::getInstance();
	if (!m_FixedPos && pGame->Client()->isClipped(m_Pos, SCREEN_MARGIN_DESTRUCTION))
		destroy();

    if (pGame->Client()->m_Paused)
    {
    	// TODO: Restar al tiempo pasado el tiempo que se esta pausado
    	return;
    }

	const float elapsedSeconds = (float)(ups::timeGet()-m_Timer)/ups::timeFreq();
	if (elapsedSeconds >= m_Duration)
		destroy();

	// Move attached to entity
    if (m_pTarget)
    {
    	if (m_pTarget->isToDelete())
    		m_pTarget = nullptr;
    	else if (m_pTarget->getBody())
    		m_Pos = CSystemBox2D::b2ToSf(m_pTarget->getBody()->GetPosition());
    }

    // Move Step
    if (!m_Collide || (m_Collide && !m_isCollide))
    {
		if (m_VelType == VEL_LINEAL)
			m_Disp = m_Dir*m_Vel;
		else if (m_VelType == VEL_INCREASE)
			m_Disp += (m_Dir*m_Vel)*elapsedSeconds;

		m_Pos += m_Disp + m_Offset;
	    if (m_ApplyForces)
	    	m_Pos.y += GRAVITY;
    }

    // Collision
    if (m_Collide)
    {
    	const int tileIndex = pGame->Client()->Controller()->Context()->Map().getWorldTileIndex(m_Pos, pGame->Client()->Controller()->Context()->Map().getGameLayer());
    	if (tileIndex == TILE_SOLID || tileIndex == TILE_NOHOOK)
    		m_isCollide = true;
    }

    m_Rotation += m_VelRot;
}
