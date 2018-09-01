/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_PLAYER_RENDER
#define H_GAME_COMPONENT_PLAYER_RENDER

#include <game/CComponent.hpp>
#include <game/entities/CCharacter.hpp>

class CPlayerRender final : public CComponent
{
public:
	CPlayerRender(CGameClient *pGameClient) noexcept;
	~CPlayerRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

protected:
	sf::Sprite *m_pSpriteHookBody;

private:
	void renderHook(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept;
	void renderPlayer(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept;
	void renderPlayerBody(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar, const sf::Vector2f &charPos, float charRot) const noexcept;
	void renderWeapon(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar, const sf::Vector2f &charPos, const sf::Vector2f &charDir) const noexcept;
	void renderPlayerLights(sf::RenderTarget& target, sf::RenderStates states, CCharacter *pChar) const noexcept;
};

#endif
