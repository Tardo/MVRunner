/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_CAMERA
#define H_GAME_CAMERA

#include <SFML/Graphics.hpp>

#define CAMERA_ZOOM_FACTOR 0.75f

class CCamera final : public sf::View
{
	friend class CGameClient;
public:
	enum
	{
		NORMAL=0,
		FOLLOW=2<<1,
		TRAVEL=2<<2,
	};


	CCamera() noexcept;
	virtual ~CCamera() noexcept;

	class CGameClient* Client() const noexcept { return m_pGameClient; }

	virtual void update(float deltaTime) noexcept final;

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
	void setSmoothZoom(float tozoom) noexcept { m_ToZoom = tozoom; }
	bool isZoomStable() const noexcept { return !(m_Zoom < m_ToZoom-0.01f || m_Zoom > m_ToZoom+0.01f); }

	void moveTo(const sf::Vector2f &pos, float duration) noexcept;
	void vibrateCam(float time, float intensity) noexcept;
	int getStatus() const { return m_Status; }

private:
	class CGameClient *m_pGameClient;
	class CEntity *m_pTarget;

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
