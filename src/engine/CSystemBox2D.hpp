/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_SYSTEM_BOX2D
#define H_ENGINE_SYSTEM_BOX2D

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <engine/CSystem.hpp>
#include <game/CEntity.hpp>
#include <vector>

#define VELITER 1
#define POSITER 1
#define GRAVITY 9.8f

#define PPM 128.0f               //PIXELS POR METRO
#define MPP (1.0f/PPM)          //METROS POR PIXEL

struct CB2BodyInfo
{
	CB2BodyInfo()
	{
		m_Friction = 0.0f;
		m_Density = 0.0f;
		m_Restitution = 0.0f;
		m_CategoryBits = 0;
		m_MaskBits = 0xFFFF;
		m_IsSensor = false;
		m_B2Type = b2_dynamicBody;
	}
	CB2BodyInfo(float friction, float density, float restitution, b2BodyType b2type, uint16 cBits, bool sensor = false, uint16 mBits = 0xFFFF)
	{
		m_Friction = friction;
		m_Density = density;
		m_Restitution = restitution;
		m_CategoryBits = cBits;
		m_MaskBits = mBits;
		m_IsSensor = sensor;
		m_B2Type = b2type;
	}

	float m_Friction;
	float m_Density;
	float m_Restitution;
	uint16 m_CategoryBits;
	uint16 m_MaskBits;
	bool m_IsSensor;
	b2BodyType m_B2Type;
};

class CSystemBox2D final : public CSystem
{
	class CContactListener final : public b2ContactListener
	{
		virtual void BeginContact(b2Contact* pContact) noexcept final;
		virtual void EndContact(b2Contact* pContact) noexcept final;
		virtual void PostSolve(b2Contact* pContact, const b2ContactImpulse* pImpulse) noexcept final;
		virtual bool getSensorAndEntity(b2Contact* pContact, b2Fixture*& sensorEntity, b2Fixture*& objEntity) noexcept final;
	} m_ContactListener;
public:
	CSystemBox2D() noexcept;
	virtual ~CSystemBox2D() noexcept;

	virtual bool init(class CGameClient *pGameClient) noexcept final;
	virtual void update(float deltaTime) noexcept final;

	b2World* getWorld() noexcept { return &m_World; }

	b2Body* createBoxBody(const sf::Vector2f &worldPos, const sf::Vector2f &size, const CB2BodyInfo &bodyInfo) noexcept;
	b2Body* createCircleBody(const sf::Vector2f &worldPos, float radius, const CB2BodyInfo &bodyInfo) noexcept;
	b2Body* createPolygonBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, const CB2BodyInfo &bodyInfo) noexcept;
	b2Body* createPolyLineBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, const CB2BodyInfo &bodyInfo) noexcept;


	std::vector<b2Body*> getBodiesNear(const sf::Vector2f &worldPos, float margin, sf::Uint16 categoryBits, b2Body *pNotThis = nullptr) noexcept;
	b2Fixture* getFixtureAt(const sf::Vector2f &worldPos) noexcept;

	CEntity* checkIntersectLine(const sf::Vector2f &worlPosInit, const sf::Vector2f &worldPosEnd, sf::Vector2f *pPoint = nullptr, CEntity *pNotThis = nullptr) noexcept;
	void createExplosion(const sf::Vector2f &worldPos, float energy, float radius, b2Body *pNotThis = nullptr) noexcept;

	void destroyBody(b2Body *pBody) noexcept;

	static inline b2Vec2 sfToB2(const sf::Vector2f &vector) noexcept { return b2Vec2(vector.x*MPP, vector.y*MPP); }
	static inline sf::Vector2f b2ToSf(const b2Vec2 &vector) noexcept { return sf::Vector2f(vector.x*PPM, vector.y*PPM); }
	static b2Vec2 ZERO;

private:
	b2World m_World;
};


class RaysCastCallback final : public b2RayCastCallback
{
public:
	RaysCastCallback(CEntity *pNotThis = nullptr) noexcept
	{
		m_Hit = false;
		m_Fraction = 1.0f;
		m_Point = VECTOR_ZERO;
		m_pEntity = nullptr;
		m_pNotThis = pNotThis;
	}

	float32 ReportFixture(b2Fixture *pFixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) noexcept
	{
		CEntity *pEnt = static_cast<CEntity*>(pFixture->GetBody()->GetUserData());
		if(!pFixture->IsSensor() && fraction < this->m_Fraction && pEnt && pEnt != m_pNotThis)
		{
			m_pEntity = static_cast<CEntity*>(pFixture->GetBody()->GetUserData());
			m_Point = CSystemBox2D::b2ToSf(point);
			m_Fraction = fraction;
			m_Hit = true;
		}
		return 1;// keep going to get all intersection points
	}

	bool m_Hit;
	sf::Vector2f m_Point;
	float32 m_Fraction;
	CEntity *m_pEntity;
	CEntity *m_pNotThis;
};

class SimpleQueryCallBack final : public b2QueryCallback
{
public:
	SimpleQueryCallBack() noexcept
	{
		m_pFixture = nullptr;
	}
	bool ReportFixture(b2Fixture* fixture) noexcept
	{
		m_pFixture = fixture;
		return false;
	}

	b2Fixture *m_pFixture;
};

class MultiQueryCallBack final : public b2QueryCallback
{
public:
	MultiQueryCallBack() noexcept
	{
		m_vpFixtures.clear();
	}
	bool ReportFixture(b2Fixture* fixture) noexcept
	{
		m_vpFixtures.push_back(fixture);
		return true;
	}

	std::vector<b2Fixture*> m_vpFixtures;
};

#endif
