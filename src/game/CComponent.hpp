/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT
#define H_GAME_COMPONENT

#include <SFML/Graphics.hpp>

enum
{
	RENDER_BACK=0,
	RENDER_FRONT,
	RENDER_FOREGROUND
};

class CComponent : public sf::Drawable
{
public:
	CComponent(class CGameClient *pGameClient)
	{
		m_pGameClient = pGameClient;
	}

	class CGameClient* Client() const noexcept { return m_pGameClient; }

protected:
	class CGameClient *m_pGameClient;
};

#endif
