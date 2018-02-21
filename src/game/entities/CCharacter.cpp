/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemSound.hpp>
#include "CCharacter.hpp"
#include "CProjectile.hpp"
#include "CHitBox.hpp"
#include "CFire.hpp"
#include <game/controllers/CControllerMain.hpp>


const float CCharacter::SIZE = 28.0f;
const long CCharacter::ANIM_TIME = 150;
const unsigned int CCharacter::ANIM_SUBRECTS = 4;
const CB2BodyInfo CCharacter::ms_BodyInfo = CB2BodyInfo(1.5f, 0.7f, 0.1f, b2_dynamicBody, CAT_CHARACTER_PLAYER);
CCharacter::CCharacter(const sf::Vector2f &pos, class CPlayer *pPlayer) noexcept
: CB2Circle(pos, SIZE, sf::Color::White, ms_BodyInfo, CEntity::CHARACTER)
{
	m_pPlayer = pPlayer;
	m_Visible = true;
	m_Health = MAX_HEALTH_PLAYER;
	m_Alive = true;
	m_Fire = false;
	m_Jumps = 0;
	m_ActiveWeapon = WEAPON_GRENADE_LAUNCHER;
	m_TimerDamageIndicator = 0;
	m_TimerFire = 0;
	m_FlagDamage = false;
	m_TimerHeartbeat = 0;
	m_CharacterState = STATE_NORMAL;

	//CGame *pGame = CGame::getInstance();

	//getBody()->SetFixedRotation(true);

	giveWeapon(WEAPON_GRENADE_LAUNCHER, -1, -1);
	setActiveWeapon(WEAPON_GRENADE_LAUNCHER);
}
CCharacter::~CCharacter() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CCharacter", "Deleted");
	#endif
}

void CCharacter::tick() noexcept
{
	CB2Circle::tick();

	if (!m_pBody || !m_Visible)
		return;

	CGame *pGame = CGame::getInstance();

	if (isAlive())
	{
		const sf::Vector2f shapePos = CSystemBox2D::b2ToSf(getBody()->GetPosition());

		// Check Ground
		CSystemBox2D *pB2Engine = pGame->Client()->getSystem<CSystemBox2D>();
		const sf::Vector2f groundColPos = shapePos + sf::Vector2f(0.0f, CCharacter::SIZE+5.0f);
		const bool isGrounded = (pB2Engine->checkIntersectLine(shapePos, groundColPos, 0x0, getBody(), CAT_CHARACTER_PLAYER) != 0x0);
		if (isGrounded)
		{
			if (m_CharacterState&STATE_ROTATE)
			{
				m_pBody->SetLinearDamping(0.0f);
				m_pBody->SetAngularDamping(0.0f);
			}
			else
			{
				m_pBody->SetLinearDamping((m_State == MOVE_STATE_STOP)?15.0f:1.5f);
				m_pBody->SetAngularDamping((m_State == MOVE_STATE_STOP)?15.0f:1.5f);
			}
			m_Jumps = 0;
		} else {
			m_pBody->SetLinearDamping(0.0f);
			m_pBody->SetAngularDamping(0.3f);
			if (m_Jumps == 0)
				++m_Jumps;
		}

		if (m_Health <= 0)
		{
			pGame->Client()->Controller()->createBlood(shapePos);
			m_Alive = false;
			m_Visible = false;
			getBody()->SetActive(false);

			pGame->Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_KILL, shapePos, 15.0f, 20.0f);
			// Gore
			new CHitBox(shapePos, sf::Vector2f(6.0f, 6.0f), sf::Vector2f(upm::floatRand(-1.0f,1.0f), upm::floatRand(-1.0f,1.0f)), upm::floatRand(6.0f, 12.0f), g_Config.m_HitBoxGoreDuration, HITBOX_CHARACTER_HEAD);
			for (int i=0; i<2; i++)
				new CHitBox(shapePos, sf::Vector2f(4.0f, 8.0f), sf::Vector2f(upm::floatRand(-1.0f,1.0f), upm::floatRand(-1.0f,1.0f)), upm::floatRand(6.0f, 12.0f), g_Config.m_HitBoxGoreDuration, HITBOX_CHARACTER_LEG);
			for (int i=0; i<4; i++)
				new CHitBox(shapePos, sf::Vector2f(4.0f, 4.0f), sf::Vector2f(upm::floatRand(-1.0f,1.0f), upm::floatRand(-1.0f,1.0f)), upm::floatRand(6.0f, 12.0f), g_Config.m_HitBoxGoreDuration, HITBOX_CHARACTER_BODY);

			pGame->Client()->Controller()->onCharacterDeath(this, nullptr);
		}
	}
}

