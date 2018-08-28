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
	m_World.ClearForces();
}


b2Body* CSystemBox2D::createBoxBody(const sf::Vector2f &worldPos, const sf::Vector2f &size, float rot, const CB2BodyInfo &bodyInfo) noexcept
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
	fixtureDef.density	= bodyInfo.m_Density;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);
	if (bodyInfo.m_Mass > 0.0f)
	{
		b2MassData MassData;
		pBody->GetMassData(&MassData);
		MassData.mass = bodyInfo.m_Mass;
		pBody->SetMassData(&MassData);
	}
	pBody->SetTransform(pBody->GetPosition(), upm::degToRad(rot));

	return pBody;
}

b2Body* CSystemBox2D::createPolygonBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, float rot, const CB2BodyInfo &bodyInfo) noexcept
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
	fixtureDef.density	= bodyInfo.m_Density;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);
	if (bodyInfo.m_Mass > 0.0f)
	{
		b2MassData MassData;
		pBody->GetMassData(&MassData);
		MassData.mass = bodyInfo.m_Mass;
		pBody->SetMassData(&MassData);
	}
	pBody->SetTransform(pBody->GetPosition(), upm::degToRad(rot));

	delete [] pB2Points;

	return pBody;
}

b2Body* CSystemBox2D::createPolyLineBody(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> points, float rot, const CB2BodyInfo &bodyInfo) noexcept
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
	fixtureDef.density	= bodyInfo.m_Density;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);
	if (bodyInfo.m_Mass > 0.0f)
	{
		b2MassData MassData;
		pBody->GetMassData(&MassData);
		MassData.mass = bodyInfo.m_Mass;
		pBody->SetMassData(&MassData);
	}
	pBody->SetTransform(pBody->GetPosition(), upm::degToRad(rot));

	delete [] pB2Points;

	return pBody;
}

b2Body* CSystemBox2D::createCircleBody(const sf::Vector2f &worldPos, float radius, float rot, const CB2BodyInfo &bodyInfo) noexcept
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
	fixtureDef.density	= bodyInfo.m_Density;
	fixtureDef.isSensor = bodyInfo.m_IsSensor;
	fixtureDef.filter.categoryBits = bodyInfo.m_CategoryBits;
	fixtureDef.filter.maskBits = bodyInfo.m_MaskBits;

	pBody->CreateFixture(&fixtureDef);
	if (bodyInfo.m_Mass > 0.0f)
	{
		b2MassData MassData;
		pBody->GetMassData(&MassData);
		MassData.mass = bodyInfo.m_Mass;
		pBody->SetMassData(&MassData);
	}
	pBody->SetTransform(pBody->GetPosition(), upm::degToRad(rot));

	return pBody;
}

void CSystemBox2D::destroyBody(b2Body *pBody) noexcept
{
	if (!pBody)
		return;

	m_World.DestroyBody(pBody);
}

void CSystemBox2D::destroyJoint(b2Joint *pJoint) noexcept
{
	if (!pJoint)
		return;

	m_World.DestroyJoint(pJoint);
}

void CSystemBox2D::applyBlastImpulse(b2Body *pBody, const sf::Vector2f &blastCenter, const sf::Vector2f &applyPoint, float blastPower)
{
	b2Vec2 blastDir = sfToB2(applyPoint) - sfToB2(blastCenter);
    float distance = blastDir.Normalize();
    //ignore bodies exactly at the blast point - blast direction is undefined
    if (distance == 0.0f)
        return;
    float invDistance = 1.0f / distance;
    float impulseMag = blastPower * invDistance * invDistance;
    pBody->ApplyLinearImpulse(impulseMag * blastDir, sfToB2(applyPoint), true);
}

