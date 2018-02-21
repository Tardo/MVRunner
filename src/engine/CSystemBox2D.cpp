/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CSystemBox2D.hpp"
#include <base/math.hpp>
#include "CGame.hpp"

b2Vec2 CSystemBox2D::ZERO(0.0f, 0.0f);
CSystemBox2D::CSystemBox2D() noexcept
: m_World(b2Vec2(0.0f, GRAVITY))
{

}
CSystemBox2D::~CSystemBox2D() noexcept
{
	b2Body *pBodyList = m_World.GetBodyList();
	while (pBodyList)
	{
		m_World.DestroyBody(pBodyList);
		pBodyList = pBodyList->GetNext();
	}

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSystemBox2D", "Deleted");
	#endif
}

bool CSystemBox2D::init() noexcept
{
	m_World.SetContactListener(&m_ContactListener);
	return true;
}

void CSystemBox2D::update(float deltaTime) noexcept
{
	m_World.Step(deltaTime, VELITER, POSITER);
}


b2Body* CSystemBox2D::createBoxBody(const sf::Vector2f &worldPos, const sf::Vector2f &size, const CB2BodyInfo &bodyInfo) noexcept
{
	b2PolygonShape polyShape;
	b2FixtureDef fixtureDef;
	b2BodyDef bodyDef;
	b2Body *pBody = nullptr;

	bodyDef.type = bodyInfo.m_B2Type;
	bodyDef.position.Set(worldPos.x*MPP, worldPos.y*MPP);
	pBody = m_World.CreateBody(&bodyDef);

	polyShape.SetAsBox(size.x/2*MPP, size.y/2*MPP);

	fixtureDef.shape = &polyShape;
	fixtureDef.friction = bodyInfo.m_Friction;
	fixtureDef.restitution = bodyInfo.m_Restitution;
	fixtureDef.density	= bodyInfo.m_Restitution;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);

	return pBody;
}

b2Body* CSystemBox2D::createPolygonBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, const CB2BodyInfo &bodyInfo) noexcept
{
	b2PolygonShape polyShape;
	b2FixtureDef fixtureDef;
	b2BodyDef bodyDef;
	b2Body *pBody = nullptr;

	bodyDef.type = bodyInfo.m_B2Type;
	bodyDef.position.Set(worldPos.x*MPP, worldPos.y*MPP);
	pBody = m_World.CreateBody(&bodyDef);

	const std::size_t numPoints = points.size();
	b2Vec2 *pB2Points = new b2Vec2[numPoints];
	for (std::size_t i=0; i<numPoints; i++)
		pB2Points[i] = sfToB2(points[i]);
	polyShape.Set(pB2Points, numPoints);

	fixtureDef.shape = &polyShape;
	fixtureDef.friction = bodyInfo.m_Friction;
	fixtureDef.restitution = bodyInfo.m_Restitution;
	fixtureDef.density	= bodyInfo.m_Restitution;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);

	delete [] pB2Points;

	return pBody;
}

b2Body* CSystemBox2D::createPolyLineBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, const CB2BodyInfo &bodyInfo) noexcept
{
	b2ChainShape polyLineShape;
	b2FixtureDef fixtureDef;
	b2BodyDef bodyDef;
	b2Body *pBody = nullptr;

	bodyDef.type = bodyInfo.m_B2Type;
	bodyDef.position.Set(worldPos.x*MPP, worldPos.y*MPP);
	pBody = m_World.CreateBody(&bodyDef);

	const std::size_t numPoints = points.size();
	b2Vec2 *pB2Points = new b2Vec2[numPoints];
	for (std::size_t i=0; i<numPoints; i++)
		pB2Points[i] = sfToB2(points[i]);
	polyLineShape.CreateChain(pB2Points, numPoints);

	fixtureDef.shape = &polyLineShape;
	fixtureDef.friction = bodyInfo.m_Friction;
	fixtureDef.restitution = bodyInfo.m_Restitution;
	fixtureDef.density	= bodyInfo.m_Restitution;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);

	delete [] pB2Points;

	return pBody;
}

