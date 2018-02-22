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
}
CContext::~CContext() noexcept
{
	if (m_pPlayer)
		delete m_pPlayer;

	m_pPlayer = nullptr;

    std::vector<CEntity*>::iterator itEnt = m_vpEntities.begin();
    while (itEnt != m_vpEntities.end())
    {
    	CEntity *pEnty = (*itEnt);

    	// Is a MapObject Entity?
		if (Map().isMapLoaded())
		{
			// TODO: Add a flag for know if is a map object entity
			std::list<CMapRenderObject*> mapObjs = Map().getObjects()->queryAll();
			std::list<CMapRenderObject*>::iterator itObj = mapObjs.begin();
			while (itObj != mapObjs.end())
			{
				if ((*itObj)->m_pEntity == pEnty)
				{
					(*itObj)->m_pEntity = nullptr;
					break;
				}
				++itObj;
			}
		}


    	delete pEnty;
    	pEnty = nullptr;
    	itEnt = m_vpEntities.erase(itEnt);
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
		return m_vpEntities.size() - 1;
	}
	return -1;
}
