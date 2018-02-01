/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_POLYGONB2
#define H_ENTITY_POLYGONB2

#include <SFML/Graphics.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/CEntity.hpp>

class CB2Polygon : public CEntity
{
public:
	CB2Polygon(sf::Vector2f worldPos, int zlevel, const std::vector<sf::Vector2f> points, const sf::Color, const CB2BodyInfo &bodyInfo, int type = CEntity::B2POLY) noexcept;
	CB2Polygon(sf::Vector2f worldPos, int zlevel, const sf::Vector2f points, const sf::Color color, const CB2BodyInfo &bodyInfo, int type = CEntity::B2BOX) noexcept;
	virtual ~CB2Polygon() noexcept;

	virtual void tick() noexcept;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept;

	b2Body* getBody() noexcept { return m_pBody; }

private:
	b2Body *m_pBody;
	sf::RectangleShape m_DbgShape;
	bool m_IsSensor;
};

#endif
