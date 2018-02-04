/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_CONTEXT
#define H_GAME_CONTEXT

#include <engine/CSystemFx.hpp>
#include <game/CPlayer.hpp>
#include "CMap.hpp"
#include <vector>

class CContext final
{
public:
	CContext() noexcept;
	~CContext() noexcept;

	CMap& Map() { return m_Map; }

	int addEntity(CEntity *pEntity) noexcept;
	int addParticle(CParticle *pParticle) noexcept;
	void clearParticles() noexcept;

	CEntity* getEntity(std::size_t index) const noexcept
	{
		std::vector<CEntity*>::const_iterator ite = m_vpEntities.begin()+index;
		if (ite != m_vpEntities.end())
			return (*ite);
		return nullptr;
	}
	std::vector<CEntity*>& getAllEntities() noexcept { return m_vpEntities; }
	std::list<CEntity*> getEntitiesByType(int type) noexcept
	{
		std::list<CEntity*> res;
		std::vector<CEntity*>::const_iterator ite = m_vpEntities.begin();
		while (ite != m_vpEntities.end())
		{
			CEntity *pEnt = (*ite++);
			if (pEnt && pEnt->getType() == type)
				res.push_back(pEnt);
		}
		return res;
	}

	std::vector<CParticle*>& getAllParticles() noexcept { return m_vpParticles; }

	std::size_t getNumEntities() noexcept { return m_vpEntities.size(); }
	std::size_t getNumParticles() noexcept { return m_vpParticles.size(); }

	CPlayer* getPlayer() noexcept { return m_pPlayer; }

protected:
	bool m_MapLoaded;

private:
	std::vector<CParticle*> m_vpParticles;
	std::vector<CEntity*> m_vpEntities;
	std::size_t m_NumParticles;

	CPlayer *m_pPlayer;
	CMap m_Map;
};

#endif
