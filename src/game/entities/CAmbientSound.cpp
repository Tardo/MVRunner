/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CAmbientSound.hpp"
#include <engine/CGame.hpp>
#include <engine/CSystemSound.hpp>

CAmbientSound::CAmbientSound(const sf::Vector2f &worldPos, int soundId, float radius, bool loop, float volume) noexcept
: CEntity(CEntity::AMBIENTSOUND)
{
	CGame *pGame = CGame::getInstance();
	m_pSound = pGame->Client()->getSystem<CSystemSound>()->createSound(soundId, worldPos, volume, loop, radius, true);

	m_Id = pGame->Client()->Controller()->Context()->addEntity(this);
}
CAmbientSound::~CAmbientSound() noexcept
{
	m_pSound->stop();
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CAmbientSound", "Deleted");
	#endif
}

void CAmbientSound::tick() noexcept
{
	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->isClipped(sf::Vector2f(m_pSound->getPosition().x, m_pSound->getPosition().z), SCREEN_MARGIN_DESTRUCTION))
		destroy();
}
