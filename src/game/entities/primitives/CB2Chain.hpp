/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CHAINB2
#define H_ENTITY_CHAINB2

#include <SFML/Graphics.hpp>
#include <game/CEntity.hpp>
#include <engine/CSystemBox2D.hpp>

class CB2Chain : public CEntity
{
public:
	CB2Chain(const sf::Vector2f &worldPos, const std::vector<sf::Vector2f> &points, float rot, const sf::Color &color, const CB2BodyInfo &bodyInfo, int entityType = CEntity::B2CHAIN) noexcept;
	virtual ~CB2Chain() noexcept;

	virtual void tick() noexcept;
	virtual void onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept final;
};

#endif
