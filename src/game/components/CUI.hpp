/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_CUI
#define H_GAME_COMPONENT_CUI

#include <game/CComponent.hpp>


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

private:
	bool m_MouseLeftClicked;
};

#endif
