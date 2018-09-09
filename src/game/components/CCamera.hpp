/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_CAMERA
#define H_GAME_COMPONENT_CAMERA

#include <SFML/Graphics.hpp>

#define CAMERA_ZOOM_FACTOR 0.75f

class CCamera final : public CComponent, public sf::View
{
	friend class CGameClient;
public:
	enum
	{
		NORMAL=0,
		FOLLOW=2<<1,
		TRAVEL=2<<2,
	};


	CCamera(CGameClient *pGameClient, const sf::Vector2u &screenSize) noexcept;
	virtual ~CCamera() noexcept;

	void update(float deltaTime) noexcept;

	void setTarget(class CEntity *pEntity) noexcept
	{
		m_pTarget = pEntity;
		if (m_pTarget)
			m_Status |= FOLLOW;
		else
			m_Status &= ~FOLLOW;
	}
	class CEntity* getTarget() noexcept { return m_pTarget; }

	void setZoom(float zoom) noexcept { m_ToZoom = m_Zoom = zoom; }
	float getZoom() const noexcept { return m_Zoom; }
	void setSmoothZoom(float tozoom) noexcept { m_ToZoom = tozoom; }
	bool isZoomStable() const noexcept { return !(m_Zoom < m_ToZoom-0.01f || m_Zoom > m_ToZoom+0.01f); }

	void moveTo(const sf::Vector2f &pos, float duration) noexcept;
	void vibrateCam(float time, float intensity) noexcept;
	int getStatus() const { return m_Status; }

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final
	{
		// Not Used
	}

private:
	class CEntity *m_pTarget;

	sf::Vector2u m_ScreenSize;

	float m_VibrateCamTime;
	float m_VibrateCamIntensity;
	sf::Int64 m_TimerVibrateCam;

	sf::Vector2f m_ToPos;
	float m_ToPosDuration;
	sf::Int64 m_TimerToPos;

	float m_Zoom;
	float m_ToZoom;

	int m_Status;
};

#endif
