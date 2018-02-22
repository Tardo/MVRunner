/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <game/CGameClient.hpp>
#include "CParticle.hpp"

/** PARTICLES **/
CParticle::CParticle(sf::BlendMode blendMode, int render, bool luminance, int shader) noexcept
: CEntity(CEntity::PARTICLE)
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
}
CParticle::~CParticle() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CParticle", "Deleted");
	#endif
}

void CParticle::tick() noexcept
{
	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->isClipped(m_Pos, SCREEN_MARGIN_DESTRUCTION))
		destroy();

    if (pGame->Client()->m_Paused)
    {
    	// TODO: Restar al tiempo pasado el tiempo que se esta pausado
    	return;
    }

	const float elapsedSeconds = (float)(ups::timeGet()-m_Timer)/ups::timeFreq();
	if (elapsedSeconds >= m_Duration)
		destroy();

    if (m_pTarget)
    {
    	if (m_pTarget->isToDelete())
    		m_pTarget = nullptr;
    	else if (m_pTarget->getBody())
    	{
    		m_Pos = CSystemBox2D::b2ToSf(m_pTarget->getBody()->GetPosition());
    	}
    }

    if (m_VelType == VEL_LINEAL)
    	m_Disp = m_Dir*m_Vel;
    else if (m_VelType == VEL_INCREASE)
    	m_Disp += (m_Dir*m_Vel)*elapsedSeconds;

    m_Pos += m_Disp + m_Offset;
    m_Rotation += m_VelRot;
}
