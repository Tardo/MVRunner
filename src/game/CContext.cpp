/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/system.hpp>
#include <engine/CGame.hpp>
#include "CContext.hpp"

CContext::CContext() noexcept
{
	m_pPlayer = new CPlayer();
	m_Weather = WEATHER_NORMAL;
	m_TimerStorm = 0;
	m_ColorClear = sf::Color(86, 86, 86);
	m_ColorShadow = sf::Color(32, 32, 32);
	m_GID = 0;
}
CContext::~CContext() noexcept
{
	if (m_pPlayer)
		delete m_pPlayer;

    std::vector<CEntity*>::iterator itEnt = m_vpEntities.begin();
    while (itEnt != m_vpEntities.end())
    {
    	CEntity *pEnty = (*itEnt);
    	itEnt = m_vpEntities.erase(itEnt);
    	delete pEnty;
    	pEnty = nullptr;
    }
    m_vpEntities.clear();

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CContext", "Borrado");
	#endif
}

int CContext::addEntity(CEntity *pEntity) noexcept
{
	if (pEntity)
	{
		m_vpEntities.push_back(pEntity);
		return m_GID++;
	}
	return -1;
}

void CContext::tick() noexcept
{
	std::vector<CEntity*> ents = m_vpEntities; // Copy fixed list FIXME: tick can create new entities...
    std::vector<CEntity*>::const_iterator citE = ents.begin();
    while (citE != ents.end())
    	(*citE++)->tick();
}

void CContext::clearTrash() noexcept
{
	std::vector<CEntity*>::iterator itE = m_vpEntities.begin();
    while (itE != m_vpEntities.end())
    {
    	CEntity *pEnt = static_cast<CEntity*>(*itE);
    	if (pEnt->isToDelete())
    	{
    		itE = m_vpEntities.erase(itE);
    		delete pEnt;
    		pEnt = nullptr;
    	}
    	else
    		++itE;
    }
}
