/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CSystemSound.hpp>
#include <engine/CLocalization.hpp>
#include "CCamera.hpp"

CCamera::CCamera() noexcept
: CComponent(),
  sf::View()
{
	m_VibrateCamTime = 0.0f;
	m_VibrateCamIntensity = 0.0f;
	m_TimerVibrateCam = 0;
	m_pTarget = nullptr;
	m_ToZoom = m_Zoom = 1.0f;

	m_ToPos = VECTOR_ZERO;
	m_TimerToPos = 0;
	m_ToPosDuration = 0.0f;

	m_Status = NORMAL;
}
CCamera::~CCamera() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CCamera", "Deleted");
	#endif
}

void CCamera::update(float deltaTime) noexcept
{
    // Camara: Zoom
    if (m_Zoom < m_ToZoom-0.01f)
    	m_Zoom += CAMERA_ZOOM_FACTOR*deltaTime;
    else if (m_Zoom > m_ToZoom+0.01f)
    	m_Zoom -= CAMERA_ZOOM_FACTOR*deltaTime;
    setSize(g_Config.m_ScreenWidth, g_Config.m_ScreenHeight);
    zoom(m_Zoom);

    // Info Cam
    sf::Vector2f camCenter = getCenter();

    if (!(m_Status&TRAVEL))
    {
		// Follow Target
		if (m_pTarget && m_pTarget->getShape())
			camCenter = m_pTarget->getShape()->getPosition();

		// Vibration
		if (ups::timeGet()-m_TimerVibrateCam < ups::timeFreq()*m_VibrateCamTime)
			camCenter += sf::Vector2f(sinf(Client()->getElapsedTime()*1000.0f)*m_VibrateCamIntensity, cosf(Client()->getElapsedTime()*1000.0f)*m_VibrateCamIntensity);
    }
    else
    {
    	if (camCenter == m_ToPos)
    	{
    		m_Status &= ~TRAVEL;
    	}
    	else
    	{
    		const float elapsedSeconds = (float)(ups::timeGet()-m_TimerToPos)/ups::timeFreq();
    		const sf::Vector2f stepDir = upm::vectorNormalize(m_ToPos-camCenter);
    		const float stepPos = (upm::vectorLength(m_ToPos-camCenter)/m_ToPosDuration)*elapsedSeconds;
    		camCenter += stepDir*stepPos;
    	}
    }

    setCenter(camCenter);
}

void CCamera::vibrateCam(float time, float intensity) noexcept
{
	m_VibrateCamTime = time;
	m_VibrateCamIntensity = intensity;
	m_TimerVibrateCam = ups::timeGet();
}

void CCamera::moveTo(const sf::Vector2f &pos, float duration) noexcept
{
	m_TimerToPos = ups::timeGet();
	m_ToPos = pos;
	m_ToPosDuration = duration;
	m_Status |= TRAVEL;
}