void CCharacter::giveHealth(int amount) noexcept
{
	m_Health = upm::min(m_Health+amount, MAX_HEALTH_PLAYER);
}

void CCharacter::doFire() noexcept
{
	if (!m_Visible || !m_Alive || m_ActiveWeapon < 0)
		return;

	if (ups::timeGet()-m_TimerFire > ups::timeFreq()*g_Config.m_aWeaponsInfo[m_ActiveWeapon].m_FireDelay)
	{
		if (m_aWeapons[m_ActiveWeapon].m_Ammo != -1 && m_aWeapons[m_ActiveWeapon].m_Ammo == 0)
			return;

		CGame *pGame = CGame::getInstance();
		const sf::Vector2f CharPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
		const sf::Vector2f CharDir = upm::vectorNormalize(pGame->Client()->mapPixelToCoords(pGame->Client()->Controls().getMousePos(), pGame->Client()->Camera()) - CharPos);

		switch (m_ActiveWeapon)
		{
			case WEAPON_GRENADE_LAUNCHER:
			{
				new CProjectile(CharPos+CharDir*(CCharacter::SIZE), sf::Vector2f(30.0f, 22.0f), CharDir, g_Config.m_aWeaponsInfo[m_ActiveWeapon].m_Speed, getOwner(), m_ActiveWeapon, 0);
			} break;
			case WEAPON_JET_PACK:
			{
				m_pBody->ApplyLinearImpulseToCenter(CSystemBox2D::sfToB2(-CharDir*g_Config.m_WeaponJetPackEnergy), true);
				new CFire(CharPos+CharDir*(CCharacter::SIZE), CharDir, 40.f, 1.5f);
			} break;
		}

		m_TimerFire = ups::timeGet();
	}

	m_Fire = true;
}

void CCharacter::doHook() noexcept
{

}

void CCharacter::setVisible(bool visible) noexcept
{
	m_Visible = visible;
	getBody()->SetActive(m_Visible);
}

void CCharacter::takeHealth(int amount, class CPlayer *pPlayer) noexcept
{
	const sf::Vector2f charPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
	m_Health = upm::max(m_Health-amount, 0);
	CGame *pGame = CGame::getInstance();
	m_TimerDamageIndicator = ups::timeGet();
	pGame->Client()->Controller()->createPoints(charPos, -amount);
	pGame->Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_DAMAGE, charPos, 15.0f);
	if (m_Health == 1)
		m_TimerHeartbeat = ups::timeGet();
}


void CCharacter::doImpulse(sf::Vector2f dir, float energy) noexcept
{
	if (!m_pBody)
		return;

	m_pBody->ApplyForceToCenter(CSystemBox2D::sfToB2(dir*energy), true);
}

