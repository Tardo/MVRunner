/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CClient.hpp"

#include <base/system.hpp>
#ifdef SFML_SYSTEM_LINUX
//#include <X11/Xlib.h>
#endif

CClient* CClient::ms_pCoreInstance = nullptr;
CClient::CClient() noexcept
{
	m_pGameClient = nullptr;
}
CClient::~CClient() noexcept
{
    if (m_pGameClient)
    	delete m_pGameClient;
    m_pGameClient = nullptr;

#ifdef DEBUG_DESTRUCTORS
    ups::msgDebug("CGame", "Deleted");
#endif
}


void CClient::init() noexcept
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

	CClient::getInstance()->init();

	if (!config.save())
		ups::msgDebug("CONFIG", "Error while saving configuration!");

	ups::msgDebug("MAIN", "Bye!");
	exit(0);
}
