/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_CONTROLLER
#define H_GAME_CONTROLLER

#include <base/math.hpp>
#include <game/CContext.hpp>
#include <game/entities/CLight.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

#define MARGIN_CREATE_OBJECTS	150.0f

class CSpawn
{
public:
	CSpawn()
	{
		m_Pos = VECTOR_ZERO;
		m_Dir = VECTOR_ZERO;
	}
	CSpawn(const sf::Vector2f &pos) noexcept
	{
		m_Pos = pos;
		m_Dir = VECTOR_ZERO;
	}
	CSpawn(const sf::Vector2f &pos, const sf::Vector2f &dir) noexcept
	{
		m_Pos = pos;
		m_Dir = dir;
	}

	sf::Vector2f m_Pos;
	sf::Vector2f m_Dir;
};


class CController
{
public:
	CController() noexcept;
	CController(CContext *pContext) noexcept;
	virtual ~CController() noexcept;

	class CGame* Game() const noexcept { return m_pGame; }
	CContext *Context() const noexcept { return m_pGameContext; }

	virtual void tick() noexcept;

	virtual bool onInit() noexcept;
	virtual void onStart() noexcept;
	virtual void onResetGame() noexcept;
	virtual void onCharacterDeath(CCharacter *pVictim, CPlayer *pKiller) noexcept;
	virtual bool onMapTile(unsigned int tileId, const sf::Vector2f &pos, unsigned int tileDir, unsigned int modifierId) noexcept;
	virtual void onMapObject(CMapRenderObject *pMapObj, int objId, const sf::Vector2f &worldPos, const sf::Vector2f &size) noexcept;

	virtual void updateCamera(float deltaTime) noexcept;

	virtual bool isStaticObject(const char *pType) const noexcept; // FIXME: Use other implementation!!

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
	void createImpactSparkMetal(const sf::Vector2f &worldPos) noexcept;
	void createExplosionCar(const sf::Vector2f &worldPos, bool ring) noexcept;

	CLight* createPoint(const sf::Vector2f &worldPos, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;
	CLight* createSpot(const sf::Vector2f &worldPos, float angle, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;
	CLight* createEmissive(const sf::Vector2f &worldPos, float angle, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;
	CLight* createCustom(int textId, const sf::Vector2f &worldPos, const sf::Vector2f &origin, float angle, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;

	CSpawn m_PlayerSpawnPos;

private:
	class CGame *m_pGame;
	CContext *m_pGameContext;
};

#endif
