/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CFire.hpp"
#include <engine/client/CClient.hpp>
#include <engine/client/CAssetManager.hpp>

const float CFire::SIZE = 8.0f;
const CB2BodyInfo CFire::ms_BodyInfo = CB2BodyInfo(0.2f, 0.7f, 0.1f, 0.0f, b2_kinematicBody, CAT_FIRE, false, CAT_FIRE|CAT_BUILD|CAT_GENERIC);
CFire::CFire(const sf::Vector2f &pos, float rot, const sf::Vector2f &dir, float force, float lifeTime) noexcept
: CB2Circle(pos, SIZE, rot, sf::Color::Transparent, ms_BodyInfo, CEntity::FIRE)
{
	m_LifeTime = lifeTime;
	m_Dir = dir;
	m_Force = force;

	CClient *pGame = CClient::getInstance();

	pGame->Client()->Controller()->createFireBall(this, lifeTime-0.01f);
	if (getBody())
	{
		getBody()->SetBullet(true);
		getBody()->SetFixedRotation(true);
		b2MassData massData;
		getBody()->GetMassData(&massData);
		massData.mass = 0.0000001f;
		getBody()->SetMassData(&massData);
		getBody()->SetTransform(getBody()->GetPosition(), upm::degToRad(upm::vectorAngle(m_Dir)+90.0f));
		getBody()->SetLinearVelocity(CSystemBox2D::sfToB2(m_Dir*m_Force));
	}

	m_TickStart = ups::timeGet();
}
CFire::~CFire() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CFire", "Deleted");
	#endif
}

void CFire::tick() noexcept
{
	CB2Circle::tick();
	CClient *pGame = CClient::getInstance();

	const sf::Vector2f shapePos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
	const unsigned long elapsedTicks = ups::timeGet()-m_TickStart;

	getBody()->SetLinearVelocity(CSystemBox2D::sfToB2(m_Dir*m_Force));

	if (m_LifeTime != 0.0f)
	{
		if (elapsedTicks > ups::timeFreq()*m_LifeTime || pGame->Client()->isClipped(shapePos, 512.0f))
			destroy();
	}

	m_LastPos = shapePos;
}

void CFire::onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept
{
	CB2Circle::onContact(pEntity, worldPos);

	if (pEntity->getType() == CEntity::FIRE)
		return;

	CClient *pGame = CClient::getInstance();
	pGame->Client()->Controller()->createFireTrailLarge(worldPos);
}