b2Body* CSystemBox2D::createCircleBody(const sf::Vector2f &worldPos, float radius, const CB2BodyInfo &bodyInfo) noexcept
{
	b2CircleShape circleShape;
	b2FixtureDef fixtureDef;
	b2BodyDef bodyDef;
	b2Body *pBody = nullptr;

	bodyDef.type = bodyInfo.m_B2Type;
	bodyDef.position.Set(worldPos.x*MPP, worldPos.y*MPP);
	pBody = m_World.CreateBody(&bodyDef);

	//circleShape.m_p.Set(radius*MPP, radius*MPP);
	circleShape.m_radius = radius*MPP;

	fixtureDef.shape = &circleShape;
	fixtureDef.friction = bodyInfo.m_Friction;
	fixtureDef.restitution = bodyInfo.m_Restitution;
	fixtureDef.density	= bodyInfo.m_Restitution;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);

	return pBody;
}

void CSystemBox2D::destroyBody(b2Body *pBody) noexcept
{
	if (!pBody)
		return;

	m_World.DestroyBody(pBody);
}

void CSystemBox2D::createExplosion(const sf::Vector2f &worldPos, float energy, float radius, b2Body *pNotThis) noexcept
{
	std::vector<b2Body*> vpBodies = getBodiesNear(worldPos, radius, 0, pNotThis);
	for (std::size_t i=0; i<vpBodies.size(); ++i)
	{
		b2Body *pBody = vpBodies[i];
		const sf::Vector2f pos = b2ToSf(pBody->GetPosition());
		const sf::Vector2f dir = upm::vectorNormalize(pos - worldPos);
		const float dist = upm::vectorLength(pos - worldPos);
		if (dist > radius)
			continue;

		float impulse = upm::clamp((1.0f - dist/radius) * energy, 0.0f, 40.0f);
		sf::Vector2f force = dir * impulse;
		pBody->ApplyLinearImpulseToCenter(sfToB2(force), true);
	}
}

b2Fixture* CSystemBox2D::createFixture(b2Body *pBody, const b2Shape &Shape, bool sensor, const b2Filter &filter, void *pUserData, float density, float friction, float restitution) noexcept
{
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &Shape;
	fixtureDef.isSensor = sensor;
	fixtureDef.filter = filter;
	fixtureDef.density = density;
	fixtureDef.friction = friction;
	fixtureDef.restitution = restitution;
	fixtureDef.userData = pUserData;
	return pBody->CreateFixture(&fixtureDef);
}

std::vector<b2Body*> CSystemBox2D::getBodiesNear(const sf::Vector2f &worldPos, float margin, sf::Uint16 categoryBits, b2Body *pNotThis) noexcept
{
	std::vector<b2Body*> vpBodies;
	MultiQueryCallBack queryCallback;
	b2AABB aabb;
	const sf::Vector2f lowerBound = worldPos - sf::Vector2f(margin, margin);
	const sf::Vector2f upperBound = worldPos + sf::Vector2f(margin, margin);
	aabb.lowerBound = sfToB2(lowerBound);
	aabb.upperBound = sfToB2(upperBound);
	m_World.QueryAABB(&queryCallback, aabb);

	std::vector<b2Fixture*>::iterator it = queryCallback.m_vpFixtures.begin();
	while (it != queryCallback.m_vpFixtures.end())
	{
		b2Body *pBody = (*it)->GetBody();
		if (!pBody || pBody == pNotThis)
		{
			++it;
			continue;
		}

		if (categoryBits)
		{
			if ((*it)->GetFilterData().categoryBits&categoryBits)
				vpBodies.push_back((*it)->GetBody());
		}
		else
			vpBodies.push_back((*it)->GetBody());

		++it;
	}

	return vpBodies;
}

b2Fixture* CSystemBox2D::getFixtureAt(const sf::Vector2f &worldPos) noexcept
{
	SimpleQueryCallBack queryCallBack;
	b2AABB aabb;
	aabb.upperBound = aabb.lowerBound = sfToB2(worldPos);
	m_World.QueryAABB(&queryCallBack, aabb);
	return queryCallBack.m_pFixture;
}

