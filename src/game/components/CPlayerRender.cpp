/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <game/CPlayer.hpp>
#include "CPlayerRender.hpp"

CPlayerRender::CPlayerRender(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{ }
CPlayerRender::~CPlayerRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CPlayerRender", "Deleted");
	#endif
}

void CPlayerRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!Client()->Controller() || !Client()->Controller()->Context() || !Client()->Controller()->Context()->getPlayer() || !Client()->Controller()->Context()->getPlayer()->getCharacter())
		return;

	target.setView(Client()->Camera());

	if (Client()->getRenderMode() == RENDER_MODE_NORMAL)
		renderPlayer(target, states, Client()->Controller()->Context()->getPlayer()->getCharacter());
	else if (Client()->getRenderMode() == RENDER_MODE_LIGHTING)
		renderPlayerLights(target, states, Client()->Controller()->Context()->getPlayer()->getCharacter());
}

void CPlayerRender::renderPlayer(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept
{
	if (!pChar->isVisible())
		return;

	// TODO: Create a component for this stuff... not use "draw"
	sf::Color bodyInColor = sf::Color::White;
	sf::Color bodyOutColor = sf::Color::Black;
	if (pChar->getCharacterState()&CCharacter::STATE_ROTATE)
	{
		bodyInColor = sf::Color::Blue;
		bodyOutColor = sf::Color::White;
	}

	const sf::Vector2f CharPos = CSystemBox2D::b2ToSf(pChar->getBody()->GetPosition());
	const float CharRot = pChar->getBody()->GetAngle();
	const sf::Vector2f CharDir = upm::vectorNormalize(Client()->mapPixelToCoords(Client()->Controls().getMousePos(), Client()->Camera()) - CharPos);

	// Render Body
	sf::CircleShape ShapeBody(CCharacter::SIZE, 10); // Low Definition
	ShapeBody.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
	ShapeBody.setTextureRect(sf::IntRect(0, 0, 32, 32));
	ShapeBody.setFillColor(bodyInColor);
	ShapeBody.setOutlineColor(bodyOutColor);
	ShapeBody.setOrigin(CCharacter::SIZE, CCharacter::SIZE);
	ShapeBody.setOutlineThickness(3.0f);
	ShapeBody.setPosition(CharPos);
	ShapeBody.setRotation(upm::radToDeg(CharRot));
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
		const sf::Vector2f weaponPos = CharPos-CharDir*OffsetWeapon;
		if (g_Config.m_AimLineLength > 0.0f)
		{
			sf::VertexArray Line(sf::LinesStrip, 2);
			Line[0].position = CharPos;
			Line[0].color = g_Config.m_AimLineColor;
			Line[1].position = weaponPos + CharDir*g_Config.m_AimLineLength;
			Line[1].color = sf::Color::Transparent;
			target.draw(Line, states);
		}

		sf::CircleShape ShapeWeapon(CCharacter::SIZE, 10); // Low Definition
		ShapeWeapon.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
		ShapeWeapon.setTextureRect(sf::IntRect(32, 0, 28, 28));
		ShapeWeapon.setOrigin(CCharacter::SIZE, CCharacter::SIZE);
		ShapeWeapon.setPosition(weaponPos);
		ShapeWeapon.setRotation(upm::vectorAngle(CharDir));
		target.draw(ShapeWeapon, states);
	}

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
	sf::CircleShape ShapeBodyLight(CCharacter::SIZE, 10); // Low Definition
	ShapeBodyLight.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
	ShapeBodyLight.setTextureRect(sf::IntRect(0, 32, 32, 32));
	ShapeBodyLight.setFillColor(Rgb);
	ShapeBodyLight.setOrigin(CCharacter::SIZE, CCharacter::SIZE);
	ShapeBodyLight.setPosition(CharPos);
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
		const sf::Vector2f CharDir = upm::vectorNormalize(Client()->mapPixelToCoords(Client()->Controls().getMousePos(), Client()->Camera()) - CharPos);
		const sf::Vector2f weaponPos = CharPos-CharDir*OffsetWeapon;
		if (g_Config.m_AimLineLength > 0.0f)
		{
			sf::VertexArray Line(sf::LinesStrip, 2);
			Line[0].position = CharPos;
			Line[0].color = g_Config.m_AimLineColor;
			Line[1].position = weaponPos + CharDir*g_Config.m_AimLineLength;
			Line[1].color = sf::Color::Transparent;
			target.draw(Line, states);
		}

		sf::CircleShape ShapeWeapon(CCharacter::SIZE, 10); // Low Definition
		ShapeWeapon.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SKIN_DEFAULT));
		ShapeWeapon.setTextureRect(sf::IntRect(32, 0, 28, 28));
		ShapeWeapon.setOrigin(CCharacter::SIZE, CCharacter::SIZE);
		ShapeWeapon.setPosition(weaponPos);
		ShapeWeapon.setRotation(upm::vectorAngle(CharDir));
		ShapeWeapon.setFillColor(sf::Color::Black);
		target.draw(ShapeWeapon, states);
	}
}
