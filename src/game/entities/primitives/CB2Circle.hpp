/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CIRCLEB2
#define H_ENTITY_CIRCLEB2

#include <SFML/Graphics.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/CEntity.hpp>

class CB2Circle : public CEntity
{
public:
	CB2Circle(sf::Vector2f worldPos, int zlevel, float radius, sf::Color, const CB2BodyInfo &bodyInfo, int entityType = CEntity::B2CIRCLE) noexcept;
	virtual ~CB2Circle() noexcept;

	virtual void tick() noexcept;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept;

	b2Body* getBody() const noexcept { return m_pBody; }
	void setShadowSizeFactor(float factor) noexcept { m_ShadowSizeFactor = factor; }

private:
	b2Body *m_pBody;
	sf::CircleShape m_DbgShape;
	bool m_IsSensor;
	float m_ShadowSizeFactor;
	sf::Sprite m_aShadows[MAX_SHADOWS];
	unsigned int m_NumShadows;
};

#endif
