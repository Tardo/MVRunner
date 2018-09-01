/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CConfig.hpp>
#include <engine/CGame.hpp>
#include "CUI.hpp"

CUI::CUI(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{
	m_TimerBroadcast = ups::timeGet();
	m_BroadcastDuration = 0.0f;
	m_aHelpMsg[0] = 0;
	m_aBroadcastMsg[0] = 0;
	m_pHotControl = 0x0;
}
CUI::~CUI() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CUI", "Deleted");
	#endif
}

void CUI::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	target.setView(Client()->getHudView());

	if (Client()->getRenderMode() == RENDER_MODE_NORMAL)
	{
		drawHUD(target, states);
		if (Client()->m_Debug)
			drawDebugInfo(target, states);
		drawCursor(target, states);
	}
}

bool CUI::isMouseInsideControl(sf::Shape *pControlShape) const noexcept
{
	return pControlShape->getGlobalBounds().contains((sf::Vector2f)Client()->Controls().getMousePos());
}
bool CUI::isMouseInsideControl(sf::Text *pControlShape) const noexcept
{
	return pControlShape->getGlobalBounds().contains((sf::Vector2f)Client()->Controls().getMousePos());
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

void CUI::doBox(sf::RenderTarget& target, sf::RenderStates states, const sf::FloatRect &bounds, const sf::Color &color, float outline, const sf::Color &outlineColor) const noexcept
{
	sf::RectangleShape button(sf::Vector2f(bounds.width, bounds.height));
	button.setPosition(bounds.left, bounds.top);
	button.setFillColor(color);
	button.setOutlineThickness(outline);
	button.setOutlineColor(outlineColor);
	target.draw(button, states);
}

void CUI::doLabel(sf::RenderTarget& target, sf::RenderStates states, const char* pText, const sf::FloatRect &bounds, const sf::Color &color, unsigned int fontSize, int align) const noexcept
{
	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());
	text.setCharacterSize(fontSize);
	text.setString(pText);
	text.setFillColor(sf::Color::Red);
	const sf::Vector2f textSize(text.getLocalBounds().width, text.getLocalBounds().height);
	text.setOrigin(textSize.x/2.0f, 0.0f);
	if (align == ALIGN_RIGHT)
		text.setPosition(bounds.left+bounds.width-textSize.x/2.0f, bounds.top-bounds.height/2.0f-textSize.y/2.0f);
	else if (align == ALIGN_CENTER)
		text.setPosition(bounds.left+bounds.width/2.0f, 0.0f);
	else
		text.setPosition(bounds.left+textSize.x/2.0f, bounds.top-bounds.height/2.0f-textSize.y/2.0f);
	target.draw(text, states);

	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());
	rectArea.left += 10.0f;
	rectArea.top += 10.0f;
	rectArea.width -= 20.0f;
	rectArea.height -= 20.0f;
	doBox(target, states, rectArea, sf::Color::Transparent, 10.0f, sf::Color::Yellow);
	ups::msgDebug("LABEL", "X: %.2f -- Y: %.2f -- W: %.2f -- H: %.2f", rectArea.left, rectArea.top, rectArea.width, rectArea.height);
}

bool CUI::doButton(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const char* pText, const sf::FloatRect &bounds, unsigned int fontSize, int align) noexcept
{
	sf::RectangleShape button(sf::Vector2f(bounds.width, bounds.height));
	button.setPosition(bounds.left, bounds.top);
	const bool isMouseInside = isMouseInsideControl(&button);
	button.setFillColor(isMouseInside?g_Config.m_ButtonFocusFillColor:g_Config.m_ButtonNormalFillColor);
	button.setOutlineThickness(2.0f);
	button.setOutlineColor(sf::Color::Black);
	target.draw(button, states);

	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());
	text.setCharacterSize(fontSize);
	text.setString(pText);
	const sf::Vector2f textSize(text.getLocalBounds().width, text.getLocalBounds().height);
	text.setOrigin(textSize.x/2.0f, textSize.y/2.0f);
	if (align == ALIGN_RIGHT)
		text.setPosition(bounds.left+bounds.width-textSize.x/2.0f, bounds.top+bounds.height/2.0f-textSize.y/2.0f);
	else if (align == ALIGN_CENTER)
		text.setPosition(bounds.left+bounds.width/2.0f, bounds.top-bounds.height/2.0f-textSize.y/2.0f);
	else
		text.setPosition(bounds.left+textSize.x/2.0f, bounds.top-bounds.height/2.0f-textSize.y/2.0f);
	text.setFillColor(isMouseInside?g_Config.m_ButtonFocusTextColor:g_Config.m_ButtonNormalTextColor);
	target.draw(text, states);

	if (Client()->Controls().isMouseLeftClicked() && isMouseInside)
	{
		if (pId != m_pHotControl)
		{
			m_pHotControl = pId;
			return true;
		}
	} else if (!Client()->Controls().isMouseLeftClicked() && m_pHotControl)
	{
		m_pHotControl = 0x0;
	}
	return false;
}

void CUI::drawCursor(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::Vertex line[] =
	{
		sf::Vertex(sf::Vector2f(Client()->Controls().getMousePos().x-10.0f, Client()->Controls().getMousePos().y), sf::Color::White),
		sf::Vertex(sf::Vector2f(Client()->Controls().getMousePos().x+10.0f, Client()->Controls().getMousePos().y), sf::Color::White),
		sf::Vertex(sf::Vector2f(Client()->Controls().getMousePos().x, Client()->Controls().getMousePos().y-10.0f), sf::Color::White),
		sf::Vertex(sf::Vector2f(Client()->Controls().getMousePos().x, Client()->Controls().getMousePos().y+10.0f), sf::Color::White)
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

	CSystemSound *pSystemSound = Client()->getSystem<CSystemSound>();
	CSystemBox2D *pSystemBox2D = Client()->getSystem<CSystemBox2D>();

	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, Client()->getHudView());

	char aBuff[128];
	sf::Text sfStr;
	sfStr.setFont(Client()->Assets().getDefaultFont());
	sfStr.setCharacterSize(62);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Entities: %lu", Client()->Controller()->Context()->getEntities().size());
	#else
	snprintf(aBuff, sizeof(aBuff), "Entities: %u", Client()->Controller()->Context()->getEntities().size());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 30.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	snprintf(aBuff, sizeof(aBuff), "Components: %d", Client()->getNumComponents());
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 60.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	snprintf(aBuff, sizeof(aBuff), "Systems: %d", Client()->getNumSystems());
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 90.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	snprintf(aBuff, sizeof(aBuff), "Sounds: %d", pSystemSound->getNumPlayingSound());
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 120.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	snprintf(aBuff, sizeof(aBuff), "Liquidfun Particles: %d", pSystemBox2D->getParticleSystem(CSystemBox2D::PARTICLE_SYSTEM_WATER)->GetParticleCount());
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 150.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);
}
