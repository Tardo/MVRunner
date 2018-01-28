/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/system.hpp>
#include <engine/CGame.hpp>
#include "CContext.hpp"

CContext::CContext() noexcept
{
	m_pPlayer = new CPlayer();
	m_NumParticles = 0;
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
		// Search pos
		std::vector<CEntity*>::iterator it = m_vpEntities.begin();
		while (it != m_vpEntities.end())
		{
			if (pEntity->getZLevel() < (*it)->getZLevel())
				break;
			++it;
		}
		std::vector<CEntity*>::iterator nit = m_vpEntities.insert(it, pEntity);
		return nit - m_vpEntities.begin();
	}
	return -1;
}

void CContext::setEntityZLevel(CEntity *pEntity, int ZLevel) noexcept
{
	pEntity->m_ZLevel = ZLevel;
	// TODO
}

int CContext::addParticle(CParticle *pParticle) noexcept
{
	if (pParticle)
	{
		m_vpParticles.push_back(pParticle);
		return m_NumParticles;
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