void CSystemBox2D::createExplosion(const sf::Vector2f &worldPos, float energy, float radius, b2Body *pNotThis) noexcept
{
	static const int NUM_RAYS = 32;
	const float epenergy = (energy / (float)NUM_RAYS);
	const b2Vec2 pos = sfToB2(worldPos);

	bool isCharacter = false;
	for (int i = 0; i < NUM_RAYS; ++i)
	{
		const float angle = upm::degToRad((i / (float)NUM_RAYS) * 360.0f);
		const b2Vec2 rayDir(sinf(angle), cosf(angle));
		const b2Vec2 rayEnd = pos + radius * rayDir;

		//check what this ray hits
		RaysCastCallback callback;//basic callback to record body and hit point
		m_World.RayCast(&callback, pos, rayEnd);
		if (callback.m_pEntity && callback.m_pEntity->getBody() && callback.m_pEntity->getBody()->GetType() == b2_dynamicBody)
		{
			if (callback.m_pEntity->getType() == CEntity::CHARACTER)
				isCharacter = true;
			else
				callback.m_pEntity->getBody()->ApplyLinearImpulse(epenergy * rayDir, sfToB2(callback.m_Point), true);
				//applyBlastImpulse(callback.m_pEntity->getBody(), worldPos, callback.m_Point, epenergy);
		}
	}

	// FIXME: Workaround for stable character impulse
	if (isCharacter)
	{
		std::vector<b2Body*> vpBodies = getBodiesNear(worldPos, radius, CAT_CHARACTER_PLAYER, pNotThis);
		for (std::size_t i=0; i<vpBodies.size(); ++i)
		{
			b2Body *pBody = vpBodies[i];
			const sf::Vector2f pos = b2ToSf(pBody->GetPosition());
			const sf::Vector2f dir = upm::vectorNormalize(pos - worldPos);
			const float dist = upm::vectorLength(pos - worldPos);
			if (dist > radius)
				continue;

			float impulse = (1.0f - dist/radius) * energy * 12.0f;
			sf::Vector2f force = dir * impulse;
			//applyBlastImpulse(pBody, worldPos, pos, impulse);
			pBody->ApplyLinearImpulseToCenter(sfToB2(force), true);
		}
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

bool CSystemBox2D::inside(const b2Vec2 &cp1, const b2Vec2 &cp2, const b2Vec2 &p) noexcept
{
	return (cp2.x-cp1.x)*(p.y-cp1.y) > (cp2.y-cp1.y)*(p.x-cp1.x);
}

b2Vec2 CSystemBox2D::intersection(const b2Vec2 &cp1, const b2Vec2 &cp2, const b2Vec2 &s, const b2Vec2 &e) noexcept
{
	b2Vec2 dc(cp1.x - cp2.x, cp1.y - cp2.y);
	b2Vec2 dp(s.x - e.x, s.y - e.y);
	float n1 = cp1.x * cp2.y - cp1.y * cp2.x;
	float n2 = s.x * e.y - s.y * e.x;
	float n3 = 1.0 / (dc.x * dp.y - dc.y * dp.x);
	return b2Vec2((n1*dp.x - n2*dc.x) * n3, (n1*dp.y - n2*dc.y) * n3);
}

//http://rosettacode.org/wiki/Sutherland-Hodgman_polygon_clipping#JavaScript
//Note that this only works when fB is a convex polygon, but we know all
//fixtures in Box2D are convex, so that will not be a problem
bool CSystemBox2D::findIntersectionOfFixtures(b2Fixture* pFA, b2Fixture* pFB, std::vector<b2Vec2> *pOutputVertices) noexcept
{
	//currently this only handles polygon vs polygon
	if (pFA->GetShape()->GetType() != b2Shape::e_polygon ||
			pFB->GetShape()->GetType() != b2Shape::e_polygon)
	{
		return false;
	}

	b2PolygonShape* polyA = (b2PolygonShape*)pFA->GetShape();
	b2PolygonShape* polyB = (b2PolygonShape*)pFB->GetShape();

	//fill subject polygon from fixtureA polygon
	for (int i = 0; i < polyA->m_count; i++)
		pOutputVertices->push_back(pFA->GetBody()->GetWorldPoint(polyA->m_vertices[i]));

	//fill clip polygon from fixtureB polygon
	std::vector<b2Vec2> clipPolygon;
	for (int i = 0; i < polyB->m_count; i++)
		clipPolygon.push_back(pFB->GetBody()->GetWorldPoint(polyB->m_vertices[i]));

	b2Vec2 cp1 = clipPolygon[clipPolygon.size()-1];
	for (size_t j = 0; j < clipPolygon.size(); j++)
	{
		b2Vec2 cp2 = clipPolygon[j];
		if (pOutputVertices->empty())
			return false;
		std::vector<b2Vec2> inputList = *pOutputVertices;
		pOutputVertices->clear();
		b2Vec2 s = inputList[inputList.size() - 1]; //last on the input list
		for (size_t i = 0; i < inputList.size(); i++)
		{
			b2Vec2 e = inputList[i];
			if (inside(cp1, cp2, e))
			{
				if (!inside(cp1, cp2, s))
					pOutputVertices->push_back(intersection(cp1, cp2, s, e));
				pOutputVertices->push_back(e);
			}
			else if (inside(cp1, cp2, s))
				pOutputVertices->push_back(intersection(cp1, cp2, s, e));
			s = e;
		}
		cp1 = cp2;
	}

	return !pOutputVertices->empty();
}

//Taken from b2PolygonShape.cpp
b2Vec2 CSystemBox2D::computeCentroid(const std::vector<b2Vec2> &vs, float *pArea) noexcept
{
	int count = (int)vs.size();
	b2Assert(count >= 3);

	b2Vec2 c;
	c.Set(0.0f, 0.0f);
	*pArea = 0.0f;

	// pRef is the reference point for forming triangles.
	// Its location doesnt change the result (except for rounding error).
	b2Vec2 pRef(0.0f, 0.0f);

	const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < count; ++i)
	{
		// Triangle vertices.
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = vs[i];
		b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		*pArea += triangleArea;

		// Area weighted centroid
		c += triangleArea * inv3 * (p1 + p2 + p3);
	}

	// Centroid
	if (*pArea > b2_epsilon)
		c *= 1.0f / *pArea;
	else
		*pArea = 0;
	return c;
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
