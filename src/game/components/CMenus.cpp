/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/client/CClient.hpp>
#include <engine/CLocalization.hpp>
#include <engine/client/CSystemSound.hpp>
#include <game/version.h>
#include "CMenus.hpp"

CMenus::CMenus(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{
	m_ActiveMenu = m_ActiveModal = NONE;
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

void CMenus::setActiveModal(int mid) noexcept
{
	if (mid < NONE || mid > NUM_MODALS)
		return;

	m_ActiveModal = mid;
}

void CMenus::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (Client()->getRenderMode() != RENDER_MODE_NORMAL)
		return;

	target.setView(Client()->getHudView());

	static bool inMenu = false;
	//if (!inMenu && Client()->getElapsedTime() > 4.0f)
	if (!inMenu)
	{
		Client()->Menus().setActive(CMenus::MAIN);
		inMenu = true;
	}
	//else if (!inMenu)
	//	renderMenuIntro(target, states);


	if (m_ActiveMenu == MAIN)
		renderMenuMain(target, states);
	else if (m_ActiveMenu == CREDITS)
		renderMenuCredits(target, states);
	else if (m_ActiveMenu == CONTROLS)
		renderMenuControls(target, states);

	if (m_ActiveModal == MODAL_KEY_BIND)
		renderModalKeyBind(target, states);
}

void CMenus::renderModalKeyBind(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());

	const float w = 300.0f;
	const float h = 42.0f;
	const sf::FloatRect bounds(rectArea.width/2.0f - w/2.0f, rectArea.height/2.0f - h/2.0f, w, h);
	Client()->UI().doBox(target, states, bounds, sf::Color(36, 36, 36), 1.0f, sf::Color::Cyan);
	Client()->UI().doLabel(target, states, "PRESS ANY KEY...", bounds, sf::Color::Black, CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_CENTER);
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

	text.setCharacterSize(CUI::TEXT_SIZE_HEADER);
	text.setString(_("CREDITS"));
	textW = text.getLocalBounds().width;
	text.setPosition(rectArea.width/2.0f-textW/2.0f, -50.0f);
	text.setFillColor(sf::Color::Cyan);
	target.draw(text, states);

	for (int i=0; i<1; i+=2)
	{
		text.setFillColor(sf::Color::Red);
		text.setCharacterSize(CUI::TEXT_SIZE_NORMAL);
		text.setString(aMessages[i]);
		textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-textW/2.0f, 150.0f+36.0f*i);
		target.draw(text, states);
		text.setFillColor(sf::Color::White);
		text.setCharacterSize(CUI::TEXT_SIZE_NORMAL);
		text.setString(aMessages[i+1]);
		textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-textW/2.0f, 150.0f+36.0f*i+20.0f);
		target.draw(text, states);
	}

	static int buttonId;
	if (Client()->UI().doButton(target, states, &buttonId, _("BACK"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-100.0f, 80.0f, 35.0f), CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_RIGHT))
	{
		Client()->Menus().setActive(MAIN);
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
	}
}

