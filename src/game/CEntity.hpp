/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_ENTITY
#define H_GAME_ENTITY

#include <SFML/Graphics.hpp>
#include <vector>

#include "../engine/CSystemBox2D.hpp"

#define MAX_SHADOWS	4

enum _entityCategory
{
	CAT_GENERIC = 2<<1,
	CAT_CHARACTER_PLAYER = 2<<2,
	CAT_CHARACTER_SENSOR = 2<<3,
	CAT_PROJECTILE = 2<<4,
	CAT_BUILD = 2<<5,
	CAT_HITBOX = 2<<6,
	CAT_FIRE = 2<<7,
	CAT_SIGN = 2<<8,
	CAT_CHAIN = 2<<9,
	CAT_WATER = 2<<10,
	CAT_ANTENNA = 2<<11,
	CAT_ZONE = 2<<12,
};

class CEntity
{
	friend class CContext;
public:
	enum
	{
		B2CIRCLE=0,
		B2POLY,
		B2CHAIN,

		CHARACTER,
		HITBOX,
		PROJECTILE,
		FIRE,
		AMBIENTSOUND,
		SIGN,
		LIGHT,
		SIMPLE_PARTICLE,
		WATER,
		ANTENNA,
		SYSTEM_PARTICLE_EMITTER,
		ZONE,

		NUM_ENTITIES,

		FX_NONE = 0,
		FX_SPARKS,
	};

	CEntity(int type, const sf::Color color = sf::Color::Transparent);
	virtual ~CEntity();

	int getType() const noexcept { return m_Type; }
	virtual void destroy() noexcept { m_ToDelete = true; }
	bool isToDelete() const noexcept { return m_ToDelete; }

	virtual void setSelfDelete(bool status) noexcept { m_SelfDelete = status; }
	bool canSelfDelete() const noexcept { return m_SelfDelete; }

	int getHealth() const noexcept { return m_Health; }
	virtual void setHealth(int health) noexcept { m_Health = health; }
	virtual void increaseHealth(int amount) noexcept { m_Health += amount; }
	sf::Uint64 getID() const noexcept { return m_Id; }

	b2Body* getBody() noexcept { return m_pBody; }

	virtual void takeHealth(int amount, class CPlayer *pPlayer) noexcept { m_Health -= amount; }
	virtual void kill() noexcept { m_Health = 0; }

	virtual void tick() noexcept = 0;
	virtual void onSensorIn(CEntity *pEntity) noexcept { }
	virtual void onSensorOut(CEntity *pEntity) noexcept { }
	virtual void onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept { }
	virtual void onPostSolve(CEntity* pEntity, const sf::Vector2f &worldPos, float impulse) noexcept { }

	sf::Color m_Color;
	int m_TextureId;
	int m_ContactFx;

protected:
	int m_Health;
	sf::Uint64 m_Id;
	sf::Int64 m_TimerStateAction;
	b2Body *m_pBody;

private:
	int m_Type;
	bool m_ToDelete;
	bool m_SelfDelete;
};

#endif
