/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CIRCLEB2
#define H_ENTITY_CIRCLEB2

#include <SFML/Graphics.hpp>
#include <game/CEntity.hpp>
#include <engine/CSystemBox2D.hpp>

class CB2Circle : public CEntity
{
public:
	CB2Circle(const sf::Vector2f &worldPos, float radius, float rot, const sf::Color &color, const CB2BodyInfo &bodyInfo, int entityType = CEntity::B2CIRCLE) noexcept;
	virtual ~CB2Circle() noexcept;

	virtual void tick() noexcept;
};

#endif