CEntity* CSystemBox2D::checkIntersectLine(const sf::Vector2f &worlPosInit, const sf::Vector2f &worldPosEnd, sf::Vector2f *pPoint, b2Body *pNotThis, uint16 maskBits) noexcept
{
	if (upm::vectorLength(worldPosEnd-worlPosInit) == 0.0f)
		return nullptr;

	RaysCastCallback callback(pNotThis, maskBits);
	getWorld()->RayCast(&callback, sfToB2(worlPosInit), sfToB2(worldPosEnd));
	if (callback.m_Hit)
	{
		if (pPoint)
			*pPoint = callback.m_Point;
		return callback.m_pEntity;
	}

	return nullptr;
}

/** CONTACT LISTENER **/
void CSystemBox2D::CContactListener::BeginContact(b2Contact* pContact) noexcept
{
	b2WorldManifold worldManifold;
	pContact->GetWorldManifold(&worldManifold);

	b2Fixture *pSensorFixture;
	b2Fixture *pObjFixture;
	if (getSensorAndEntity(pContact, pSensorFixture, pObjFixture))
	{
		CEntity* pSensorEntity = static_cast<CEntity*>(pSensorFixture->GetBody()->GetUserData());
		CEntity* pObjEntity = static_cast<CEntity*>(pObjFixture->GetBody()->GetUserData());
		if (pSensorEntity && pObjEntity)
			pSensorEntity->onSensorIn(pObjEntity);
	}
	else
	{
	    const b2Fixture *pFixtureA = pContact->GetFixtureA();
	    const b2Fixture *pFixtureB = pContact->GetFixtureB();

	    CEntity* pEntityA = static_cast<CEntity*>(pFixtureA->GetBody()->GetUserData());
	    CEntity* pEntityB = static_cast<CEntity*>(pFixtureB->GetBody()->GetUserData());

	    if (pEntityA && pEntityB)
	    {
			pEntityA->onContact(pEntityB, b2ToSf(worldManifold.points[0]));
			pEntityB->onContact(pEntityA, b2ToSf(worldManifold.points[0]));
	    }
	}
}
void CSystemBox2D::CContactListener::EndContact(b2Contact* pContact) noexcept
{
	b2Fixture *pSensorFixture;
	b2Fixture *pObjFixture;
	if (getSensorAndEntity(pContact, pSensorFixture, pObjFixture))
	{
		CEntity* pSensorEntity = static_cast<CEntity*>(pSensorFixture->GetBody()->GetUserData());
		CEntity* pObjEntity = static_cast<CEntity*>(pObjFixture->GetBody()->GetUserData());
		pSensorEntity->onSensorOut(pObjEntity);
	}
}

void CSystemBox2D::CContactListener::PostSolve(b2Contact* pContact, const b2ContactImpulse* pImpulse) noexcept
{
	b2WorldManifold worldManifold;
	pContact->GetWorldManifold(&worldManifold);

	const b2Fixture *pFixtureA = pContact->GetFixtureA();
	const b2Fixture *pFixtureB = pContact->GetFixtureB();

	CEntity* pEntityA = static_cast<CEntity*>(pFixtureA->GetBody()->GetUserData());
	CEntity* pEntityB = static_cast<CEntity*>(pFixtureB->GetBody()->GetUserData());

	if (pEntityA && pEntityB)
	{
		pEntityA->onPostSolve(pEntityB, b2ToSf(worldManifold.points[0]), pImpulse->normalImpulses[0]);
		pEntityB->onPostSolve(pEntityA, b2ToSf(worldManifold.points[0]), pImpulse->normalImpulses[0]);
	}
}

bool CSystemBox2D::CContactListener::getSensorAndEntity(b2Contact* pContact, b2Fixture*& sensorEntity, b2Fixture*& objEntity) noexcept
{
    b2Fixture *pFixtureA = pContact->GetFixtureA();
    b2Fixture *pFixtureB = pContact->GetFixtureB();

    //make sure only one of the fixtures was a sensor
    const bool sensorA = pFixtureA->IsSensor();
    const bool sensorB = pFixtureB->IsSensor();
    if (!(sensorA ^ sensorB))
        return false;

    if (sensorA)
    {
    	sensorEntity = pFixtureA;
    	objEntity = pFixtureB;
    }
    else
    {
    	sensorEntity = pFixtureB;
    	objEntity = pFixtureA;
    }
    return true;
}
