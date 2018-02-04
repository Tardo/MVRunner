/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_CUI
#define H_GAME_COMPONENT_CUI

#include <game/CComponent.hpp>

#define BROADCAST_MAX_LENGTH	64
#define HELP_TEXT_MAX_LENGTH	128


class CUI final : public CComponent
{
public:
	enum
	{
		ALIGN_LEFT=0,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};


	CUI() noexcept;
	virtual ~CUI() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

	void update() noexcept;
	bool doButton(const char*pText, const sf::FloatRect &bounds, unsigned int fontSize, int align = 0) noexcept;

	void showBroadcastMessage(const char *pMsg, float duration) noexcept;
	void showHelpMessage(const char *pMsg) noexcept;

	const sf::Vector2i& getMousePos() const { return m_MousePosition; }

private:
	bool m_MouseLeftClicked;
	sf::Vector2i m_MousePosition;

	sf::Int64 m_TimerBroadcast;
	float m_BroadcastDuration;
	char m_aBroadcastMsg[BROADCAST_MAX_LENGTH];
	char m_aHelpMsg[HELP_TEXT_MAX_LENGTH];

	void drawHUD(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void drawCursor(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
	void drawDebugInfo(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
};

#endif
