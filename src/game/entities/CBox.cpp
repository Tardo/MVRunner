/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CBox.hpp"
#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/CPlayer.hpp>
#include "CHitBox.hpp"

const CB2BodyInfo CBox::ms_BodyInfo = CB2BodyInfo(0.2f, 0.7f, 0.1f, b2_dynamicBody, CAT_BOX, false, CAT_BOX|CAT_CHARACTER_PLAYER|CAT_BUILD|CAT_PROJECTILE|CAT_PROJECTILE_MAP|CAT_GENERIC|CAT_HITBOX|CAT_FIRE);
CBox::CBox(const sf::Vector2f &pos, const sf::Vector2f &size, const sf::Vector2f &dir, float force, unsigned int typeBox, unsigned int level, const sf::Color &color) noexcept
: CB2Polygon(pos, level, size, color, ms_BodyInfo, CEntity::BOX)
{
	m_BoxType = typeBox;
	m_Dir = dir;
	m_Health = MAX_HEALTH_BOX;

	CGame *pGame = CGame::getInstance();
	m_pSystemFx = pGame->Client()->getSystem<CSystemFx>();

	switch (m_BoxType)
	{
		case BOX_WOOD:
		{
			getShape()->setTexture(pGame->Client()->Assets().getTexture(CAssetManager::TEXTURE_BOX));
			if (getBody())
			{
				getBody()->SetFixedRotation(true);
				getBody()->SetLinearDamping(15.0f);
			}
		} break;
	}

	m_TickStart = ups::timeGet();
}
CBox::~CBox() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CBox", "Deleted");
	#endif
}

void CBox::tick() noexcept
{
	CB2Polygon::tick();

	const sf::Vector2f &shapePos = getShape()->getPosition();

	if (m_BoxType == BOX_WOOD)
	{
		if (m_Health <= 0)
		{
			new CHitBox(shapePos, sf::Vector2f(4.0f, 16.0f), sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f)), upm::floatRand(0.4f, 1.0f), upm::floatRand(4.0f, 10.0f), HITBOX_WOOD_BROKEN_1);
			new CHitBox(shapePos, sf::Vector2f(4.0f, 16.0f), sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f)), upm::floatRand(0.4f, 1.0f), upm::floatRand(4.0f, 10.0f), HITBOX_WOOD_BROKEN_1);
			new CHitBox(shapePos, sf::Vector2f(16.0f, 4.0f), sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f)), upm::floatRand(0.4f, 1.0f), upm::floatRand(4.0f, 10.0f), HITBOX_WOOD_BROKEN_2);
			for (int i=0; i<4; ++i)
				new CHitBox(shapePos, sf::Vector2f(4.0f, 6.0f), sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f)), upm::floatRand(0.4f, 1.0f), upm::floatRand(4.0f, 10.0f), HITBOX_WOOD_BROKEN_3);

			destroy();
			return;
		}

		// Sonido
		m_Sound.setPosition(shapePos.x, 0.0f, shapePos.y);
		const b2Vec2 &curVel = getBody()->GetLinearVelocity();
		const float curVelLen = upm::vectorLength(sf::Vector2f(curVel.x,curVel.y));
		if (curVelLen > 0.1f && m_Sound.getStatus() == sf::Sound::Stopped)
			m_Sound.play();
		else if (curVelLen == 0.0f && m_Sound.getStatus() == sf::Sound::Playing)
			m_Sound.stop();
	}

	m_LastPos = shapePos;
}

void CBox::takeHealth(int amount, class CPlayer *pPlayer) noexcept
{
	CB2Polygon::takeHealth(amount, pPlayer);
}

void CBox::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	CB2Polygon::draw(target, states);
}