void CCharacter::move(int state) noexcept
{
	m_State = state;
	if (state == MOVE_STATE_STOP)
	{
		m_LastState = m_State;
		return;
	}

	const sf::Vector2f charVel = CSystemBox2D::b2ToSf(m_pBody->GetLinearVelocity());
    const float v = g_Config.m_CharacterImpulse;
    sf::Vector2f force = VECTOR_ZERO;
    // Can't impulse in air
    if ((m_State&MOVE_STATE_LEFT) || (m_State&MOVE_STATE_RIGHT))
    {
		CGame *pGame = CGame::getInstance();
		CSystemBox2D *pB2Engine = pGame->Client()->getSystem<CSystemBox2D>();
		const sf::Vector2f charPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
		const sf::Vector2f groundColPos = charPos + sf::Vector2f(0.0f, CCharacter::SIZE+5.0f);
		const bool isGrounded = (pB2Engine->checkIntersectLine(charPos, groundColPos, 0x0, getBody(), CAT_CHARACTER_PLAYER) != 0x0);
		if (!isGrounded)
			m_pBody->ApplyAngularImpulse((m_State&MOVE_STATE_LEFT)?-0.00008:0.00008, true);
		else
			m_pBody->ApplyAngularImpulse((m_State&MOVE_STATE_LEFT)?-0.00001:0.00001, true);
		force.x = (m_State&MOVE_STATE_LEFT)?-v:v;

		const sf::Vector2f endPos = charPos + sf::Vector2f((CCharacter::SIZE+5.0f)*(m_State&MOVE_STATE_LEFT?-1.0f:1.0f), 0.0f);
		if (pB2Engine->checkIntersectLine(charPos, endPos, 0x0, getBody()) != 0x0)
			force.x = 0.0f;

		const sf::Vector2f dir = upm::vectorNormalize(charVel);
		if ((isGrounded && abs(charVel.x) >= g_Config.m_CharacterMaxVelocity) || (!isGrounded && abs(charVel.x) >= g_Config.m_CharacterMaxVelocity/2.0f))
		{
			if (((m_State&MOVE_STATE_LEFT) && dir.x < 0) || ((m_State&MOVE_STATE_RIGHT) && dir.x > 0))
				force.x = 0.0f;
		}
    }

    // Only can jump one time
    if ((m_State&MOVE_STATE_UP) && m_Jumps < 2 && !(m_LastState&MOVE_STATE_UP))
    {
    	++m_Jumps;
    	//if (m_pBody->IsFixedRotation())
    	{
    		const sf::Vector2f vel(charVel.x, -g_Config.m_CharacterJumpImpulse);
    		m_pBody->SetLinearVelocity(CSystemBox2D::sfToB2(vel));
    	}
    } else
    {
    	if (!(m_CharacterState&STATE_ROTATE))
    		m_pBody->ApplyLinearImpulseToCenter(CSystemBox2D::sfToB2(force), true);
    }

    m_LastState = m_State;
}

void teleport(const sf::Vector2f &worldPosTo) noexcept
{
	//m_State
}

void CCharacter::setActiveWeapon(int wid) noexcept
{
	if (wid == -1)
		m_ActiveWeapon = -1;
	if (wid < 0 || wid >= NUM_WEAPONS || !m_aWeapons[wid].m_Active)
		return;

	m_ActiveWeapon = wid;
}

void CCharacter::giveWeapon(int wid, int ammo, int maxammo) noexcept
{
	if (wid < 0 || wid >= NUM_WEAPONS)
		return;

	m_aWeapons[wid].m_Active = true;
	m_aWeapons[wid].m_Ammo = ammo;
	m_aWeapons[wid].m_MaxAmmo = maxammo;
}

void CCharacter::setCharacterState(int state)
{
	if (m_CharacterState == state)
		return;

	m_CharacterState = state;
	m_TimerCharacterState = ups::timeGet();
}

void CCharacter::onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept
{
	const sf::Vector2f charVel = CSystemBox2D::b2ToSf(getBody()->GetLinearVelocity());
	if ((m_State&MOVE_STATE_LEFT) && charVel.x > 500.0f)
	{
		CGame *pGame = CGame::getInstance();
		pGame->Client()->Controller()->createSmokeImpact(worldPos, sf::Vector2f(-1.0f, 0.0f), upm::vectorLength(charVel)/500.0f);
	} else if ((m_State&MOVE_STATE_RIGHT) && charVel.x < -500.0f)
	{
		CGame *pGame = CGame::getInstance();
		pGame->Client()->Controller()->createSmokeImpact(worldPos, sf::Vector2f(1.0f, 0.0f), upm::vectorLength(charVel)/500.0f);
	}
}

/** SENSOR **/
void CCharacter::onSensorIn(CEntity *pEntity) noexcept
{
	m_vpSensorEntities.push_back(pEntity);
}
void CCharacter::onSensorOut(CEntity *pEntity) noexcept
{
	std::vector<CEntity*>::iterator itEnt = m_vpSensorEntities.begin();
	while (itEnt != m_vpSensorEntities.end())
	{
		if ((*itEnt) == pEntity)
		{
			itEnt = m_vpSensorEntities.erase(itEnt);
			continue;
		}
		++itEnt;
	}
}
