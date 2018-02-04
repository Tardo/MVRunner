/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_MAPRENDER
#define H_GAME_COMPONENT_MAPRENDER

#include <game/CComponent.hpp>


class CMapRender final : public CComponent
{
public:
	CMapRender(int render) noexcept;
	~CMapRender() noexcept;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;
protected:
	int m_Render;

private:
	void renderTilemap(sf::RenderTarget& target, sf::RenderStates states, const sf::IntRect &mapBounds, int layerIndex, const sf::Color &lightColor) const noexcept;
};

#endif
