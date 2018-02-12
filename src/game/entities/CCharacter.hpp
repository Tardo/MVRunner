/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CCHARACTER
#define H_ENTITY_CCHARACTER

#include <engine/CConfig.hpp>
#include "primitives/CB2Circle.hpp"
#include <vector>

#define MAX_HEALTH_PLAYER		4

class CWeapon final
{
public:
	CWeapon()
	{
		m_Active = false;
		m_Ammo = -1;
		m_MaxAmmo = -1;
	}

	bool m_Active;
	int m_Ammo;
	int m_MaxAmmo;
};


class CCharacter final : public CB2Circle
{
public:
	enum
	{
		STATE_NORMAL = 0,
		STATE_FREEZED = 2<<1,
		STATE_ROTATE = 2<<2,

	    MOVE_STATE_STOP = 0,
	    MOVE_STATE_RIGHT = 2<<1,
	    MOVE_STATE_DOWN = 2<<2,
	    MOVE_STATE_LEFT = 2<<3,
	    MOVE_STATE_UP = 2<<4,
	};

	static const float SIZE;
	static const long ANIM_TIME;
	static const unsigned int ANIM_SUBRECTS;

	CCharacter(const sf::Vector2f &pos, class CPlayer *pPlayer) noexcept;
	~CCharacter() noexcept;

	virtual void tick() noexcept final;
    virtual void onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept final;
	virtual void onSensorIn(CEntity *pEntity) noexcept final;
	virtual void onSensorOut(CEntity *pEntity) noexcept final;
	virtual void takeHealth(int amount, class CPlayer *pPlayer) noexcept final;

    void doImpulse(sf::Vector2f dir, float energy) noexcept;

    void setActiveWeapon(int wid) noexcept;
    CWeapon* getWeapon(int wid) noexcept { return &m_aWeapons[wid]; }
    int getActiveWeapon() const noexcept { return m_ActiveWeapon; }
    void giveWeapon(int wid, int ammo, int maxammo) noexcept;
    void takeWeapon(int wid) noexcept
    {
    	if (wid < 0 || wid >= NUM_WEAPONS)
    		return;
    	m_aWeapons[wid].m_Active = false;
    	m_aWeapons[wid].m_Ammo = 0;
    }
    bool hasWeapon(int wid) const noexcept { return wid >= 0 && wid < NUM_WEAPONS && m_aWeapons[wid].m_Active; }

    virtual class CPlayer* getOwner() noexcept { return m_pPlayer; }

    bool isAlive() const { return m_Alive; }
    bool isVisible() const { return m_Visible; }
    void giveHealth(int amount) noexcept;
    void doFire() noexcept;
    void doHook() noexcept;
    void setVisible(bool visible) noexcept;

    void move(int moveState) noexcept;
    void teleport(const sf::Vector2f &worldPosTo) noexcept;

    int getCharacterState() const { return m_CharacterState; }
    void setCharacterState(int state);

    std::vector<CEntity*>& getSensorEntities() { return m_vpSensorEntities; }

	sf::Int64 m_TimerFire;
	sf::Int64 m_TimerDamageIndicator;
	sf::Int64 m_TimerHeartbeat;
	sf::Int64 m_TimerCharacterState;

protected:
	class CPlayer *m_pPlayer;
	int m_CharacterState;
	sf::CircleShape m_ShapeTimerIndicator;

private:
	bool m_FlagDamage;
	bool m_Alive;
	bool m_Visible;

	int m_State;
	int m_LastState;

	CWeapon m_aWeapons[NUM_WEAPONS];
	int m_ActiveWeapon;
	bool m_Fire;
	int m_Jumps;

	std::vector<CEntity*> m_vpSensorEntities;

	static const CB2BodyInfo ms_BodyInfo;
};

#endif
