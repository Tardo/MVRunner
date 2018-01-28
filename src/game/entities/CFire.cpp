/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CFire.hpp"
#include "CBox.hpp"
#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>

const float CFire::SIZE = 2.5f;
const CB2BodyInfo CFire::ms_BodyInfo = CB2BodyInfo(0.2f, 0.7f, 0.1f, b2_dynamicBody, CAT_FIRE, false, CAT_CHARACTER_PLAYER|CAT_FIRE|CAT_BUILD|CAT_PROJECTILE_MAP|CAT_PROJECTILE|CAT_GENERIC|CAT_BOX);
CFire::CFire(const sf::Vector2f &pos, const sf::Vector2f &dir, float force, float lifeTime) noexcept
: CB2Circle(pos, 1, SIZE, sf::Color::Transparent, ms_BodyInfo, CEntity::FIRE)
{
	m_LifeTime = lifeTime;
	m_Dir = dir;
	m_Force = force;

	CGame *pGame = CGame::getInstance();

	pGame->Client()->getSystem<CSystemFx>()->createFireBall(this);
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
	CGame *pGame = CGame::getInstance();

	const sf::Vector2f &shapePos = getShape()->getPosition();
	const unsigned long elapsedTicks = ups::timeGet()-m_TickStart;

	if (getBody())
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
	if (pEntity->getType() == CEntity::FIRE)
		return;

	pEntity->takeHealth(1, nullptr);

	CGame *pGame = CGame::getInstance();
	pGame->Client()->getSystem<CSystemFx>()->createFireTrailLarge(worldPos);
}
