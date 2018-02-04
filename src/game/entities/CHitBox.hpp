/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CHITBOX
#define H_ENTITY_CHITBOX

#include "primitives/CB2Polygon.hpp"
#include <SFML/Audio/Sound.hpp>
#include <vector>

enum
{
	HITBOX_CHARACTER_BODY=0,
	HITBOX_CHARACTER_LEG,
	HITBOX_CHARACTER_HEAD,
	HITBOX_EXPLOSION_TRASH,

	HITBOX_WOOD_BURNED_1,
	HITBOX_WOOD_BURNED_2,
	HITBOX_WOOD_BURNED_3,
	HITBOX_WOOD_BROKEN_1,
	HITBOX_WOOD_BROKEN_2,
	HITBOX_WOOD_BROKEN_3,
};


class CHitBox final : public CB2Polygon
{
public:
	CHitBox(const sf::Vector2f &pos, const sf::Vector2f &size, const sf::Vector2f &dir, float force, float lifeTime, unsigned int typeHitBox, unsigned int level = 0u, const sf::Color &color = sf::Color::White, int textureId = -1) noexcept;
	~CHitBox() noexcept;

	virtual void tick() noexcept final;
    //virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	unsigned int getHitBoxType() const noexcept { return m_HitBoxType; }
	int m_TextureId;

private:
	unsigned int m_HitBoxType;
	sf::Vector2f m_LastPos;
	long m_TickStart;
	float m_LifeTime;
	sf::Sound m_Sound;
	sf::Vector2f m_Dir;

	static const CB2BodyInfo ms_BodyInfo;
};

#endif
