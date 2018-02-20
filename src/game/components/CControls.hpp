/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_CONTROLS
#define H_GAME_COMPONENT_CONTROLS

#include <game/CComponent.hpp>
#include <map>


class CControls final : public CComponent
{
public:
	CControls(CGameClient *pGameClient) noexcept;
	virtual ~CControls() noexcept;

	void init() noexcept;

	void processEvent(const sf::Event &ev) noexcept;

	const sf::Vector2i& getMousePos() const { return m_MousePosition; }
	bool isMouseLeftClicked() const { return m_MouseLeftClicked; }
	bool isMouseRightClicked() const { return m_MouseRightClicked; }

	bool isKeyPressed(const char *pCmd);
	bool isMousePressed(const char *pCmd);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final
	{
		// Not Used
	}

private:
	bool m_MouseLeftClicked;
	bool m_MouseRightClicked;
	sf::Vector2i m_MousePosition;
	std::map<std::string, int> m_mCmdBinds;
};

#endif
