/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT
#define H_GAME_COMPONENT

#include <SFML/Graphics.hpp>

class CComponent : public sf::Drawable
{
	friend class CGameClient;
public:
	virtual ~CComponent() { }

	class CGameClient* Client() const noexcept { return m_pGameClient; }

protected:
	class CGameClient *m_pGameClient;
};

#endif
