/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/system.hpp>
#include <engine/CGame.hpp>
#include "CContext.hpp"

CContext::CContext() noexcept
{
	m_pPlayer = new CPlayer();
	m_Weather = WEATHER_NORMAL;
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

    std::vector<CParticle*>::const_iterator itp = m_vpParticles.cbegin();
	while (itp != m_vpParticles.cend())
	{
		CParticle *pParticle = (*itp);
		delete pParticle;
		pParticle = nullptr;
		++itp;
	}
	m_vpParticles.clear();

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

int CContext::addParticle(CParticle *pParticle) noexcept
{
	if (pParticle)
	{
		m_vpParticles.push_back(pParticle);
		return m_vpParticles.size() - 1;
	}
	return -1;
}

void CContext::clearParticles() noexcept
{
	std::vector<CParticle*>::const_iterator itp = m_vpParticles.cbegin();
	while (itp != m_vpParticles.cend())
	{
		(*itp)->m_Duration = 0;
		++itp;
	}
}
