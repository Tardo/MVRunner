/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_DEBUGGER_RENDER
#define H_GAME_DEBUGGER_RENDER

#include <game/CComponent.hpp>

class CDebuggerRender final : public CComponent
{
public:
	CDebuggerRender() noexcept;
	~CDebuggerRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;

private:
	void renderBox2D(sf::RenderTarget& target, sf::RenderStates states) const noexcept;
};

#endif