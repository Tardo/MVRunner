/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <game/CPlayer.hpp>
#include "CPlayerRender.hpp"

CPlayerRender::CPlayerRender() noexcept
: CComponent()
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

	renderPlayer(target, states, Client()->Controller()->Context()->getPlayer()->getCharacter());
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
		bodyInColor = sf::Color::Black;
		bodyOutColor = sf::Color::White;
	}

	const sf::Vector2f CharPos = CSystemBox2D::b2ToSf(pChar->getBody()->GetPosition());
	const float CharRot = pChar->getBody()->GetAngle();
	sf::CircleShape Shape(CCharacter::SIZE, 10); // Low Definition
	Shape.setFillColor(bodyInColor);
	Shape.setOutlineColor(bodyOutColor);
	Shape.setOrigin(CCharacter::SIZE, CCharacter::SIZE);
	Shape.setOutlineThickness(3.0f);
	Shape.setPosition(CharPos);
	Shape.setRotation(upm::radToDeg(CharRot));
	target.draw(Shape, states);

	if (pChar->getCharacterState()&CCharacter::STATE_FREEZED)
	{
		sf::CircleShape ShapeTimerIndicator = Shape;
		ShapeTimerIndicator.setFillColor(sf::Color::Red);
		ShapeTimerIndicator.setOutlineColor(sf::Color::White);
		const float crad = ShapeTimerIndicator.getRadius();
		const float ucrad = 1.0f - ((((ups::timeGet() - pChar->m_TimerCharacterState) / (float)ups::timeFreq()) * crad) / g_Config.m_TimeFreeze) / crad;
		ShapeTimerIndicator.setScale(ucrad, ucrad);
		target.draw(ShapeTimerIndicator);
	}

	sf::Vector2f point = CharPos+sf::Vector2f(0.0f, -1.0f)*(CCharacter::SIZE);
	upm::vectorRotate(CharPos, &point, CharRot);
	// Direction
	sf::Color lineColor = pChar->getBody()->IsFixedRotation()?sf::Color::Black:sf::Color::White;
	const sf::Vertex lineDir[] =
	{
		sf::Vertex(CharPos, lineColor),
		sf::Vertex(point, lineColor)
	};
	target.draw(lineDir, 2, sf::Lines, states);

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
