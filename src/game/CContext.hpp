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
		if (index < 0 || index >= m_vpEntities.size())
			return nullptr;
		return m_vpEntities[index];
	}
	std::vector<CEntity*>& getEntities() noexcept { return m_vpEntities; }
	std::vector<CParticle*>& getParticles() noexcept { return m_vpParticles; }

	CPlayer* getPlayer() noexcept { return m_pPlayer; }

private:
	std::vector<CParticle*> m_vpParticles;
	std::vector<CEntity*> m_vpEntities;

	CPlayer *m_pPlayer;
	CMap m_Map;
};

#endif
