/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <cstring>
#include <base/math.hpp>
#include <base/system.hpp>
#include <engine/client/CConfig.hpp>
#include "CUI.hpp"

const sf::Vector2f CUI::SCROLL_SIZE(8.0f, 30.0f);

sf::FloatRect CUI::s_ListViewBounds;
int CUI::s_ListViewNumRows;
int CUI::s_ListViewCurrentRow;
int *CUI::s_pListViewSelectedRow;
float *CUI::s_pListViewScrollOffset;
float CUI::s_ListViewHeightPerRow;
sf::Vector2f CUI::s_ListViewContentSize;

CUI::CUI(const sf::Vector2u &screenSize, const sf::Font &font) noexcept
: m_Font(font)
{
	m_ScreenSize = screenSize;

	m_pHotControl = nullptr;
	m_pDrawControl = nullptr;
	m_HotControlType = CONTROL_NONE;
	m_pEditBuffer = nullptr;
	m_EditMaxLength = 0;
	m_pHotControlOffsetY = nullptr;
	m_pHotControlSelectedItem = nullptr;

	m_MouseLeftClicked = false;
	m_MouseRightClicked = false;
	m_MousePosition = sf::Vector2i(0, 0);
}
CUI::~CUI() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CUI", "Deleted");
	#endif
}

void CUI::processEvent(const sf::Event &event) noexcept
{
	if (event.type == sf::Event::MouseMoved)
	{
		m_MousePosition = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
	}
	else if (event.type == sf::Event::MouseButtonPressed)
	{
		if (event.mouseButton.button == sf::Mouse::Button::Left)
			m_MouseLeftClicked = true;
		else if (event.mouseButton.button == sf::Mouse::Button::Right)
			m_MouseRightClicked = true;
	}
	else if (event.type == sf::Event::MouseButtonReleased)
	{
		if (event.mouseButton.button == sf::Mouse::Button::Left)
			m_MouseLeftClicked = false;
		else if (event.mouseButton.button == sf::Mouse::Button::Right)
			m_MouseRightClicked = false;
	}

	if (m_HotControlType == CONTROL_EDIT)
	{
		const int textLength = ups::strLen(m_pEditBuffer);
		if (event.type == sf::Event::TextEntered && m_pEditBuffer && event.text.unicode < 128 && event.text.unicode != 8 && textLength < m_EditMaxLength)
		{
			 const char ascii = static_cast<char>(event.text.unicode);
			 ups::strNCat(m_pEditBuffer, &ascii, 1);
		}
		else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace && m_pEditBuffer && m_pEditBuffer[0] != 0)
		{
			m_pEditBuffer[textLength-1] = 0;
		}
	}
	else if (m_HotControlType == CONTROL_LIST)
	{
		if (event.type == sf::Event::MouseWheelScrolled && isMouseInsideArea(&m_HotControlBounds))
		{
			*m_pHotControlOffsetY += event.mouseWheelScroll.delta * -100.0f;
			*m_pHotControlOffsetY = upm::clamp(*m_pHotControlOffsetY, 0.0f, s_ListViewBounds.height-SCROLL_SIZE.y-4.0f);
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Down)
				*m_pHotControlSelectedItem = upm::clamp(*m_pHotControlSelectedItem+1, 0, s_ListViewNumRows-1);
			else if (event.key.code == sf::Keyboard::Up)
				*m_pHotControlSelectedItem = upm::clamp(*m_pHotControlSelectedItem-1, 0, s_ListViewNumRows-1);
		}
	}
}

bool CUI::isMouseInsideArea(sf::FloatRect *pArea) const noexcept
{
	return pArea->contains((sf::Vector2f)getMousePos());
}

bool CUI::isMouseInsideControl(sf::Shape *pControlShape) const noexcept
{
	return pControlShape->getGlobalBounds().contains((sf::Vector2f)getMousePos());
}
bool CUI::isMouseInsideControl(sf::Text *pControlShape) const noexcept
{
	return pControlShape->getGlobalBounds().contains((sf::Vector2f)getMousePos());
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
	text.setFont(m_Font);
	text.setCharacterSize(fontSize);
	text.setString(pText);
	text.setFillColor(color);
	const sf::Vector2f textSize(text.getLocalBounds().width, text.getLocalBounds().height);
	text.setOrigin(textSize.x/2.0f, 0.0f);
	if (align == ALIGN_RIGHT)
		text.setPosition(bounds.left+bounds.width-textSize.x/2.0f, bounds.top+bounds.height/2.0f-textSize.y/2.0f);
	else if (align == ALIGN_CENTER)
		text.setPosition(bounds.left+bounds.width/2.0f, bounds.top+bounds.height/2.0f-textSize.y/2.0f);
	else
		text.setPosition(bounds.left+textSize.x/2.0f, bounds.top+bounds.height/2.0f-textSize.y/2.0f);
	target.draw(text, states);
}

