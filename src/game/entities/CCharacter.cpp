/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemSound.hpp>
#include "CCharacter.hpp"
#include "CProjectile.hpp"
#include "CHitBox.hpp"
#include "CBox.hpp"
#include <game/controllers/CControllerMain.hpp>


const float CCharacter::SIZE = 28.0f;
const long CCharacter::ANIM_TIME = 150;
const unsigned int CCharacter::ANIM_SUBRECTS = 4;
const CB2BodyInfo CCharacter::ms_BodyInfo = CB2BodyInfo(0.01f, 0.7f, 0.1f, b2_dynamicBody, CAT_CHARACTER_PLAYER);
CCharacter::CCharacter(const sf::Vector2f &pos, const sf::Vector2f &dir, class CPlayer *pPlayer) noexcept
: CB2Circle(pos, 2, SIZE, sf::Color::White, ms_BodyInfo, CEntity::CHARACTER)
{
	m_pPlayer = pPlayer;
	m_SubrectIndex = 0;
	m_Visible = true;
	m_Health = MAX_HEALTH_PLAYER;
	m_Alive = true;
	m_Fire = false;
	m_Jumps = 0;
	m_ActiveWeapon = WEAPON_GRENADE_LAUNCHER;
	m_TimerDamageIndicator = 0;
	m_TimerFire = 0;
	m_FlagDamage = false;
	m_ShaderBlur = 0.0f;
	m_TimerHeartbeat = 0;
	m_TimerAnim = ups::timeFreq();

	setShadowSizeFactor(0.7f);

	//CGame *pGame = CGame::getInstance();

	m_pBody = getBody();
	if (m_pBody)
	{
		m_pBody->SetFixedRotation(true);
		//m_pBody->SetAngularDamping(1.25f);
		//m_pBody->SetLinearDamping(10.25f);

		// Crear Sensor
		b2FixtureDef fixtureDef;
		b2CircleShape circleShape;
		circleShape.m_radius = SIZE*15.0f*MPP;
		fixtureDef.shape = &circleShape;
		fixtureDef.isSensor = true;
		fixtureDef.filter.categoryBits = CAT_BOT_SENSOR;
		fixtureDef.filter.maskBits = CAT_CHARACTER_PLAYER;
		m_pBody->CreateFixture(&fixtureDef);
	}

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
	CSystemFx *pFxEngine = pGame->Client()->getSystem<CSystemFx>();

	if (isAlive())
	{
		const sf::Vector2f &shapePos = getShape()->getPosition();

		// Check Ground
		CSystemBox2D *pB2Engine = pGame->Client()->getSystem<CSystemBox2D>();
		const sf::Vector2f groundColPos = shapePos + sf::Vector2f(0.0f, CCharacter::SIZE+5.0f);
		const bool isGrounded = (pB2Engine->checkIntersectLine(shapePos, groundColPos, 0x0, this) != 0x0);
		if (isGrounded)
		{
			m_pBody->SetLinearDamping((m_State == MOVE_STATE_STOP)?8.0f:3.5f);
			m_Jumps = 0;
		} else {
			m_pBody->SetLinearDamping(0.0f);
			m_Jumps |= 1;
		}

		if (m_Health <= 0)
		{
			pFxEngine->createBlood(shapePos);
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

		const float velLen = upm::vectorLength(sf::Vector2f(m_pBody->GetLinearVelocity().x,m_pBody->GetLinearVelocity().y))*0.1f;
		if ((m_Fire || velLen > 0.03f) && ups::timeGet()-m_TimerAnim > ups::timeFreq()*(0.25f-velLen))
		{
			++m_SubrectIndex;
			if (m_SubrectIndex>=ANIM_SUBRECTS)
				m_SubrectIndex = 0;
			m_TimerAnim = ups::timeGet();
		}
		else if (velLen == 0.0f)
			m_SubrectIndex = 0;

		getShape()->setTextureRect(sf::IntRect(m_SubrectIndex*32, 0, 32, 32));

		m_ShaderBlur = upm::max(m_ShaderBlur-0.001f*pGame->Client()->getDeltaTime(), 0.0f);
	}

	m_Fire = false;
}

void CCharacter::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!m_Visible)
		return;

	CGame *pGame = CGame::getInstance();

	/*sf::Shader *pShader = pGame->Client()->Assets().getShader(CAssetManager::SHADER_BLUR);
	pShader->setUniform("texture", sf::Shader::CurrentTexture);
	pShader->setUniform("blur_radius", 0.04f);
	states.shader = pShader;*/
	CB2Circle::draw(target, states);
	//states.shader = nullptr;
	sf::Vector2f point = m_pShape->getPosition()+sf::Vector2f(0.0f, -1.0f)*(CCharacter::SIZE);
	upm::vectorRotate(m_pShape->getPosition(), &point, upm::degToRad(m_pShape->getRotation()));
	// Direction
	const sf::Vertex lineDir[] =
	{
		sf::Vertex(m_pShape->getPosition(), sf::Color::Black),
		sf::Vertex(point, sf::Color::Black)
	};
	target.draw(lineDir, 2, sf::Lines, states);

	if (pGame->Client()->m_Debug)
	{
		sf::Vector2f point = m_pShape->getPosition()+sf::Vector2f(0.0f, -1.0f)*(CCharacter::SIZE+g_Config.m_CharacterHitDistance);
		upm::vectorRotate(m_pShape->getPosition(), &point, upm::degToRad(m_pShape->getRotation()));
		// Direction
		const sf::Vertex lineDir[] =
		{
			sf::Vertex(m_pShape->getPosition(), sf::Color::Yellow),
			sf::Vertex(point, sf::Color::Yellow)
		};
		target.draw(lineDir, 2, sf::Lines, states);
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

		const sf::Vector2f &shapePos = getShape()->getPosition();
		const sf::Vector2f shapeDir = upm::degToDir(getShape()->getRotation()-90.0f);

		switch (m_ActiveWeapon)
		{
			case WEAPON_GRENADE_LAUNCHER:
			{
				new CProjectile(shapePos+shapeDir*(CCharacter::SIZE+25.0f), sf::Vector2f(32.0f,32.0f), shapeDir, g_Config.m_aWeaponsInfo[m_ActiveWeapon].m_Speed, getOwner(), m_ActiveWeapon, 0);
			} break;
		}

		m_TimerFire = ups::timeGet();
	}

	m_Fire = true;
}

