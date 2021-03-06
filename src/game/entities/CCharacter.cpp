/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/client/CClient.hpp>
#include <engine/client/CAssetManager.hpp>
#include <engine/client/CSystemSound.hpp>
#include "CCharacter.hpp"
#include "CProjectile.hpp"
#include "CHitBox.hpp"
#include "CFire.hpp"
#include <game/controllers/CControllerMain.hpp>


const float CCharacter::SIZE = 28.0f;
const long CCharacter::ANIM_TIME = 150;
const unsigned int CCharacter::ANIM_SUBRECTS = 4;
const sf::Vector2f CCharacter::BASE_ANTENNA_SIZE = sf::Vector2f(5.0f, 2.0f);
const CB2BodyInfo CCharacter::ms_BodyInfo = CB2BodyInfo(1.5f, 1.25f, 0.1f, 0.0f, b2_dynamicBody, CAT_CHARACTER_PLAYER);
CCharacter::CCharacter(const sf::Vector2f &pos, float rot, class CPlayer *pPlayer) noexcept
: CB2Circle(pos, SIZE, rot, sf::Color::White, ms_BodyInfo, CEntity::CHARACTER)
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

	m_HookState = HOOK_STATE_RETRACTED;
	m_HookDir = VECTOR_ZERO;
	m_pHookJoint = nullptr;
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

	CClient *pGame = CClient::getInstance();
	const sf::Vector2f shapePos = CSystemBox2D::b2ToSf(m_pBody->GetPosition());

	if (isAlive())
	{
		// Apply Movement
		const sf::Vector2f charVel = CSystemBox2D::b2ToSf(m_pBody->GetLinearVelocity());
	    const float v = g_Config.m_CharacterImpulse;
	    sf::Vector2f force = VECTOR_ZERO;
	    // Can't impulse in air
	    if ((m_State&MOVE_STATE_LEFT) || (m_State&MOVE_STATE_RIGHT))
	    {
			CClient *pGame = CClient::getInstance();
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
	    		m_pBody->ApplyLinearImpulse(CSystemBox2D::sfToB2(force), m_pBody->GetWorldCenter(), true);
	    }

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
				m_pBody->SetLinearDamping((m_State == MOVE_STATE_STOP)?15.0f:0.0f);
				m_pBody->SetAngularDamping((m_State == MOVE_STATE_STOP)?15.0f:0.0f);
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

			pGame->Client()->Controller()->onCharacterDeath(this, nullptr);
		}

		if (m_HookState != HOOK_STATE_RETRACTED)
		{
			if (m_HookState == HOOK_STATE_FLYING)
			{
				m_HookPos = shapePos + m_HookDir * m_HookLength;
				CSystemBox2D *pSystemBox2D = pGame->Client()->getSystem<CSystemBox2D>();
				if (m_HookLength < g_Config.m_CharacterHookMaxLength)
				{
					m_HookLength += g_Config.m_CharacterHookFlyVel;
					CEntity *pEnt = pSystemBox2D->checkIntersectLine(shapePos, m_HookPos, &m_HookPos, this->getBody());
					if (pEnt)
					{
						pGame->Client()->Controller()->createImpactSparkMetal(m_HookPos);
						pGame->Client()->Controller()->createSmokeImpact(m_HookPos, -m_HookDir, 1.0f);

						m_HookLength = upm::vectorLength(m_HookPos - shapePos);
				    	const int tileIndex = pGame->Client()->Controller()->Context()->Map().getWorldTileIndex(CSystemBox2D::b2ToSf(pEnt->getBody()->GetPosition()), pGame->Client()->Controller()->Context()->Map().getGameLayer());
				    	if (tileIndex != TILE_NOHOOK)
				    	{
							m_HookState = HOOK_STATE_ATTACHED;
							b2DistanceJointDef jointDef;
							jointDef.collideConnected = true;
							jointDef.frequencyHz = g_Config.m_CharacterHookFrequency;
							jointDef.dampingRatio = g_Config.m_CharacterHookDampingRatio;
							jointDef.Initialize(getBody(), pEnt->getBody(), getBody()->GetWorldCenter(), CSystemBox2D::sfToB2(m_HookPos-m_HookDir*3.0f));
							m_pHookJoint = pGame->Client()->getSystem<CSystemBox2D>()->getWorld()->CreateJoint(&jointDef);
							b2DistanceJoint *pDistanceJoint = static_cast<b2DistanceJoint*>(m_pHookJoint);
							pDistanceJoint->SetLength(-g_Config.m_CharacterHookRetractVel);
				    	} else
				    	{
				    		m_HookState = HOOK_STATE_RETRACTING;
				    	}
					}
				}
				else
				{
					m_HookState = HOOK_STATE_RETRACTING;
				}
			} else if (m_HookState == HOOK_STATE_RETRACTING)
			{
				m_HookPos = shapePos + m_HookDir * m_HookLength;
				if (m_pHookJoint)
				{
					pGame->Client()->getSystem<CSystemBox2D>()->destroyJoint(m_pHookJoint);
					m_pHookJoint = nullptr;
				}

				m_HookLength -= g_Config.m_CharacterHookRetractVel;
				if (m_HookLength <= 0.0f)
				{
					m_HookState = HOOK_STATE_RETRACTED;
				}
			} else if (m_HookState == HOOK_STATE_ATTACHED && m_pHookJoint)
			{
				b2DistanceJoint *pDistanceJoint = static_cast<b2DistanceJoint*>(m_pHookJoint);
				m_HookPos = CSystemBox2D::b2ToSf(m_pHookJoint->GetAnchorB());
				m_HookLength = CSystemBox2D::b2ToSf(pDistanceJoint->GetLength());
				m_HookDir = upm::vectorNormalize(m_HookPos-shapePos);
			}
		}

		m_LastState = m_State;
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

		CClient *pGame = CClient::getInstance();
		const sf::Vector2f CharPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
		const sf::Vector2f CharDir = upm::vectorNormalize(pGame->Client()->mapPixelToCoords(pGame->Client()->UI().getMousePos(), pGame->Client()->Camera()) - CharPos);

		switch (m_ActiveWeapon)
		{
			case WEAPON_GRENADE_LAUNCHER:
			case WEAPON_VISCOSITY_LAUNCHER:
			{
				new CProjectile(CharPos+CharDir*(CCharacter::SIZE), sf::Vector2f(30.0f, 22.0f), 0.0f, CharDir, getOwner(), m_ActiveWeapon, 0);
			} break;
			case WEAPON_JET_PACK:
			{
				m_pBody->ApplyLinearImpulse(CSystemBox2D::sfToB2(-CharDir*g_Config.m_aWeaponsInfo[m_ActiveWeapon].m_Energy), m_pBody->GetWorldCenter(), true);
				new CFire(CharPos+CharDir*15.0f, 0.0f, CharDir, g_Config.m_aWeaponsInfo[m_ActiveWeapon].m_Speed, g_Config.m_aWeaponsInfo[m_ActiveWeapon].m_LifeTime);
			} break;
		}

		m_TimerFire = ups::timeGet();
	}

	m_Fire = true;
}