bool CUI::doButtonLogic(const void *pId, const sf::FloatRect &bounds) noexcept
{
	const bool isMouseInside = bounds.contains(static_cast<sf::Vector2f>(getMousePos()));
	if (isMouseLeftClicked() && isMouseInside && pId != m_pHotControl)
	{
		m_pHotControl = pId;
		return true;
	}

	return false;
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
	text.setFont(m_Font);
	text.setCharacterSize(fontSize);
	text.setString(pText);
	const sf::Vector2f textSize(text.getLocalBounds().width, text.getLocalBounds().height);
	text.setOrigin(textSize.x/2.0f, textSize.y/2.0f);
	if (align == ALIGN_RIGHT)
		text.setPosition(bounds.left+bounds.width-textSize.x/2.0f, bounds.top+bounds.height/2.0f-textSize.y/2.0f);
	else if (align == ALIGN_CENTER)
		text.setPosition(bounds.left+bounds.width/2.0f, bounds.top+bounds.height/2.0f-textSize.y/2.0f);
	else
		text.setPosition(bounds.left+textSize.x/2.0f, bounds.top+bounds.height/2.0f-textSize.y/2.0f);
	text.setFillColor(isMouseInside?g_Config.m_ButtonFocusTextColor:g_Config.m_ButtonNormalTextColor);
	target.draw(text, states);

	if (doButtonLogic(pId, bounds))
	{
		m_HotControlType = CONTROL_BUTTON;
		m_HotControlBounds = bounds;
		m_pHotControlOffsetY = nullptr;
		m_pHotControlSelectedItem = nullptr;
		return true;
	}

	return false;
}

void CUI::doListInit(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const sf::FloatRect &bounds, int numItems, int *pSelected, float HeightPerRow, float *pScrollOffset) noexcept
{
	clipEnable(target, static_cast<sf::IntRect>(bounds));
	m_pDrawControl = pId;
	s_ListViewBounds = bounds;
	s_ListViewNumRows = numItems;
	s_ListViewHeightPerRow = HeightPerRow;
	s_ListViewCurrentRow = 0;
	s_pListViewSelectedRow = pSelected;
	s_pListViewScrollOffset = pScrollOffset;
	s_ListViewContentSize = sf::Vector2f(bounds.width, s_ListViewHeightPerRow * (s_ListViewNumRows + 1));
}

CListItem CUI::doNextListItem(const void *pId, const void *pRowId) noexcept
{
	CListItem item;
	item.m_Bounds.top = s_ListViewBounds.top - (*s_pListViewScrollOffset * (s_ListViewContentSize.y-s_ListViewBounds.height)) / (s_ListViewBounds.height-SCROLL_SIZE.y-4.0f) + s_ListViewHeightPerRow * s_ListViewCurrentRow;
	item.m_Bounds.left = s_ListViewBounds.left;
	item.m_Bounds.width = s_ListViewBounds.width - SCROLL_SIZE.x - 2.0f;
	item.m_Bounds.height = s_ListViewHeightPerRow;
	item.m_Visible = s_ListViewBounds.intersects(item.m_Bounds);
	item.m_Selected = false;
	if (isMouseInsideArea(&s_ListViewBounds) && doButtonLogic(pRowId, item.m_Bounds))
	{
		*s_pListViewSelectedRow = s_ListViewCurrentRow;
		item.m_Selected = true;
		m_pHotControl = pId;
		m_HotControlType = CONTROL_LIST;
		m_HotControlBounds = s_ListViewBounds;
		m_pHotControlOffsetY = s_pListViewScrollOffset;
		m_pHotControlSelectedItem = s_pListViewSelectedRow;
	} else if (s_ListViewCurrentRow == *s_pListViewSelectedRow)
	{
		item.m_Selected = true;
	}

	++s_ListViewCurrentRow;
	return item;
}

void CUI::doListEnd(sf::RenderTarget& target, sf::RenderStates states, const void *pId) noexcept
{
	if (s_ListViewContentSize.y > s_ListViewBounds.height)
	{
		sf::FloatRect bounds(s_ListViewBounds.left+s_ListViewBounds.width-SCROLL_SIZE.x-2.0f, s_ListViewBounds.top+(*s_pListViewScrollOffset)+2.0f, SCROLL_SIZE.x, SCROLL_SIZE.y);
		sf::RectangleShape scroll(sf::Vector2f(bounds.width, bounds.height));
		scroll.setPosition(bounds.left, bounds.top);
		scroll.setFillColor(sf::Color::White);
		target.draw(scroll, states);

		const sf::Vector2f mousePos = static_cast<sf::Vector2f>(getMousePos());
		static sf::Vector2f lastMousePos;
		static bool useScroll = false;
		const bool isMouseInside = bounds.contains(mousePos);
		if (isMouseLeftClicked() && isMouseInside)
		{
			if (!useScroll)
				lastMousePos = mousePos;
			useScroll = true;
			m_pHotControl = pId;
			m_HotControlType = CONTROL_LIST;
			m_HotControlBounds = s_ListViewBounds;
			m_pHotControlOffsetY = s_pListViewScrollOffset;
			m_pHotControlSelectedItem = s_pListViewSelectedRow;
		} else if (!isMouseLeftClicked())
		{
			useScroll = false;
		}

		if (useScroll && pId == m_pHotControl)
		{
			*s_pListViewScrollOffset += mousePos.y - lastMousePos.y;
			*s_pListViewScrollOffset = upm::clamp(*s_pListViewScrollOffset, 0.0f, s_ListViewBounds.height-bounds.height-4.0f);

			lastMousePos = mousePos;
		}
	}

	clipDisable();
	doBox(target, states, s_ListViewBounds, sf::Color::Transparent, 1.0f, sf::Color::White);
}

