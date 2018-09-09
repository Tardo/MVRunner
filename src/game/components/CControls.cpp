/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <game/controllers/CControllerMenu.hpp>
#include "CControls.hpp"

CControls::CControls(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{
	m_aListenKeyBindCmd[0] = 0x0;

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
	setKeyBind("jump", sf::Keyboard::Space);
	setKeyBind("left", sf::Keyboard::A);
	setKeyBind("right", sf::Keyboard::D);
	setKeyBind("fire", sf::Mouse::Left, true); // FIXME: Workaround for use Keyboard+Mouse binds
	setKeyBind("hook", sf::Mouse::Right, true); // FIXME: Workaround for use Keyboard+Mouse binds
}

void CControls::setKeyBind(const char *pCmd, int key, bool isMouseButton) noexcept
{
	std::map<std::string, int>::iterator it = m_mCmdBinds.find(pCmd);
	if (it == m_mCmdBinds.end())
		m_mCmdBinds.insert(std::pair<std::string, int>(pCmd, isMouseButton?(key+2)*-1:key));
	else
		(*it).second = isMouseButton?(key+2)*-1:key;
}

void CControls::listenKeyBind(const char *pCmd) noexcept
{
	ups::strCopy(m_aListenKeyBindCmd, pCmd, MAX_COMMAND_LENGTH);
}

void CControls::processEvent(const sf::Event &ev) noexcept
{
	if (ev.type == sf::Event::KeyPressed)
	{
		if (ev.key.code == sf::Keyboard::Escape)
		{
			m_aListenKeyBindCmd[0] = 0x0;

			if (Client()->Menus().getActiveModal() == CMenus::NONE)
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
			else
				Client()->Menus().setActiveModal(CMenus::NONE);
		}
		else if (m_aListenKeyBindCmd[0])
		{
			setKeyBind(m_aListenKeyBindCmd, ev.key.code);
			m_aListenKeyBindCmd[0] = 0x0;
			Client()->Menus().setActiveModal(CMenus::NONE);
			return;
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
	else if (ev.type == sf::Event::MouseButtonPressed)
	{
		if (m_aListenKeyBindCmd[0])
		{
			setKeyBind(m_aListenKeyBindCmd, ev.key.code, true);
			m_aListenKeyBindCmd[0] = 0x0;
			Client()->Menus().setActiveModal(CMenus::NONE);
			return;
		}
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
	return sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(CControls::getRealKey((*It).second)));
}

const char* CControls::getKeyName(int key)
{
	if (key < 0)
	{
		key = getRealKey(key);
		if (key < sf::Mouse::ButtonCount)
		{
			static const char *pMouseNames[] = {
				"Left",
				"Right",
				"Middle",
				"XButton1",
				"XButton2"
			};
			return pMouseNames[key];
		}
	} else if (key < sf::Keyboard::KeyCount)
	{
		static const char *pKeyboardNames[] = {
			"A",
			"B",
			"C",
			"D",
			"E",
			"F",
			"G",
			"H",
			"I",
			"J",
			"K",
			"L",
			"M",
			"N",
			"O",
			"P",
			"Q",
			"R",
			"S",
			"T",
			"U",
			"V",
			"W",
			"X",
			"Y",
			"Z",
			"Num0",
			"Num1",
			"Num2",
			"Num3",
			"Num4",
			"Num5",
			"Num6",
			"Num7",
			"Num8",
			"Num9",
			"Escape",
			"LControl",
			"LShift",
			"LAlt",
			"LSystem",
			"RControl"
			"RShift",
			"RAlt",
			"RSystem",
			"Menu",
			"LBracket",
			"RBracket",
			"SemiColon",
			"Comma",
			"Period",
			"Quote",
			"Slash",
			"BackSlash",
			"Tilde",
			"Equal",
			"Dash",
			"Space",
			"Return",
			"BackSpace",
			"Tab",
			"PageUp",
			"PageDown",
			"End",
			"Home",
			"Insert",
			"Delete",
			"Add",
			"Subtract",
			"Multiply",
			"Divide",
			"Left",
			"Right",
			"Up",
			"Down",
			"Numpad0",
			"Numpad1",
			"Numpad2",
			"Numpad3",
			"Numpad4",
			"Numpad5",
			"Numpad6",
			"Numpad7",
			"Numpad8",
			"Numpad9",
			"F1",
			"F2",
			"F3",
			"F4",
			"F5",
			"F6",
			"F7",
			"F8",
			"F9",
			"F10",
			"F11",
			"F12",
			"F13",
			"F14",
			"F15",
			"Pause"
		};

		return pKeyboardNames[key];
	}

	return "Unknown";
}
