/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT
#define H_GAME_COMPONENT

class CComponent
{
	friend class CGameClient;
public:
	virtual ~CComponent() { }

	class CGameClient* Client() const noexcept { return m_pGameClient; }

	virtual void update(float deltaTime) noexcept = 0;

protected:
	class CGameClient *m_pGameClient;
};

#endif
