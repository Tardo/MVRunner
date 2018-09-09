/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <game/CPlayer.hpp>
#include "CPlayerRender.hpp"

CPlayerRender::CPlayerRender(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{
	m_pSpriteHookBody = new sf::Sprite(*Client()->Assets().getTexture(CAssetManager::TEXTURE_HOOK_DEFAULT_BODY));
}
CPlayerRender::~CPlayerRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CPlayerRender", "Deleted");
	#endif

	if (m_pSpriteHookBody)
		delete m_pSpriteHookBody;
}

void CPlayerRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!Client()->Controller() || !Client()->Controller()->Context() || !Client()->Controller()->Context()->getPlayer() || !Client()->Controller()->Context()->getPlayer()->getCharacter())
		return;

	target.setView(Client()->Camera());

	if (Client()->getRenderMode() == RENDER_MODE_NORMAL)
	{
		renderHook(target, states, Client()->Controller()->Context()->getPlayer()->getCharacter());
		renderPlayer(target, states, Client()->Controller()->Context()->getPlayer()->getCharacter());
	}
	else if (Client()->getRenderMode() == RENDER_MODE_LIGHTING)
		renderPlayerLights(target, states, Client()->Controller()->Context()->getPlayer()->getCharacter());
}

void CPlayerRender::renderHook(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept
{
	if (!pChar->isVisible() || pChar->m_HookState == CCharacter::HOOK_STATE_RETRACTED || pChar->m_HookLength == 0.0f)
		return;

	const sf::Vector2f charPos = CSystemBox2D::b2ToSf(pChar->getBody()->GetPosition());
	const float hookLength = upm::vectorLength(pChar->m_HookPos - charPos);

	m_pSpriteHookBody->setTextureRect({0, 0, (int)hookLength, 16});
	m_pSpriteHookBody->setOrigin(0.0f, 8.0f);
	m_pSpriteHookBody->setPosition(charPos);
	m_pSpriteHookBody->setRotation(upm::vectorAngle(pChar->m_HookDir));
	target.draw(*m_pSpriteHookBody, states);

	sf::RectangleShape hookHead(sf::Vector2f(16.0f, 16.0f));
	hookHead.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_HOOK_DEFAULT_HEAD));
	hookHead.setOrigin(0.0f, 8.0f);
	hookHead.setPosition(charPos + pChar->m_HookDir * hookLength);
	hookHead.setRotation(upm::vectorAngle(pChar->m_HookDir));
	target.draw(hookHead, states);
}
void CPlayerRender::renderPlayer(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept
{
	if (!pChar->isVisible())
		return;

	const sf::Vector2f CharPos = CSystemBox2D::b2ToSf(pChar->getBody()->GetPosition());
	const float CharRot = pChar->getBody()->GetAngle();
	const sf::Vector2f CharDir = upm::vectorNormalize(Client()->mapPixelToCoords(Client()->UI().getMousePos(), Client()->Camera()) - CharPos);

	renderPlayerBody(target, states, pChar, CharPos, CharRot);
	renderWeapon(target, states, pChar, CharPos, CharDir);

	if (Client()->m_Debug)
	{
		sf::Vector2f point = CharPos+sf::Vector2f(0.0f, -1.0f)*(CCharacter::SIZE+g_Config.m_CharacterHitDistance);
		upm::vectorRotate(CharPos, &point, CharRot);
		// Direction
		const sf::Vertex lineDir[] =
		{
			sf::Vertex(CharPos, sf::Color::Yellow),
			sf::Vertex(point, sf::Color::Yellow)
		};
		target.draw(lineDir, 2, sf::Lines, states);
	}
}

void CPlayerRender::renderPlayerBody(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar, const sf::Vector2f &charPos, float charRot) const noexcept
{
	sf::Color bodyInColor = sf::Color::White;
	sf::Color bodyOutColor = sf::Color::Black;
	if (pChar->getCharacterState()&CCharacter::STATE_ROTATE)
	{
		bodyInColor = sf::Color::Blue;
		bodyOutColor = sf::Color::White;
	}

	static const float playerRadius = CCharacter::SIZE/2.0f;
	sf::CircleShape ShapeBody(playerRadius, 10); // Low Definition
	ShapeBody.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
	ShapeBody.setTextureRect(sf::IntRect(0, 0, 32, 32));
	ShapeBody.setFillColor(bodyInColor);
	ShapeBody.setOutlineColor(bodyOutColor);
	ShapeBody.setOrigin(playerRadius, playerRadius);
	ShapeBody.setOutlineThickness(3.0f);
	ShapeBody.setPosition(charPos);
	ShapeBody.setScale(2.0f, 2.0f);
	ShapeBody.setRotation(upm::radToDeg(charRot));
	target.draw(ShapeBody, states);

	if (pChar->getCharacterState()&CCharacter::STATE_FREEZED)
	{
		sf::CircleShape ShapeTimerIndicator = ShapeBody;
		ShapeTimerIndicator.setFillColor(sf::Color::Red);
		ShapeTimerIndicator.setOutlineColor(sf::Color::White);
		const float crad = ShapeTimerIndicator.getRadius();
		const float ucrad = 1.0f - ((((ups::timeGet() - pChar->m_TimerCharacterState) / (float)ups::timeFreq()) * crad) / g_Config.m_TimeFreeze) / crad;
		ShapeTimerIndicator.setScale(ucrad, ucrad);
		target.draw(ShapeTimerIndicator);
	}
}

void CPlayerRender::renderWeapon(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar, const sf::Vector2f &charPos, const sf::Vector2f &charDir) const noexcept
{
	// Weapon Movement
	static sf::Int64 TimerShoot = 0;
	static bool LastFire = false;
	static float OffsetWeapon = 0.0f;
	static int State = 0;
	const bool Fire = Client()->Controls().isMousePressed("fire");
	const int ActiveWeapon = pChar->getActiveWeapon();
	if ((!LastFire || (LastFire && ups::timeGet() - TimerShoot >= ups::timeFreq()*g_Config.m_aWeaponsInfo[ActiveWeapon].m_FireDelay)) && Fire)
	{
		OffsetWeapon = 12.0f;
		TimerShoot = ups::timeGet();
		State = 0;
	}

	if (!Fire || (State == 0 && ups::timeGet() - TimerShoot >= ups::timeFreq()*0.05f))
	{
		OffsetWeapon = 0.0f;
		TimerShoot = ups::timeGet();
		State = 1;
	}
	LastFire = Fire;

	// Render Weapon
	if (!(pChar->getCharacterState()&CCharacter::STATE_ROTATE))
	{
		const sf::Vector2f weaponPos = charPos-charDir*OffsetWeapon;
		if (g_Config.m_AimLineLength > 0.0f)
		{
			sf::VertexArray Line(sf::LinesStrip, 2);
			Line[0].position = charPos;
			Line[0].color = g_Config.m_AimLineColor;
			Line[1].position = weaponPos + charDir*g_Config.m_AimLineLength;
			Line[1].color = sf::Color::Transparent;
			target.draw(Line, states);
		}

		sf::RectangleShape ShapeWeapon(sf::Vector2f(28.0f, 28.0f));
		ShapeWeapon.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
		// FIXME: Change texture distribution to not use conditionals
		if (pChar->getActiveWeapon() == WEAPON_GRENADE_LAUNCHER || pChar->getActiveWeapon() == WEAPON_JET_PACK)
			ShapeWeapon.setTextureRect(sf::IntRect(32, 0, 28, 28));
		else if (pChar->getActiveWeapon() == WEAPON_VISCOSITY_LAUNCHER)
			ShapeWeapon.setTextureRect(sf::IntRect(69, 0, 28, 28));
		ShapeWeapon.setOrigin(28.0f/2.0f, 28.0f/2.0f);
		ShapeWeapon.setPosition(weaponPos);
		ShapeWeapon.setScale(2.0f, 2.0f);
		ShapeWeapon.setRotation(upm::vectorAngle(charDir));
		target.draw(ShapeWeapon, states);
	}
}

void CPlayerRender::renderPlayerLights(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept
{
	if (!pChar->isVisible())
		return;

	const sf::Vector2f CharPos = CSystemBox2D::b2ToSf(pChar->getBody()->GetPosition());
	const float CharRot = pChar->getBody()->GetAngle();

	// Render Body Light
	const float CharAngVel = abs(pChar->getBody()->GetAngularVelocity());
	const float MaxAngVel = 30;
	sf::Color Rgb;
	if (CharAngVel > MaxAngVel)
		Rgb = ups::hslToRgb(sf::Vector3f(1.0f, 1.0f, 0.5f));
	else
		Rgb = ups::hslToRgb(sf::Vector3f(((CharAngVel * 0.35f) / MaxAngVel), 1.0f, 0.5f));
	static const float playerRadius = CCharacter::SIZE/2.0f;
	sf::CircleShape ShapeBodyLight(playerRadius, 10); // Low Definition
	ShapeBodyLight.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
	ShapeBodyLight.setTextureRect(sf::IntRect(0, 32, 32, 32));
	ShapeBodyLight.setFillColor(Rgb);
	ShapeBodyLight.setOrigin(playerRadius, playerRadius);
	ShapeBodyLight.setPosition(CharPos);
	ShapeBodyLight.setScale(2.0f, 2.0f);
	ShapeBodyLight.setRotation(upm::radToDeg(CharRot));
	target.draw(ShapeBodyLight, states);

	// Weapon Movement
	static sf::Int64 TimerShoot = 0;
	static bool LastFire = false;
	static float OffsetWeapon = 0.0f;
	static int State = 0;
	const bool Fire = Client()->Controls().isMousePressed("fire");
	const int ActiveWeapon = pChar->getActiveWeapon();
	if ((!LastFire || (LastFire && ups::timeGet() - TimerShoot >= ups::timeFreq()*g_Config.m_aWeaponsInfo[ActiveWeapon].m_FireDelay)) && Fire)
	{
		OffsetWeapon = 12.0f;
		TimerShoot = ups::timeGet();
		State = 0;
	}

	if (!Fire || (State == 0 && ups::timeGet() - TimerShoot >= ups::timeFreq()*0.05f))
	{
		OffsetWeapon = 0.0f;
		TimerShoot = ups::timeGet();
		State = 1;
	}
	LastFire = Fire;

	// Render Weapon
	if (!(pChar->getCharacterState()&CCharacter::STATE_ROTATE))
	{
		const sf::Vector2f CharDir = upm::vectorNormalize(Client()->mapPixelToCoords(Client()->UI().getMousePos(), Client()->Camera()) - CharPos);
		const sf::Vector2f weaponPos = CharPos-CharDir*OffsetWeapon;
		sf::RectangleShape ShapeWeapon(sf::Vector2f(28.0f, 28.0f));
		ShapeWeapon.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
		// FIXME: Change texture distribution to not use conditionals
		if (pChar->getActiveWeapon() == WEAPON_GRENADE_LAUNCHER || pChar->getActiveWeapon() == WEAPON_JET_PACK)
			ShapeWeapon.setTextureRect(sf::IntRect(32, 0, 28, 28));
		else if (pChar->getActiveWeapon() == WEAPON_VISCOSITY_LAUNCHER)
			ShapeWeapon.setTextureRect(sf::IntRect(69, 0, 28, 28));
		ShapeWeapon.setOrigin(28.0f/2.0f, 28.0f/2.0f);
		ShapeWeapon.setPosition(weaponPos);
		ShapeWeapon.setScale(2.0f, 2.0f);
		ShapeWeapon.setRotation(upm::vectorAngle(CharDir));
		ShapeWeapon.setFillColor(sf::Color::Black);
		target.draw(ShapeWeapon, states);
	}
}
