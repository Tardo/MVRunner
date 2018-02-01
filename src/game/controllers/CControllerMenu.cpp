/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <base/system.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemLight.hpp>
#include <engine/CSystemSound.hpp>
#include <engine/CLocalization.hpp>
#include <game/entities/primitives/CB2Circle.hpp>
#include <game/CContext.hpp>
#include "CControllerMenu.hpp"
#include <cstring>


CControllerMenu::CControllerMenu() noexcept
: CController(),
  m_CamPos(0.0f, 0.0f)
{
	if (Game()->Client()->MapRender().getMap())
	{
		m_CamPos = sf::Vector2f(Game()->Client()->MapRender().getMap()->GetWidth()*Game()->Client()->MapRender().getMap()->GetTileWidth()/2.0f,
								Game()->Client()->MapRender().getMap()->GetHeight()*Game()->Client()->MapRender().getMap()->GetTileHeight()/2.0f);
	}
}
CControllerMenu::CControllerMenu(class CContext *pContext) noexcept
: CController(pContext),
  m_CamPos(0.0f, 0.0f)
{
	if (Game()->Client()->MapRender().getMap())
	{
		m_CamPos = sf::Vector2f(Game()->Client()->MapRender().getMap()->GetWidth()*Game()->Client()->MapRender().getMap()->GetTileWidth()/2.0f,
								Game()->Client()->MapRender().getMap()->GetHeight()*Game()->Client()->MapRender().getMap()->GetTileHeight()/2.0f);
	}
}
CControllerMenu::~CControllerMenu() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CControllerMenu", "Deleted");
	#endif
}

void CControllerMenu::tick() noexcept
{
	static bool inMenu = false;
	if (!inMenu && Game()->Client()->getElapsedTime() > 4.0f)
	{
		Game()->Client()->Menus().setActive(CMenus::MAIN);
		inMenu = true;
	}
	else if (!inMenu)
	{
		sf::Shader *pShader = Game()->Client()->Assets().getShader(CAssetManager::SHADER_FLAG);
		if (pShader)
		{
			pShader->setUniform("wave_phase", Game()->Client()->getElapsedTime());
			pShader->setUniform("wave_amplitude", sf::Vector2f(8.0f, 8.0f));
			pShader->setUniform("texture", sf::Shader::CurrentTexture);
		}

		Game()->Client()->clear(sf::Color::Black);

		sf::Text text;
		text.setFont(Game()->Client()->Assets().getDefaultFont());
		text.setString(_("Powered By"));
		text.setCharacterSize(92.0f);
		text.setFillColor(sf::Color(80, 80, 80));
		text.setPosition(g_Config.m_ScreenWidth/2.0f - text.getLocalBounds().width/2.0f, 180.0f);
		Game()->Client()->draw(text);


		Game()->Client()->draw(m_SpriteLogo, pShader);
	}
	else if (inMenu)
		CController::tick();
}

void CControllerMenu::updateCamera(float deltaTime) noexcept
{
	Game()->Client()->Camera().setCenter(m_CamPos+sf::Vector2f(cosf(Game()->Client()->getElapsedTime()*0.25f)*64.0f, sinf(Game()->Client()->getElapsedTime()*0.25f)*64.0f));
}


void CControllerMenu::onStart() noexcept
{
	m_SpriteLogo.setTexture(*Game()->Client()->Assets().getTexture(CAssetManager::TEXTURE_SFML_LOGO));
	m_SpriteLogo.setScale(0.75f, 0.75f);
	m_SpriteLogo.setPosition(sf::Vector2f(g_Config.m_ScreenWidth/2.0f - m_SpriteLogo.getLocalBounds().width/2.0f*m_SpriteLogo.getScale().x, g_Config.m_ScreenHeight/2.0f - m_SpriteLogo.getLocalBounds().height/2.0f*m_SpriteLogo.getScale().y));

	Game()->Client()->getSystem<CSystemSound>()->playBackgroundMusic(CAssetManager::MUSIC_MAIN_MENU);
}

