/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_CONTROLLER
#define H_GAME_CONTROLLER

#include <game/CContext.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

#define MARGIN_CREATE_OBJECTS	150.0f

class CSpawn
{
public:
	CSpawn()
	{
		m_Pos = VECTOR_ZERO;
		m_Dir = VECTOR_ZERO;
	}
	CSpawn(const sf::Vector2f &pos) noexcept
	{
		m_Pos = pos;
		m_Dir = VECTOR_ZERO;
	}
	CSpawn(const sf::Vector2f &pos, const sf::Vector2f &dir) noexcept
	{
		m_Pos = pos;
		m_Dir = dir;
	}

	sf::Vector2f m_Pos;
	sf::Vector2f m_Dir;
};


class CController
{
public:
	CController() noexcept;
	CController(CContext *pContext) noexcept;
	virtual ~CController() noexcept;

	class CGame* Game() const noexcept { return m_pGame; }
	CContext *Context() const noexcept { return m_pGameContext; }

	virtual void tick() noexcept;

	virtual void onStart() noexcept;
	virtual void onResetGame() noexcept;
	virtual void onCharacterDeath(CCharacter *pVictim, CPlayer *pKiller) noexcept;
	virtual void onSystemEvent(sf::Event *pEvent) noexcept;
	virtual bool onMapTile(unsigned int tileId, const sf::Vector2f &pos, unsigned int tileDir, unsigned int modifierId) noexcept;
	virtual void onMapObject(CMapRenderObject *pMapObj, int objId, const sf::Vector2f &worldPos, const sf::Vector2f &size) noexcept;

	virtual void updateCamera(float deltaTime) noexcept;

	virtual bool isStaticObject(const char *pType) const noexcept; // FIXME: Use other implementation!!

	CSpawn m_PlayerSpawnPos;
	CLight *m_pPlayerLight;

private:
	class CGame *m_pGame;
	CContext *m_pGameContext;
};

#endif
