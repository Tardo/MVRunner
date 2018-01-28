/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CBOX
#define H_ENTITY_CBOX

#include "primitives/CB2Polygon.hpp"
#include <SFML/Audio/Sound.hpp>
#include <engine/CSystemFx.hpp>
#include <vector>

#define MAX_HEALTH_BOX		255

enum
{
	BOX_WOOD=1
};


class CBox final : public CB2Polygon
{
public:
	CBox(const sf::Vector2f &pos, const sf::Vector2f &size, const sf::Vector2f &dir, float force, unsigned int typeBox, unsigned int level = 0u, const sf::Color &color = sf::Color::White) noexcept;
	~CBox() noexcept;

	virtual void tick() noexcept final;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept final;
    void takeHealth(int amount, class CPlayer *pPlayer) noexcept;

	unsigned int getBoxType() const noexcept { return m_BoxType; }

	sf::Vector2f m_InitialFirePos;

protected:
	CSystemFx *m_pSystemFx;

private:
	unsigned int m_BoxType;
	sf::Vector2f m_LastPos;
	long m_TickStart;
	sf::Sound m_Sound;
	sf::Vector2f m_Dir;

	static const CB2BodyInfo ms_BodyInfo;
};

#endif
