/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_CONTROLS
#define H_GAME_COMPONENT_CONTROLS

#include <game/CComponent.hpp>
#include <map>


class CControls final : public CComponent
{
public:
	enum
	{
		MAX_COMMAND_LENGTH = 16
	};

	CControls(CGameClient *pGameClient) noexcept;
	virtual ~CControls() noexcept;

	void init() noexcept;

	void processEvent(const sf::Event &ev) noexcept;

	std::map<std::string, int>& getCmdBinds() { return m_mCmdBinds; }

	bool isKeyPressed(const char *pCmd);
	bool isMousePressed(const char *pCmd);
	bool isButtonPressed(const char *pCmd);

	void setKeyBind(const char *pCmd, int key, bool isMouseButton=false) noexcept;
	void listenKeyBind(const char *pCmd) noexcept;

	// FIXME: When SFML use the same enum for keyboard and mouse buttons delete this!!
	static int getRealKey(int key) { return key<0?key * -1 - 2:key; }
	static const char* getKeyName(int key);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final
	{
		// Not Used
	}

private:
	std::map<std::string, int> m_mCmdBinds;
	char m_aListenKeyBindCmd[MAX_COMMAND_LENGTH];
};

#endif
