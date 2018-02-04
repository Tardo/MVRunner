/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include "CUI.hpp"

CUI::CUI() noexcept
: CComponent()
{
	m_MouseLeftClicked = false;
	m_MousePosition = sf::Vector2i(0, 0);
	m_TimerBroadcast = ups::timeGet();
	m_BroadcastDuration = 0.0f;
	m_aHelpMsg[0] = 0;
	m_aBroadcastMsg[0] = 0;
}
CUI::~CUI() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CUI", "Deleted");
	#endif
}

void CUI::update() noexcept
{
	m_MousePosition = sf::Mouse::getPosition(*Client());
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

void CUI::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	Client()->setView(Client()->getHudView());

	drawHUD(target, states);
	if (Client()->m_Debug)
		drawDebugInfo(target, states);
	drawCursor(target, states);
}

void CUI::showBroadcastMessage(const char *pMsg, float duration) noexcept
{
	m_TimerBroadcast = ups::timeGet();
	m_BroadcastDuration = duration;
	strncpy(m_aBroadcastMsg, pMsg, BROADCAST_MAX_LENGTH);
}

void CUI::showHelpMessage(const char *pMsg) noexcept
{
	strncpy(m_aHelpMsg, pMsg, HELP_TEXT_MAX_LENGTH);
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
	text.setFillColor((text.getGlobalBounds().contains((sf::Vector2f)m_MousePosition))?sf::Color::Red:sf::Color::White);
	Client()->draw(text);

	return (m_MouseLeftClicked && text.getGlobalBounds().contains((sf::Vector2f)m_MousePosition));
}

void CUI::drawCursor(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::Vertex line[] =
	{
		sf::Vertex(sf::Vector2f(m_MousePosition.x-10.0f, m_MousePosition.y), sf::Color::White),
		sf::Vertex(sf::Vector2f(m_MousePosition.x+10.0f, m_MousePosition.y), sf::Color::White),
		sf::Vertex(sf::Vector2f(m_MousePosition.x, m_MousePosition.y-10.0f), sf::Color::White),
		sf::Vertex(sf::Vector2f(m_MousePosition.x, m_MousePosition.y+10.0f), sf::Color::White)
	};

	target.draw(line, 4, sf::Lines);
}

void CUI::drawHUD(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (Client()->Menus().getActive() != CMenus::NONE)
		return;

	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, Client()->getHudView());

	sf::Text sfStr;
	sfStr.setFont(Client()->Assets().getDefaultFont());

	char aBuff[128];
	sfStr.setCharacterSize(62);
	snprintf(aBuff, sizeof(aBuff), "FPS: %u [Min: %u] [%.2fms]", Client()->m_FPS, Client()->m_MinFPS, Client()->getDeltaTime()*1000.0f);
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 0.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	if (ups::timeGet() - m_TimerBroadcast < ups::timeFreq()*m_BroadcastDuration)
	{
		sfStr.setCharacterSize(128);
		sfStr.setString(m_aBroadcastMsg);
		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f+2.0f, 2.0f);
		sfStr.setFillColor(sf::Color::Yellow);
		target.draw(sfStr, states);

		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f, 0.0f);
		sfStr.setFillColor(sf::Color::Red);
		target.draw(sfStr, states);
	}

	if (m_aHelpMsg[0] != 0)
	{
		sfStr.setCharacterSize(92);
		sfStr.setString(m_aHelpMsg);
		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f+2.0f, 78.0f+2.0f);
		sfStr.setFillColor(sf::Color::Black);
		target.draw(sfStr, states);

		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f, 78.0f);
		sfStr.setFillColor(sf::Color::White);
		target.draw(sfStr, states);
	}
}

void CUI::drawDebugInfo(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!Client()->Controller() || !Client()->Controller()->Context())
		return;

	CSystemLight *pSystemLight = Client()->getSystem<CSystemLight>();
	CSystemSound *pSystemSound = Client()->getSystem<CSystemSound>();
	CSystemWeather *pSystemWeather = Client()->getSystem<CSystemWeather>();

	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, Client()->getHudView());

	char aBuff[128];
	sf::Text sfStr;
	sfStr.setFont(Client()->Assets().getDefaultFont());
	sfStr.setCharacterSize(62);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Entidades: %lu", Client()->Controller()->Context()->getEntities().size());
	#else
	snprintf(aBuff, sizeof(aBuff), "Entidades: %u", Client()->Controller()->Context()->getEntities().size());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 30.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Particulas: %lu", Client()->Controller()->Context()->getParticles().size());
	#else
	snprintf(aBuff, sizeof(aBuff), "Particulas: %u", Controller()->Context()->getParticles().size());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 60.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Luces: %lu", pSystemLight->getLights().size());
	#else
	snprintf(aBuff, sizeof(aBuff), "Luces: %u", m_SystemLight.getLights().size());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 90.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Sonidos: %d", pSystemSound->getNumPlayingSound());
	#else
	snprintf(aBuff, sizeof(aBuff), "Sonidos: %d", m_SystemSound.getNumPlayingSound());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 120.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	if (pSystemWeather->getWeather() == CSystemWeather::WEATHER_RAIN)
			snprintf(aBuff, sizeof(aBuff), "Clima: Lluvia");
	else
		snprintf(aBuff, sizeof(aBuff), "Clima: Soleado");
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 150.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);
}
