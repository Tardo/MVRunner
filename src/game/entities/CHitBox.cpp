/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CHitBox.hpp"
#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>

const CB2BodyInfo CHitBox::ms_BodyInfo = CB2BodyInfo(0.1f, 0.7f, 0.1f, b2_dynamicBody, CAT_HITBOX, false, CAT_CHARACTER_PLAYER|CAT_BOX|CAT_BUILD|CAT_GENERIC|CAT_HITBOX);
CHitBox::CHitBox(const sf::Vector2f &pos, const sf::Vector2f &size, const sf::Vector2f &dir, float force, float lifeTime, unsigned int typeHitBox, unsigned int level, const sf::Color &color) noexcept
: CB2Polygon(pos, level, size, color, ms_BodyInfo, CEntity::HITBOX)
{
	m_HitBoxType = typeHitBox;
	m_LifeTime = lifeTime;
	m_Dir = dir;

	CGame *pGame = CGame::getInstance();
	int textId = -1;

	switch (m_HitBoxType)
	{
		case HITBOX_CHARACTER_BODY:
		case HITBOX_CHARACTER_LEG:
		case HITBOX_CHARACTER_HEAD:
		case HITBOX_EXPLOSION_TRASH:
		{
			if (m_HitBoxType == HITBOX_EXPLOSION_TRASH)
			{
				textId = -1;
				getShape()->setFillColor(sf::Color::Black);
			}

			getShape()->setTexture(pGame->Client()->Assets().getTexture(textId));

			if (getBody())
			{
				getBody()->SetAngularDamping(5.25f);
				getBody()->SetLinearDamping(5.25f);

				b2MassData massData;
				getBody()->GetMassData(&massData);
				massData.mass = 0.001f;
				getBody()->SetMassData(&massData);
			}
		} break;
		case HITBOX_WOOD_BURNED_1:
		case HITBOX_WOOD_BURNED_2:
		case HITBOX_WOOD_BURNED_3:
		case HITBOX_WOOD_BROKEN_1:
		case HITBOX_WOOD_BROKEN_2:
		case HITBOX_WOOD_BROKEN_3:
		{
			if (getBody())
			{
				getBody()->SetAngularDamping(5.25f);
				getBody()->SetLinearDamping(5.25f);
			}

			getShape()->setTexture(pGame->Client()->Assets().getTexture(textId));
		} break;
	}

	if (force > 0.0f && getBody())
		getBody()->ApplyForceToCenter(CSystemBox2D::sfToB2(dir*force), true);

	m_TickStart = ups::timeGet();
}
CHitBox::~CHitBox() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CHitBox", "Deleted");
	#endif
}

void CHitBox::tick() noexcept
{
	CB2Polygon::tick();
	CGame *pGame = CGame::getInstance();

	const sf::Vector2f &shapePos = getShape()->getPosition();
	const unsigned long elapsedTicks = ups::timeGet()-m_TickStart;

	if (m_LifeTime != 0.0f)
	{
		if (elapsedTicks > ups::timeFreq()*m_LifeTime || pGame->Client()->isClipped(shapePos, 512.0f))
			destroy();
	}

	if (m_HitBoxType == HITBOX_CHARACTER_BODY ||
		m_HitBoxType == HITBOX_CHARACTER_LEG ||
		m_HitBoxType == HITBOX_CHARACTER_HEAD ||
		m_HitBoxType == HITBOX_EXPLOSION_TRASH)
	{
		if (getBody() && m_LastPos != shapePos)
		{
			if (m_HitBoxType == HITBOX_EXPLOSION_TRASH && elapsedTicks < ups::timeFreq()*0.25f)
				pGame->Client()->getSystem<CSystemFx>()->createImpactSparkMetal(shapePos);
			else if ((m_HitBoxType == HITBOX_CHARACTER_HEAD || m_HitBoxType == HITBOX_CHARACTER_BODY || m_HitBoxType == HITBOX_CHARACTER_LEG) && elapsedTicks < ups::timeFreq()*0.75f)
				pGame->Client()->getSystem<CSystemFx>()->createBloodSpark(shapePos, 10.0f);
		}
	}

	m_LastPos = shapePos;
}
