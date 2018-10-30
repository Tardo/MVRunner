/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_SYSTEM_BOX2D
#define H_ENGINE_SYSTEM_BOX2D

#include <base/math.hpp>
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <game/CEntity.hpp>
#include <vector>
#include "ISystem.hpp"

#define VELITER 1
#define POSITER 1
#define GRAVITY 9.8f
#define MAX_PARTICLES 100

#define PPM 128.0f               //PIXELS POR METRO
#define MPP (1.0f/PPM)          //METROS POR PIXEL

struct CB2BodyInfo
{
	CB2BodyInfo()
	{
		m_Friction = 0.0f;
		m_Density = 0.0f;
		m_Restitution = 0.0f;
		m_Mass = 0.0f;
		m_CategoryBits = 0;
		m_MaskBits = 0xFFFF;
		m_IsSensor = false;
		m_B2Type = b2_dynamicBody;
	}
	CB2BodyInfo(float friction, float density, float restitution, float mass, b2BodyType b2type, uint16 cBits, bool sensor = false, uint16 mBits = 0xFFFF)
	{
		m_Friction = friction;
		m_Density = density;
		m_Restitution = restitution;
		m_Mass = mass;
		m_CategoryBits = cBits;
		m_MaskBits = mBits;
		m_IsSensor = sensor;
		m_B2Type = b2type;
	}

	float m_Friction;
	float m_Density;
	float m_Restitution;
	float m_Mass;
	uint16 m_CategoryBits;
	uint16 m_MaskBits;
	bool m_IsSensor;
	b2BodyType m_B2Type;
};

class CSystemBox2D final : public ISystem
{
	class CContactListener final : public b2ContactListener
	{
		virtual void BeginContact(b2Contact* pContact) noexcept final;
		virtual void EndContact(b2Contact* pContact) noexcept final;
		virtual void PostSolve(b2Contact* pContact, const b2ContactImpulse* pImpulse) noexcept final;
		virtual bool getSensorAndEntity(b2Contact* pContact, b2Fixture*& sensorEntity, b2Fixture*& objEntity) noexcept final;
	} m_ContactListener;
public:
	enum
	{
		PARTICLE_SYSTEM_WATER=0,

		NUM_PARTICLE_SYSTEMS
	};

	CSystemBox2D() noexcept;
	virtual ~CSystemBox2D() noexcept;

	virtual bool init(CAssetManager *pAssetManager) noexcept final;
	virtual void reset() noexcept final;
	virtual void update(float deltaTime) noexcept final;

	b2World* getWorld() noexcept { return &m_World; }
	b2ParticleSystem* getParticleSystem(std::size_t index) noexcept;

	void resetParticleSystems() noexcept;

	b2Body* createBoxBody(const sf::Vector2f &worldPos, const sf::Vector2f &size, float rot, const CB2BodyInfo &bodyInfo) noexcept;
	b2Body* createCircleBody(const sf::Vector2f &worldPos, float radius, float rot, const CB2BodyInfo &bodyInfo) noexcept;
	b2Body* createPolygonBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, float rot, const CB2BodyInfo &bodyInfo) noexcept;
	b2Body* createPolyLineBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, float rot, const CB2BodyInfo &bodyInfo) noexcept;

	b2Fixture* createFixture(b2Body *pBody, const b2Shape &Shape, bool sensor, const b2Filter &filter, void *pUserData = nullptr, float density = 0.0f, float friction = 0.0f, float restitution = 0.0f) noexcept;

	b2ParticleGroup* createLiquidFunParticleGroup(const sf::Vector2f &worldPos, b2Shape *pShape, const b2ParticleColor &color, float lifetime, int flags, const b2Vec2 &vel=b2Vec2_zero, float angularVelocity=0.0f, int groupFlags=0, float strength=1.0f) noexcept;
	b2ParticleGroup* createViscosity(const sf::Vector2f &worldPos, float radius) noexcept;
	b2ParticleGroup* createWater(const sf::Vector2f &worldPos, float radius) noexcept;
	b2ParticleGroup* createBlood(const sf::Vector2f &worldPos, float radius) noexcept;

	std::vector<b2Body*> getBodiesNear(const sf::Vector2f &worldPos, float margin, sf::Uint16 categoryBits, b2Body *pNotThis = nullptr) noexcept;
	b2Fixture* getFixtureAt(const sf::Vector2f &worldPos) noexcept;

	void applyBlastImpulse(b2Body *pBody, const sf::Vector2f &blastCenter, const sf::Vector2f &applyPoint, float blastPower);
	class CEntity* checkIntersectLine(const sf::Vector2f &worlPosInit, const sf::Vector2f &worldPosEnd, sf::Vector2f *pPoint = nullptr, b2Body *pNotThis = nullptr, uint16 maskBits = 0xFFF) noexcept;
	void createExplosion(const sf::Vector2f &worldPos, float energy, float radius, b2Body *pNotThis = nullptr) noexcept;

	void destroyBody(b2Body *pBody) noexcept;
	void destroyJoint(b2Joint *pJoint) noexcept;

	static inline b2Vec2 sfToB2(const sf::Vector2f &vector) noexcept { return b2Vec2(vector.x*MPP, vector.y*MPP); }
	static inline float sfToB2(float num) noexcept { return num*MPP; }
	static inline b2Color sfToB2(const sf::Color &color) noexcept { return b2Color(color.r/255, color.g/255, color.b/255); }
	static inline sf::Vector2f b2ToSf(const b2Vec2 &vector) noexcept { return sf::Vector2f(vector.x*PPM, vector.y*PPM); }
	static inline float b2ToSf(float num) noexcept { return num*PPM; }
	static inline sf::Color b2ToSf(const b2Color &color) noexcept { return sf::Color(color.r*255, color.g*255, color.b*255, 255); }
	static inline sf::FloatRect b2ToSf(const b2AABB &aabb) noexcept { return sf::FloatRect(aabb.lowerBound.x*PPM, aabb.lowerBound.y*PPM, aabb.upperBound.x*PPM, aabb.upperBound.y*PPM); }

	static bool findIntersectionOfFixtures(b2Fixture *pFA, b2Fixture *pFB, std::vector<b2Vec2> *pOutputVertices) noexcept;
	static b2Vec2 computeCentroid(const std::vector<b2Vec2> &vs, float *pArea) noexcept;
	static b2Vec2 ZERO;