void CCharacter::doHook(bool state) noexcept
{
	if (state && m_HookState == HOOK_STATE_RETRACTED)
	{
		m_HookState = HOOK_STATE_FLYING;
		CClient *pGame = CClient::getInstance();
		const sf::Vector2f CharPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
		m_HookDir = upm::vectorNormalize(pGame->Client()->mapPixelToCoords(pGame->Client()->UI().getMousePos(), pGame->Client()->Camera()) - CharPos);
		m_HookLength = 0.0f;
	}
	else if (!state && m_HookState != HOOK_STATE_RETRACTED)
	{
		m_HookState = HOOK_STATE_RETRACTING;
	}
}

void CCharacter::setVisible(bool visible) noexcept
{
	m_Visible = visible;
	getBody()->SetActive(m_Visible);
}

void CCharacter::setAlive(bool state) noexcept
{
	m_Alive = state;
	if (m_Alive)
		setHealth(MAX_HEALTH_PLAYER);
	else
		setHealth(0);
}

void CCharacter::takeHealth(int amount, class CPlayer *pPlayer) noexcept
{
	const sf::Vector2f charPos = CSystemBox2D::b2ToSf(getBody()->GetPosition());
	m_Health = upm::max(m_Health-amount, 0);
	CClient *pGame = CClient::getInstance();
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

void CCharacter::setMoveState(int state) noexcept
{
	m_State = state;
	if (state == MOVE_STATE_STOP)
	{
		m_LastState = m_State;
		return;
	}
}

void CCharacter::teleport(const sf::Vector2f &worldPosTo) noexcept
{
	getBody()->SetTransform(CSystemBox2D::sfToB2(worldPosTo), getBody()->GetAngle());
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
		CClient *pGame = CClient::getInstance();
		pGame->Client()->Controller()->createSmokeImpact(worldPos, sf::Vector2f(-1.0f, 0.0f), upm::vectorLength(charVel)/500.0f);
	} else if ((m_State&MOVE_STATE_RIGHT) && charVel.x < -500.0f)
	{
		CClient *pGame = CClient::getInstance();
		pGame->Client()->Controller()->createSmokeImpact(worldPos, sf::Vector2f(1.0f, 0.0f), upm::vectorLength(charVel)/500.0f);
	}
}
