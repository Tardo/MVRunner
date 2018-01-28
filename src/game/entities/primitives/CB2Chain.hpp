/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CHAINB2
#define H_ENTITY_CHAINB2

#include <SFML/Graphics.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/CEntity.hpp>

class CB2Chain : public CEntity
{
public:
	CB2Chain(sf::Vector2f worldPos, int zlevel, const std::vector<sf::Vector2f> points, const sf::Color, const CB2BodyInfo &bodyInfo, int entityType = CEntity::B2POLY) noexcept;
	virtual ~CB2Chain() noexcept;

	virtual void tick() noexcept;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept;

	b2Body* getBody() noexcept { return m_pBody; }

protected:
	sf::VertexArray m_LineShape;
	sf::Color m_Color;

private:
	b2Body *m_pBody;
	bool m_IsSensor;
	sf::VertexArray m_DbgShape;
};

#endif
