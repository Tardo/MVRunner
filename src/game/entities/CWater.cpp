/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CWater.hpp"
#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>

const CB2BodyInfo CWater::ms_BodyInfo = CB2BodyInfo(0.2f, 1.2f, 0.1f, 0.0f, b2_staticBody, CAT_WATER, true, CAT_BUILD|CAT_GENERIC|CAT_CHARACTER_PLAYER|CAT_PROJECTILE);
CWater::CWater(const sf::Vector2f &pos, const sf::Vector2f &size, float rot) noexcept
: CB2Polygon(pos, size, rot, sf::Color::Transparent, ms_BodyInfo, CEntity::WATER)
{
	m_pFixture = &getBody()->GetFixtureList()[0];
}
CWater::~CWater() noexcept
{
	m_vInEntities.clear();

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CWater", "Deleted");
	#endif
}

void CWater::tick() noexcept
{
	const b2Vec2 gravity(0, GRAVITY);

	std::vector<b2Vec2> intersectionPoints;
	std::vector<CEntity*>::const_iterator cit = m_vInEntities.cbegin();
	while (cit != m_vInEntities.cend())
	{
		if ((*cit)->getBody())
		{
			for (b2Fixture *pFixture = (*cit)->getBody()->GetFixtureList(); pFixture; pFixture = pFixture->GetNext())
			{
				intersectionPoints.clear();
				if (CSystemBox2D::findIntersectionOfFixtures(m_pFixture, pFixture, &intersectionPoints))
				{
					//find centroid
					float area = 0;
					const b2Vec2 centroid = CSystemBox2D::computeCentroid(intersectionPoints, &area);

					//apply buoyancy force (m_pFixture is the fluid)
					const float displacedMass = m_pFixture->GetDensity() * area;
					const b2Vec2 buoyancyForce = displacedMass * -gravity;
					pFixture->GetBody()->ApplyForce(buoyancyForce, centroid, true);

					//apply drag separately for each polygon edge
					for (size_t i = 0; i < intersectionPoints.size(); ++i)
					{
						//the end points and mid-point of this edge
						b2Vec2 v0 = intersectionPoints[i];
						b2Vec2 v1 = intersectionPoints[(i+1)%intersectionPoints.size()];
						b2Vec2 midPoint = 0.5f * (v0+v1);

						//find relative velocity between object and fluid at edge midpoint
						b2Vec2 velDir = pFixture->GetBody()->GetLinearVelocityFromWorldPoint(midPoint) -
										m_pFixture->GetBody()->GetLinearVelocityFromWorldPoint(midPoint);
						float vel = velDir.Normalize();

						b2Vec2 edge = v1 - v0;
						float edgeLength = edge.Normalize();
						b2Vec2 normal = b2Cross(-1, edge); //gets perpendicular vector

						float dragDot = b2Dot(normal, velDir);
						if (dragDot < 0)
						  continue; //normal points backwards - this is not a leading edge

						float dragMag = dragDot * edgeLength * m_pFixture->GetDensity() * vel * vel;
						b2Vec2 dragForce = dragMag * -velDir;
						pFixture->GetBody()->ApplyForce(dragForce, midPoint, true);

						//apply lift
						float liftDot = b2Dot(edge, velDir);
						float liftMag =  (dragDot * liftDot) * edgeLength * m_pFixture->GetDensity() * vel * vel;
						b2Vec2 liftDir = b2Cross(1,velDir); //gets perpendicular vector
						b2Vec2 liftForce = liftMag * liftDir;
						pFixture->GetBody()->ApplyForce(liftForce, midPoint, true);
					}
				} else if (pFixture->GetType() == b2Shape::e_circle) {
					b2CircleShape* pCircleShape = (b2CircleShape*)pFixture->GetShape();
					//apply buoyancy force (m_pFixture is the fluid)
					const float displacedMass = m_pFixture->GetDensity() * I_PI * pCircleShape->m_radius*pCircleShape->m_radius;
					const b2Vec2 buoyancyForce = displacedMass * -gravity;
					pFixture->GetBody()->ApplyForceToCenter(buoyancyForce, true);
				}
			}
		}
		++cit;
	}
}

void CWater::onSensorIn(CEntity *pEntity) noexcept
{
	CB2Polygon::onSensorIn(pEntity);

	if (pEntity->getBody()->GetType() == b2_dynamicBody)
		m_vInEntities.push_back(pEntity);
}

void CWater::onSensorOut(CEntity *pEntity) noexcept
{
	CB2Polygon::onSensorOut(pEntity);

	std::vector<CEntity*>::iterator it = m_vInEntities.begin();
	while (it != m_vInEntities.cend())
	{
		if ((*it) == pEntity)
		{
			m_vInEntities.erase(it);
			break;
		}
		++it;
	}
}
