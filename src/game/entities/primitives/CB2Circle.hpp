/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CIRCLEB2
#define H_ENTITY_CIRCLEB2

#include <SFML/Graphics.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/CEntity.hpp>

class CB2Circle : public CEntity
{
public:
	CB2Circle(sf::Vector2f worldPos, float radius, sf::Color, const CB2BodyInfo &bodyInfo, int entityType = CEntity::B2CIRCLE) noexcept;
	virtual ~CB2Circle() noexcept;

	virtual void tick() noexcept;
};

#endif