private:
	b2World m_World;
	b2ParticleSystem *m_pParticleSystems[NUM_PARTICLE_SYSTEMS];

	static bool inside(const b2Vec2 &cp1, const b2Vec2 &cp2, const b2Vec2 &p) noexcept;
	static b2Vec2 intersection(const b2Vec2 &cp1, const b2Vec2 &cp2, const b2Vec2 &s, const b2Vec2 &e) noexcept;
};


class RaysCastCallback final : public b2RayCastCallback
{
public:
	RaysCastCallback(b2Body *pNotThis = nullptr, uint16 maskBits=0xFFF) noexcept
	{
		m_Hit = false;
		m_Fraction = 1.0f;
		m_Point = VECTOR_ZERO;
		m_pEntity = nullptr;
		m_pNotThis = pNotThis;
		m_MaskBits = maskBits;
	}

	virtual float32 ReportFixture(b2Fixture *pFixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) noexcept final
	{
		class CEntity *pEnt = static_cast<class CEntity*>(pFixture->GetBody()->GetUserData());
		if(!pFixture->IsSensor() && pEnt && fraction < this->m_Fraction && pFixture->GetBody() != m_pNotThis && (pFixture->GetFilterData().maskBits&m_MaskBits))
		{
			m_pEntity = static_cast<class CEntity*>(pFixture->GetBody()->GetUserData());
			m_Point = CSystemBox2D::b2ToSf(point);
			m_Fraction = fraction;
			m_Hit = true;
		}
		return 1;// keep going to get all intersection points
	}

	bool m_Hit;
	sf::Vector2f m_Point;
	float32 m_Fraction;
	class CEntity *m_pEntity;
	b2Body *m_pNotThis;
	uint16 m_MaskBits;
};

class RaysCastParticleSystemCallback final : public b2RayCastCallback
{
public:
	RaysCastParticleSystemCallback(b2Body *pNotThis = nullptr, uint16 maskBits=0xFFF) noexcept
	{
		m_Hit = false;
		m_Fraction = 1.0f;
		m_Point = VECTOR_ZERO;
		m_ParticleIndex = -1;
	}

	virtual float32 ReportFixture(b2Fixture *pFixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) noexcept final
	{ return 1; }

	virtual float32 ReportParticle(const b2ParticleSystem *pParticleSystem, int32 index, const b2Vec2& point, const b2Vec2& normal, float32 fraction) noexcept final
	{
		if (fraction < this->m_Fraction)
		{
			m_Hit = true;
			m_ParticleIndex = index;
			m_Point = CSystemBox2D::b2ToSf(point);
		}
		return 1;// keep going to get all intersection points
	}

	bool m_Hit;
	sf::Vector2f m_Point;
	float32 m_Fraction;
	int32 m_ParticleIndex;
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

class MultiQueryParticleSystemCallBack final : public b2QueryCallback
{
public:
	MultiQueryParticleSystemCallBack() noexcept
	{
		m_vIndex.clear();
	}
	bool ReportFixture(b2Fixture* fixture) noexcept
	{
		return false;
	}
	virtual bool ReportParticle(const b2ParticleSystem* pParticleSystem, int32 index)
	{
		m_vIndex.push_back(index);
		return true;
	}

	std::vector<int32> m_vIndex;
};

#endif
