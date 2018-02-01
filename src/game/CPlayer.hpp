/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_COMPONENT_PLAYER
#define H_GAME_COMPONENT_PLAYER

#include <game/CEntity.hpp>
#include <game/CComponent.hpp>
#include <game/entities/CCharacter.hpp>


class CPlayer
{
public:
	CPlayer() noexcept;
	virtual ~CPlayer() noexcept;

	virtual void update(float deltaTime) noexcept;

	virtual CEntity* createCharacter(const sf::Vector2f &pos) noexcept;
	virtual void destroyCharacter() noexcept;
	CCharacter* getCharacter() const noexcept { return m_pCharacter; }

	void addPoints(int points) noexcept { m_Points += points; }
	void resetPoints() noexcept { m_Points = 0; }
	const unsigned int getPoints() const noexcept { return m_Points; }

	void giveWeapon(int wid, int ammo, int maxammo) noexcept;

protected:
	CCharacter *m_pCharacter;

private:
	unsigned int m_Points;
};

#endif
