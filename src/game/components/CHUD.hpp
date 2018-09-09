/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_HUD
#define H_GAME_COMPONENT_HUD

#include <game/CComponent.hpp>

class CHUD final : public CComponent
{
public:
	CHUD(CGameClient *pGameClient) noexcept;
	~CHUD() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

	void showBroadcastMessage(const char *pMsg, float duration) noexcept;
	void showHelpMessage(const char *pMsg) noexcept;

private:
	void renderTime(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void renderMessages(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
};

#endif
