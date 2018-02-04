/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_PLAYER_RENDER
#define H_GAME_COMPONENT_PLAYER_RENDER

#include <game/CComponent.hpp>
#include <game/entities/CCharacter.hpp>

class CPlayerRender final : public CComponent
{
public:
	CPlayerRender() noexcept;
	~CPlayerRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

private:
	void renderPlayer(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept;
};

#endif
