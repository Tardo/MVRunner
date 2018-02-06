/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_SYSTEM_EFFECTS
#define H_ENGINE_SYSTEM_EFFECTS

#include <engine/CSystemLight.hpp>
#include <engine/CSystem.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

class CParticle final : sf::Drawable
{
public:
	enum
	{
		VEL_LINEAL=0,
		VEL_INCREASE,
	};

    CParticle(sf::BlendMode blendMode = sf::BlendAlpha, int render = RENDER_FRONT, bool Light = false, int shader = -1) noexcept;
    virtual ~CParticle() noexcept;

    void destroy() noexcept;
    void update() noexcept;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

    CLight* getLight() const noexcept { return m_pLight; }

    sf::Vector2f m_Pos;
    sf::Vector2f m_Dir;
    sf::Vector2f m_SizeInit;
    sf::Vector2f m_SizeEnd;
    sf::Vector2i m_AnimSize;
    sf::Color m_ColorInit;
    sf::Color m_ColorEnd;
    sf::RectangleShape m_Shape;
    sf::Text m_String;
    sf::BlendMode m_BlendMode;
    sf::Shader *m_pShader;
    float m_Vel;
    float m_VelRot;
    float m_Duration;
    int m_Render;
    bool m_ModeText;
    bool m_DoMove;
    bool m_Animated;
    bool m_Light;
    bool m_FixedPos;
    bool m_OnlyLight;
    int m_VelType;
    sf::Int32 m_ShaderId;
    sf::Int64 m_Timer;
    class CEntity *m_pTarget;
    sf::Vector2f m_Offset;

private:
    CLight *m_pLight;
    sf::Vector2f m_Disp;
};

class CSystemFx final : public CSystem
{
public:
	CSystemFx() noexcept;
	virtual ~CSystemFx() noexcept;

	virtual bool init(class CGameClient *pGameClient) noexcept final;
	virtual void update(float deltaTime) noexcept final;

	void createSpiritTrail(const sf::Vector2f &worldPos) noexcept;
	void createSpirit(const sf::Vector2f &worldPos) noexcept;
	void createFireBall(class CEntity *pTarget, const sf::Vector2f &offSet = VECTOR_ZERO) noexcept;
	void createFireTrailSmall(const sf::Vector2f &worldPos) noexcept;
	void createFireTrailLarge(const sf::Vector2f &worldPos) noexcept;
	void createBloodSpark(const sf::Vector2f &worldPos, float duration = 60.0f) noexcept;
	void createBlood(const sf::Vector2f &worldPos) noexcept;
	void createPoints(const sf::Vector2f &worldPos, int points) noexcept;
	void createSmokeImpact(const sf::Vector2f &worldPos, const sf::Vector2f &dir, float vel) noexcept;
	void createRainBack(const sf::Vector2f &worldPos, float rainVel) noexcept;
	void createRainFront(const sf::Vector2f &worldPos, float rainVel) noexcept;
	void createStorm() noexcept;
	void createSnowBack(const sf::Vector2f &worldPos, float snowVel) noexcept;
	void createSnowFront(const sf::Vector2f &worldPos, float snowVel) noexcept;

	void createSmokeCarDamaged(const sf::Vector2f &worldPos, bool fire) noexcept;
	void createSpitFireCarDamaged(const sf::Vector2f &worldPos, const sf::Vector2f &dir = VECTOR_ZERO) noexcept;
	void createImpactSparkMetal(const sf::Vector2f &worldPos) noexcept;
	void createExplosionCar(const sf::Vector2f &worldPos, bool ring) noexcept;
	void createExplosionRocket(const sf::Vector2f &worldPos) noexcept;
	void createPointLight(const sf::Vector2f &worldPos, float radius, float duration, sf::Color colorI, sf::Color colorE) noexcept;
	void createMarkWheel(const sf::Vector2f &worldPos, float angle) noexcept;
	void createWaterSplash(const sf::Vector2f &worldPos) noexcept;
	void createSnowSplash(const sf::Vector2f &worldPos) noexcept;
	void createSnowTrail(const sf::Vector2f &worldPos) noexcept;
	void createGasolineFire(const sf::Vector2f &worldPos) noexcept;
	void createHighlightItem(const sf::Vector2f &worldPos, sf::Color color, int renderZone) noexcept;
	void createHighlightItemSimple(const sf::Vector2f &worldPos, sf::Color color, float size, int rederZone) noexcept;

private:
	sf::Clock m_Timer100Hz;
	sf::Clock m_Timer50Hz;
	bool m_Add100Hz;
	bool m_Add50Hz;
};

#endif
