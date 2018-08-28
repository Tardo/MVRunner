/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <game/controllers/CControllerMenu.hpp>
#include "CControls.hpp"

CControls::CControls(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{
	m_MouseLeftClicked = false;
	m_MouseRightClicked = false;
	m_MousePosition = sf::Vector2i(0, 0);

	init();
}
CControls::~CControls() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CControls", "Deleted");
	#endif
}

void CControls::init() noexcept
{
	m_mCmdBinds.insert(std::pair<std::string, int>("jump", sf::Keyboard::Space));
	m_mCmdBinds.insert(std::pair<std::string, int>("left", sf::Keyboard::A));
	m_mCmdBinds.insert(std::pair<std::string, int>("right", sf::Keyboard::D));
	m_mCmdBinds.insert(std::pair<std::string, int>("fire", (sf::Mouse::Left+2) * -1)); // FIXME: Workaround for use Keyboard+Mouse binds
	m_mCmdBinds.insert(std::pair<std::string, int>("hook", (sf::Mouse::Right+2) * -1)); // FIXME: Workaround for use Keyboard+Mouse binds
}

void CControls::processEvent(const sf::Event &ev) noexcept
{
	if (ev.type == sf::Event::KeyPressed)
	{
		if (ev.key.code == sf::Keyboard::Escape)
		{
			if (Client()->Controller() && !dynamic_cast<CControllerMenu*>(Client()->Controller()))
			{
				if (Client()->Menus().getActive() != CMenus::NONE)
					Client()->Menus().setActive(CMenus::NONE);
				else
					Client()->Menus().setActive(CMenus::MAIN);
			}
			else if (Client()->Menus().getActive() != CMenus::MAIN)
				Client()->Menus().setActive(CMenus::MAIN);
		}
		else if (ev.key.code == sf::Keyboard::F7)
			Client()->m_Debug = !Client()->m_Debug;
		else if (ev.key.code == sf::Keyboard::B)
		{
			if (Client()->Controller())
			{
				int curWeather = Client()->Controller()->Context()->getWeather();
				if (curWeather == WEATHER_SNOW)
					Client()->Controller()->Context()->setWeather(WEATHER_NORMAL);
				else
					Client()->Controller()->Context()->setWeather(++curWeather);
			}
		}
		else if (ev.key.code == sf::Keyboard::R)
		{
			Client()->initializeGameMode("main");
		}
	}
	if (ev.type == sf::Event::MouseMoved)
	{
		m_MousePosition = sf::Vector2i(ev.mouseMove.x, ev.mouseMove.y);
	}
	else if (ev.type == sf::Event::MouseButtonPressed)
	{
		if (ev.mouseButton.button == sf::Mouse::Button::Left)
			m_MouseLeftClicked = true;
		else if (ev.mouseButton.button == sf::Mouse::Button::Right)
			m_MouseRightClicked = true;
	}
	else if (ev.type == sf::Event::MouseButtonReleased)
	{
		if (ev.mouseButton.button == sf::Mouse::Button::Left)
			m_MouseLeftClicked = false;
		else if (ev.mouseButton.button == sf::Mouse::Button::Right)
			m_MouseRightClicked = false;
	}
}

bool CControls::isKeyPressed(const char *pCmd)
{
	std::map<std::string, int>::const_iterator It = m_mCmdBinds.find(pCmd);
	if (It == m_mCmdBinds.cend())
		return false;
	return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>((*It).second));
}

bool CControls::isMousePressed(const char *pCmd)
{
	std::map<std::string, int>::const_iterator It = m_mCmdBinds.find(pCmd);
	if (It == m_mCmdBinds.cend())
		return false;
	return sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>((*It).second * -1 - 2));
}



