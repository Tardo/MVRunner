/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_INTERFACE_COMPONENT
#define H_ENGINE_INTERFACE_COMPONENT


enum
{
	RENDER_BACK=0,
	RENDER_FRONT,
	RENDER_FOREGROUND
};


class CSystem
{
	friend class CGameClient;

public:
	virtual ~CSystem()
	{ }

	class CGameClient* Client() const noexcept { return m_pGameClient; }

	virtual bool init(class CGameClient *pGameClient)
	{
		m_pGameClient = pGameClient;
		return true;
	}
	virtual void update(float deltaTime) = 0;

protected:
	class CGameClient *m_pGameClient;
};

#endif