bool CUI::doCheck(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const sf::FloatRect &bounds, bool *pSelected, const char *pText, unsigned int fontSize, const sf::Color &color, int align) noexcept
{
	sf::RectangleShape scroll(sf::Vector2f(bounds.height, bounds.height));
	scroll.setPosition(bounds.left, bounds.top);
	scroll.setFillColor(*pSelected?sf::Color::White:sf::Color::Transparent);
	scroll.setOutlineThickness(1.0f);
	scroll.setOutlineColor(sf::Color::White);
	target.draw(scroll, states);
	doLabel(target, states, pText, sf::FloatRect(bounds.left+bounds.height+5.0f, bounds.top - 5.0f, bounds.width-bounds.height-5.0f, bounds.height), color, fontSize, align);

	if (doButtonLogic(pId, bounds))
	{
		m_pHotControl = pId;
		m_HotControlType = CONTROL_CHECK;
		m_HotControlBounds = bounds;
		m_pHotControlOffsetY = nullptr;
		m_pHotControlSelectedItem = nullptr;
		*pSelected = !(*pSelected);
		return true;
	}

	return false;
}

bool CUI::doEdit(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const sf::FloatRect &bounds, const char *pText, unsigned int fontSize, const sf::Color &color, unsigned int maxLength) noexcept
{
	sf::RectangleShape edit(sf::Vector2f(bounds.width, bounds.height));
	edit.setPosition(bounds.left, bounds.top);
	edit.setFillColor(sf::Color::Transparent);
	edit.setOutlineThickness(1.0f);
	edit.setOutlineColor(sf::Color::White);
	target.draw(edit, states);

	clipEnable(target, static_cast<sf::IntRect>(bounds));
	sf::Text text;
	text.setFont(m_Font);
	text.setCharacterSize(fontSize);
	text.setString(pText);
	text.setFillColor(color);
	const sf::Vector2f textSize(text.getLocalBounds().width, text.getLocalBounds().height);
	const float OffsetX = upm::clamp(textSize.x - bounds.width + 6.0f, 0.0f, textSize.x);
	text.setPosition(bounds.left - OffsetX, bounds.top+bounds.height/2.0f-fontSize/2.0f);
	target.draw(text, states);
	clipDisable();

	static sf::Int64 timerMarkBlink = ups::timeGet();
	static bool showMark = true;
	if (ups::timeGet() - timerMarkBlink > ups::timeFreq() * 0.5f)
	{
		showMark = !showMark;
		timerMarkBlink = ups::timeGet();
	}
	if (pId == m_pHotControl)
	{
		m_pEditBuffer = const_cast<char*>(pText);
		m_EditMaxLength = maxLength;

		if (showMark)
		{
			sf::RectangleShape mark(sf::Vector2f(2.0f, bounds.height - 4.0f));
			mark.setPosition((bounds.left - OffsetX) + text.getLocalBounds().width + 2.0f, bounds.top + 2.0f);
			mark.setFillColor(sf::Color::White);
			target.draw(mark, states);
		}
	}

	if (doButtonLogic(pId, bounds))
	{
		m_pHotControl = pId;
		m_HotControlType = CONTROL_EDIT;
		m_HotControlBounds = bounds;
		m_pHotControlOffsetY = nullptr;
		m_pHotControlSelectedItem = nullptr;
		timerMarkBlink = ups::timeGet();
		showMark = true;
		return true;
	}

	return false;
}


void CUI::clipEnable(const sf::RenderTarget &target, sf::IntRect bounds) const noexcept
{

	if(bounds.left < 0)
		bounds.width += bounds.left;
	if(bounds.top < 0)
		bounds.height += bounds.top;

	bounds.left = upm::clamp(bounds.left, 0, (int)m_ScreenSize.x);
	bounds.top = upm::clamp(bounds.top, 0, (int)m_ScreenSize.y);
	bounds.width = upm::clamp(bounds.width, 0, (int)m_ScreenSize.x-bounds.left);
	bounds.height = upm::clamp(bounds.height, 0, (int)m_ScreenSize.y-bounds.top);

	glScissor(bounds.left, m_ScreenSize.y-(bounds.top+bounds.height), bounds.width, bounds.height);
	glEnable(GL_SCISSOR_TEST);
}

void CUI::clipDisable() const noexcept
{
	glDisable(GL_SCISSOR_TEST);
}
