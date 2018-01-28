/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_ENTITY
#define H_GAME_ENTITY

#include <base/math.hpp>
#include <SFML/Graphics.hpp>
#include <base/system.hpp>
#include <vector>

#define MAX_SHADOWS	4

enum _entityCategory
{
	CAT_GENERIC = 2<<1,
	CAT_CHARACTER_PLAYER = 2<<2,
	CAT_CHARACTER_SENSOR = 2<<3,
	CAT_BOT_SENSOR = 2<<4,
	CAT_PROJECTILE = 2<<5,
	CAT_PROJECTILE_MAP = 2<<6,
	CAT_BUILD = 2<<7,
	CAT_HITBOX = 2<<8,
	CAT_FIRE = 2<<9,
	CAT_BOX = 2<<10,
	CAT_SIGN = 2<<11,
};

class CEntity : public sf::Drawable
{
	friend class CContext;
public:
	enum
	{
		B2BOX=0,
		B2CIRCLE,
		B2POLY,

		CHARACTER,
		HITBOX,
		PROJECTILE,
		BOX,
		FIRE,
		AMBIENTSOUND,
		SIGN,

		NUM_ENTITIES
	};

	CEntity(int type, int zlevel)
	{
		m_Id = -1;
		m_Health = -1;
		m_pShape = nullptr;
		m_Type = type;
		m_ToDelete = false;
		m_ZLevel = zlevel;
		m_TimerStateAction = 0;
	}
	virtual ~CEntity()
	{
		delete m_pShape;
		m_pShape = nullptr;
	}

	sf::Shape* getShape() noexcept { return m_pShape; }
	int getType() const noexcept { return m_Type; }
	virtual void destroy() noexcept { m_ToDelete = true; }
	bool isToDelete() const noexcept { return m_ToDelete; }

	int getHealth() const noexcept { return m_Health; }
	virtual void setHealth(int health) noexcept { m_Health = health; }
	virtual void increaseHealth(int amount) noexcept { m_Health += amount; }
	int getID() const noexcept { return m_Id; }

	const int getZLevel() const noexcept { return m_ZLevel; }

	virtual void takeHealth(int amount, class CPlayer *pPlayer) noexcept { m_Health -= amount; }
	virtual void kill() noexcept { m_Health = 0; }

	virtual void tick() noexcept = 0;
	virtual void onSensorIn(CEntity *pEntity) noexcept { }
	virtual void onSensorOut(CEntity *pEntity) noexcept { }
	virtual void onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept { }
	virtual void onPostSolve(CEntity* pEntity, const sf::Vector2f &worldPos, float impulse) noexcept { }

protected:
	sf::Shape *m_pShape;
	int m_Health;
	int m_Id;
	sf::Int64 m_TimerStateAction;

private:
	int m_Type;
	bool m_ToDelete;
	int m_ZLevel;
};

#endif
