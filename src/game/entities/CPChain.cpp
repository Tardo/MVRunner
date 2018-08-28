/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CAntenna.hpp"
#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>

const CB2BodyInfo CAntenna::ms_BodyInfo = CB2BodyInfo(0.1f, 0.7f, 0.1f, 0.0f, b2_dynamicBody, CAT_ANTENNA, false, CAT_BUILD|CAT_GENERIC|CAT_HITBOX);
const CB2BodyInfo CAntenna::ms_BodyInfoBase = CB2BodyInfo(0.1f, 0.7f, 0.1f, 0.0f, b2_staticBody, CAT_ANTENNA, false, 0x0);
const sf::Vector2f CAntenna::SLAVE_SIZE = sf::Vector2f(5.0f, 30.0f);
CAntenna::CAntenna(const sf::Vector2f &pos, const sf::Color &color, int textureId) noexcept
: CEntity(CEntity::ANTENNA)
{
	m_TextureId = textureId;

	m_pPolyBase = new CB2Polygon(pos, sf::Vector2f(2.0f, 2.0f), 0.0f, color, ms_BodyInfoBase);
	m_pBody = m_pPolyBase->getBody();

	CGame *pGame = CGame::getInstance();
	b2RevoluteJointDef jointDef;
	sf::Vector2f slavePos;
	for (int i=0; i<NUM_SLAVES; ++i)
	{
		slavePos = pos + sf::Vector2f(0.0f, -(SLAVE_SIZE.y-2.0f)*i);
		m_apPolygons[i] = new CB2Polygon(slavePos, SLAVE_SIZE, 0.0f, color, ms_BodyInfo);
		if (i == 0)
			jointDef.Initialize(m_pPolyBase->getBody(), m_apPolygons[i]->getBody(), CSystemBox2D::sfToB2(slavePos));
		else
			jointDef.Initialize(m_apPolygons[i-1]->getBody(), m_apPolygons[i]->getBody(), CSystemBox2D::sfToB2(slavePos));
		jointDef.lowerAngle = -0.0f * b2_pi; // -90 degrees
		jointDef.upperAngle = 0.0f * b2_pi; // 45 degrees
		jointDef.enableLimit = true;
		pGame->Client()->getSystem<CSystemBox2D>()->getWorld()->CreateJoint(&jointDef);
	}

}
CAntenna::~CAntenna() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CAntenna", "Deleted");
	#endif

	for (int i=0; i<NUM_SLAVES; ++i)
	{
		CGame *pGame = CGame::getInstance();
		pGame->Client()->getSystem<CSystemBox2D>()->destroyBody(m_apPolygons[i]->getBody());
	}
}

void CAntenna::tick() noexcept
{ }
