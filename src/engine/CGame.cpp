/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/system.hpp>
#include "CGame.hpp"
#ifdef SFML_SYSTEM_LINUX
//#include <X11/Xlib.h>
#endif

CGame* CGame::ms_pCoreInstance = nullptr;
CGame::CGame() noexcept
{
	m_pGameClient = nullptr;
}
CGame::~CGame() noexcept
{
    if (m_pGameClient)
    	delete m_pGameClient;
    m_pGameClient = nullptr;

#ifdef DEBUG_DESTRUCTORS
    ups::msgDebug("CGame", "Deleted");
#endif
}


void CGame::init() noexcept
{
	m_pGameClient = new CGameClient();
	m_pGameClient->run();
}


int main()
{
#ifdef SFML_SYSTEM_LINUX
	//XInitThreads();
#endif

	CConfig config("game.cfg");
	if (!config.load())
		ups::msgDebug("MAIN", "No configuration file found... using default values.");
	else
		ups::msgDebug("MAIN", "Configuration file successfully loaded");

	CGame::getInstance()->init();

	if (!config.save())
		ups::msgDebug("CONFIG", "Error while saving configuration!");

	ups::msgDebug("MAIN", "Bye!");
	exit(0);
}
