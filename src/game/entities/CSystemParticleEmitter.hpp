/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CSYSTEMPARTICLEEMITTER
#define H_ENTITY_CSYSTEMPARTICLEEMITTER

#include <game/CEntity.hpp>

class CSystemParticleEmitter final : public CEntity
{
public:
	CSystemParticleEmitter(const sf::Vector2f &worldPos, const sf::Vector2f &force, const sf::Color &color, const sf::Vector2f &size, float lifetime, float delay) noexcept;
    virtual ~CSystemParticleEmitter() noexcept;

    virtual void tick() noexcept final;

    sf::Vector2f m_Pos;
    sf::Vector2f m_Force;
    sf::Color m_Color;
    float m_Lifetime;
    float m_Delay;

private:
    sf::Int64 m_Timer;
    b2PolygonShape m_Shape;
};

#endif
