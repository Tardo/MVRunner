/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CAMBIENTSOUND
#define H_ENTITY_CAMBIENTSOUND

#include <SFML/Audio.hpp>
#include <game/CEntity.hpp>

class CAmbientSound final : public CEntity
{
public:
	CAmbientSound(const sf::Vector2f &worldPos, int soundId, float radius, bool loop = true, float volume = 100.0f) noexcept;
	~CAmbientSound() noexcept;

	virtual void tick() noexcept final;

private:
	sf::Sound *m_pSound;
};

#endif