void CMenus::renderMenuControls(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());
	float textW = 0.0f;

	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());

	text.setCharacterSize(CUI::TEXT_SIZE_HEADER);
	text.setString(_("CONTROLS"));
	textW = text.getLocalBounds().width;
	text.setPosition(rectArea.width/2.0f-textW/2.0f, -50.0f);
	text.setFillColor(sf::Color::Cyan);
	target.draw(text, states);

	std::map<std::string, int> &binds = Client()->Controls().getCmdBinds();
	std::map<std::string, int>::const_iterator cit = binds.begin();
	int i = 0;
	static int buttonId;
	while (cit != binds.end())
	{
		text.setFillColor(sf::Color::Red);
		text.setCharacterSize(46);
		text.setString((*cit).first);
		textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-90.0f, 130.0f+50.0f*i);
		target.draw(text, states);

		if (Client()->UI().doButton(target, states, &buttonId+i, CControls::getKeyName((*cit).second), sf::FloatRect(rectArea.width/2.0f+30.0f, 150.0f+50.0f*i, 110.0f, 35.0f), CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_CENTER))
		{
			Client()->Menus().setActiveModal(MODAL_KEY_BIND);
			Client()->Controls().listenKeyBind((*cit).first.c_str());
		}
		++cit; ++i;
	}

	static int buttonIdB;
	if (Client()->UI().doButton(target, states, &buttonIdB, _("BACK"), sf::FloatRect(rectArea.width-100.0f, rectArea.height-100.0f, 80.0f, 35.0f), CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_RIGHT))
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

	text.setCharacterSize(CUI::TEXT_SIZE_SMALL);
	char title[128];
	snprintf(title, sizeof(title), "v%s", GAME_RELEASE_VERSION);
	text.setString(title);
	float textH = text.getLocalBounds().height;
	text.setPosition(10.0f, rectArea.height-textH-30.0f);
	text.setFillColor(sf::Color::White);
	target.draw(text, states);

	static int buttonIdA;
	if (Client()->UI().doButton(target, states, &buttonIdA, _("START GAME"), sf::FloatRect(rectArea.width-270.0f, rectArea.height-220.0f, 250.0f, 35.0f), CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_CENTER))
	{
		Client()->initializeGameMode(GAMETYPE_MAIN);
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
	}

	static int buttonIdB;
	if (Client()->UI().doButton(target, states, &buttonIdB, _("CONTROLS"), sf::FloatRect(rectArea.width-270.0f, rectArea.height-160.0f, 250.0f, 35.0f), CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_CENTER))
	{
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
		Client()->Menus().setActive(CONTROLS);
	}

	static int buttonIdC;
	if (Client()->UI().doButton(target, states, &buttonIdC, _("CREDITS"), sf::FloatRect(rectArea.width-270.0f, rectArea.height-120.0f, 250.0f, 35.0f), CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_CENTER))
	{
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
		Client()->Menus().setActive(CREDITS);
	}

	static int buttonIdD;
	if (Client()->UI().doButton(target, states, &buttonIdD, _("EXIT"), sf::FloatRect(rectArea.width-270.0f, rectArea.height-60.0f, 250.0f, 35.0f), CUI::TEXT_SIZE_NORMAL, CUI::ALIGN_CENTER))
	{
		Client()->getSystem<CSystemSound>()->play(CAssetManager::SOUND_MOUSE_CLICK);
		Client()->close();
	}
}

void CMenus::renderMenuIntro(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::Shader *pShader = Client()->Assets().getShader(CAssetManager::SHADER_FLAG);
	if (pShader)
	{
		pShader->setUniform("wave_phase", Client()->getElapsedTime());
		pShader->setUniform("wave_amplitude", sf::Vector2f(8.0f, 8.0f));
		pShader->setUniform("texture", sf::Shader::CurrentTexture);
	}

	sf::Text text;
	text.setFont(Client()->Assets().getDefaultFont());
	text.setString(_("Powered By"));
	text.setCharacterSize(92.0f);
	text.setFillColor(sf::Color(80, 80, 80));
	text.setPosition(g_Config.m_ScreenWidth/2.0f - text.getLocalBounds().width/2.0f, 180.0f);
	target.draw(text);

	sf::Sprite SpriteLogo;
	SpriteLogo.setTexture(*Client()->Assets().getTexture(CAssetManager::TEXTURE_SFML_LOGO));
	SpriteLogo.setScale(0.75f, 0.75f);
	SpriteLogo.setPosition(sf::Vector2f(g_Config.m_ScreenWidth/2.0f - SpriteLogo.getLocalBounds().width/2.0f*SpriteLogo.getScale().x, g_Config.m_ScreenHeight/2.0f - SpriteLogo.getLocalBounds().height/2.0f*SpriteLogo.getScale().y));
	states.shader = pShader;
	target.draw(SpriteLogo, states);
}
