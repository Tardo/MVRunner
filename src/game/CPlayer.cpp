/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CAssetManager.hpp>
#include <engine/CSystemFx.hpp>
#include "CPlayer.hpp"

CPlayer::CPlayer() noexcept
{
	m_pCharacter = nullptr;
	m_Points = 0;
}
CPlayer::~CPlayer() noexcept
{
	destroyCharacter();
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CPlayer", "Deleted");
	#endif
}

void CPlayer::update(float deltaTime) noexcept
{ }

CEntity* CPlayer::createCharacter(const sf::Vector2f &pos) noexcept
{
	if (m_pCharacter)
		return m_pCharacter;

	m_pCharacter = new CCharacter(pos, this);
	return m_pCharacter;
}
void CPlayer::destroyCharacter() noexcept
{
	if (m_pCharacter) {
		m_pCharacter->destroy();
		m_pCharacter = nullptr;
	}
}

