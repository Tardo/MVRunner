/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CSystemSound.hpp>
#include <engine/CLocalization.hpp>
#include <game/version.h>
#include <SFML/OpenGL.hpp>
#include "CMenus.hpp"

CMenus::CMenus() noexcept
: CComponent()
{
	m_ActiveMenu = NONE;
	m_pEntity = nullptr;
}
CMenus::~CMenus() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CMenus", "Deleted");
	#endif
}

void CMenus::setActive(int mid, CEntity *pEnt) noexcept
{
	if (mid < NONE || mid > NUM_MENUS)
		return;

	m_pEntity = pEnt;
	m_ActiveMenu = mid;
}

void CMenus::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	Client()->setView(Client()->getHudView());

	if (m_ActiveMenu == MAIN)
		renderMenuMain(target, states);
	else if (m_ActiveMenu == CREDITS)
		renderMenuCredits(target, states);
	else if (m_ActiveMenu == CONTROLS)
		renderMenuControls(target, states);
}

void CMenus::renderMenuCredits(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	static const std::string aMessages[2] = {
		"unsigned char*\0", _("Main Coder"),
	};

	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());
	float textW = 0.0f;

	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());

	text.setCharacterSize(142);
	text.setString(_("CREDITS"));
	textW = text.getLocalBounds().width;
	text.setPosition(rectArea.width/2.0f-textW/2.0f, -50.0f);
	text.setFillColor(sf::Color::Cyan);
	target.draw(text, states);

	for (int i=0; i<1; i+=2)
	{
		text.setFillColor(sf::Color::Red);
		text.setCharacterSize(92);
		text.setString(aMessages[i]);
		textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-textW/2.0f, 150.0f+36.0f*i);
		target.draw(text, states);
		text.setFillColor(sf::Color::White);
		text.setCharacterSize(42);
		text.setString(aMessages[i+1]);
		textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-textW/2.0f, 150.0f+36.0f*i+20.0f);
		target.draw(text, states);
	}

	if (Client()->UI().doButton(_("BACK"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-100.0f, 80.0f, 35.0f), 64, CUI::ALIGN_RIGHT))
	{
		Client()->Menus().setActive(MAIN);
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
	}
}

void CMenus::renderMenuControls(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	static const std::string aControls[8] = {
		_("A"), _("Move Left"),
		_("D"), _("Move Right"),
		_("SPACE"), _("Jump"),
		_("E"), _("Use"),
	};

	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());
	float textW = 0.0f;

	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());

	text.setCharacterSize(142);
	text.setString(_("CONTROLS"));
	textW = text.getLocalBounds().width;
	text.setPosition(rectArea.width/2.0f-textW/2.0f, -50.0f);
	text.setFillColor(sf::Color::Cyan);
	target.draw(text, states);

	for (int i=0; i<8; i+=2)
	{
		text.setFillColor(sf::Color::Red);
		text.setCharacterSize(46);
		text.setString(aControls[i]);
		textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-90.0f, 150.0f+25.0f*i);
		target.draw(text, states);
		text.setFillColor(sf::Color::White);
		text.setCharacterSize(46);
		text.setString(aControls[i+1]);
		text.setPosition(rectArea.width/2.0f+30.0f, 150.0f+25.0f*i);
		target.draw(text, states);
	}

	if (Client()->UI().doButton(_("BACK"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-100.0f, 80.0f, 35.0f), 64, CUI::ALIGN_RIGHT))
	{
		Client()->Menus().setActive(MAIN);
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
	}
}

void CMenus::renderMenuMain(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());

	// Background
	sf::RectangleShape bkg(sf::Vector2f(rectArea.width, rectArea.height));
	bkg.setPosition(0.0f, 0.0f);
	bkg.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_POINT_LIGHT));
	states.blendMode = sf::BlendMultiply;
	target.draw(bkg, states);
	states.blendMode = sf::BlendAlpha; // Reset Blend

	// Title Box
	sf::RectangleShape box(sf::Vector2f(rectArea.width-100, 100.0f));
	box.setPosition(50.0f, 10.0f);
	box.setFillColor(sf::Color(0, 0, 0, 80));
	//box.setOutlineThickness(2.0f);
	//box.setOutlineColor(sf::Color(0, 0, 200, 80));
	target.draw(box, states);

	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());

	text.setCharacterSize(32);
	char title[128];
	snprintf(title, sizeof(title), "v%s", GAME_RELEASE_VERSION);
	text.setString(title);
	float textH = text.getLocalBounds().height;
	text.setPosition(10.0f, rectArea.height-textH-30.0f);
	text.setFillColor(sf::Color::White);
	target.draw(text, states);

	if (Client()->UI().doButton(_("Start Game"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-200.0f, 80.0f, 35.0f), 64, CUI::ALIGN_RIGHT))
	{
		Client()->initializeGameMode("main");
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
	}

	if (Client()->UI().doButton(_("Controls"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-140.0f, 80.0f, 35.0f), 64, CUI::ALIGN_RIGHT))
	{
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
		Client()->Menus().setActive(CONTROLS);
	}

	if (Client()->UI().doButton(_("Credits"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-110.0f, 80.0f, 35.0f), 64, CUI::ALIGN_RIGHT))
	{
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
		Client()->Menus().setActive(CREDITS);
	}

	if (Client()->UI().doButton(_("EXIT"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-50.0f, 80.0f, 35.0f), 64, CUI::ALIGN_RIGHT))
	{
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
		Client()->close();
	}
}

void CMenus::ClipEnable(const sf::RenderTarget &target, int x, int y, int w, int h) const noexcept
{
	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());

	if(x < 0)
		w += x;
	if(y < 0)
		h += y;

	x = upm::clamp(x, 0, (int)rectArea.width);
	y = upm::clamp(y, 0, (int)rectArea.height);
	w = upm::clamp(w, 0, (int)rectArea.width-x);
	h = upm::clamp(h, 0, (int)rectArea.height-y);

	glScissor(x, rectArea.height-(y+h), w, h);
	glEnable(GL_SCISSOR_TEST);
}

void CMenus::ClipDisable() const noexcept
{
	glDisable(GL_SCISSOR_TEST);
}
