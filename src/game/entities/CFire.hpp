/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_FIRE
#define H_ENTITY_FIRE

#include "primitives/CB2Circle.hpp"
#include <SFML/Audio/Sound.hpp>
#include <vector>


class CFire final : public CB2Circle
{
public:
	static const float SIZE;

	CFire(const sf::Vector2f &pos, const sf::Vector2f &dir, float force, float lifeTime) noexcept;
	~CFire() noexcept;

	virtual void tick() noexcept final;
	virtual void onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept;
    //virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::Vector2f m_LastPos;
	long m_TickStart;
	float m_LifeTime;
	sf::Vector2f m_Dir;
	float m_Force;

	static const CB2BodyInfo ms_BodyInfo;
};

#endif
