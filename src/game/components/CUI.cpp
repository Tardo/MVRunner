/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include "CUI.hpp"

CUI::CUI() noexcept
: CComponent()
{
	m_MouseLeftClicked = false;
}
CUI::~CUI() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CUI", "Deleted");
	#endif
}

void CUI::update(float deltaTime) noexcept
{
	m_MouseLeftClicked = false;
	static bool s_MouseLeftPressed = false;
	if (!s_MouseLeftPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
		s_MouseLeftPressed = true;
	else if (s_MouseLeftPressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		s_MouseLeftPressed = false;
		m_MouseLeftClicked = true;
	}
}

bool CUI::doButton(const char* pText, const sf::FloatRect &bounds, unsigned int fontSize, int align) noexcept
{
	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());
	text.setCharacterSize(fontSize);
	text.setString(pText);
	const float textW = text.getLocalBounds().width;
	if (align == ALIGN_RIGHT)
		text.setPosition(bounds.left+bounds.width-textW, bounds.top+bounds.height/2.0f-fontSize);
	else if (align == ALIGN_CENTER)
		text.setPosition(bounds.left+bounds.width/2.0f-textW/2.0f, bounds.top+bounds.height/2.0f-fontSize);
	else
		text.setPosition(bounds.left, bounds.top+bounds.height/2.0f-fontSize);
	text.setFillColor((text.getGlobalBounds().contains((sf::Vector2f)Client()->m_MousePosition))?sf::Color::Red:sf::Color::White);
	Client()->draw(text);

	return (m_MouseLeftClicked && text.getGlobalBounds().contains((sf::Vector2f)Client()->m_MousePosition));
}