void CCharacter::setVisible(bool visible) noexcept
{
	m_Visible = visible;
	getBody()->SetActive(m_Visible);
}

void CCharacter::takeHealth(int amount, class CPlayer *pPlayer) noexcept
{
	m_Health = upm::max(m_Health-amount, 0);
	getShape()->setFillColor(sf::Color::Red);
	CGame *pGame = CGame::getInstance();
	m_TimerDamageIndicator = ups::timeGet();
	pGame->Client()->getSystem<CSystemFx>()->createPoints(getShape()->getPosition(), -amount);
	pGame->Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_DAMAGE, getShape()->getPosition(), 15.0f);
	if (m_Health == 1)
		m_TimerHeartbeat = ups::timeGet();
}


void CCharacter::doImpulse(sf::Vector2f dir, float energy) noexcept
{
	if (!m_pBody)
		return;

	m_pBody->ApplyForceToCenter(CSystemBox2D::sfToB2(dir*energy), true);
}

void CCharacter::move(int state, bool turbo) noexcept
{
	m_State = state;
	if (state == MOVE_STATE_STOP)
		return;

	const sf::Vector2f charVel = CSystemBox2D::b2ToSf(m_pBody->GetLinearVelocity());
    const float v = g_Config.m_CharacterImpulse;
    sf::Vector2f force = {
    	(state & MOVE_STATE_RIGHT ? v : ( MOVE_STATE_LEFT & state ? -v : 0)),
		(state & MOVE_STATE_DOWN ? v : ( MOVE_STATE_UP & state ? -g_Config.m_CharacterJumpImpulse : 0))
    };
    bool canMove = true;

    // Only can jump one time
    if (m_State&MOVE_STATE_UP)
    {
    	if (m_Jumps == 3)
    	{
    		force.y = 0.0f;
    	}
    	m_Jumps |= 2;
    }

    // Can't impulse in air
    if ((m_State&MOVE_STATE_LEFT) || (m_State&MOVE_STATE_RIGHT))
    {
		CGame *pGame = CGame::getInstance();
		CSystemBox2D *pB2Engine = pGame->Client()->getSystem<CSystemBox2D>();
		const sf::Vector2f &shapePos = getShape()->getPosition();

		const sf::Vector2f groundColPos = shapePos + sf::Vector2f(0.0f, CCharacter::SIZE+5.0f);
		const bool isGrounded = (pB2Engine->checkIntersectLine(shapePos, groundColPos, 0x0, this) != 0x0);
		if (!isGrounded)
			force.x = (m_State&MOVE_STATE_LEFT)?-v:v;

		const sf::Vector2f endPos = shapePos + sf::Vector2f((CCharacter::SIZE+5.0f)*(m_State&MOVE_STATE_LEFT?-1.0f:1.0f), 0.0f);
		if (pB2Engine->checkIntersectLine(shapePos, endPos, 0x0, this) != 0x0)
			force.x = 0.0f;

		const sf::Vector2f dir = upm::vectorNormalize(charVel);
		if ((isGrounded && abs(charVel.x) >= g_Config.m_CharacterMaxVelocity) || (!isGrounded && abs(charVel.x) >= g_Config.m_CharacterMaxVelocity/2.0f))
		{
			if (((m_State&MOVE_STATE_LEFT) && dir.x < 0) || ((m_State&MOVE_STATE_RIGHT) && dir.x > 0))
				force.x = 0.0f;
		}
    }

    if (canMove)
    	m_pBody->ApplyLinearImpulseToCenter(CSystemBox2D::sfToB2(force), true);
    	//m_pBody->ApplyForceToCenter(CSystemBox2D::sfToB2(force), true);

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
