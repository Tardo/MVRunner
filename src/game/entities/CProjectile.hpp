#ifndef GAME_ENTITIES_PROJECTILE
#define GAME_ENTITIES_PROJECTILE

#include "primitives/CB2Polygon.hpp"
#include <engine/CSystemFx.hpp>
//#include <vector>


class CProjectile final : public CB2Polygon
{
	static const sf::Vector2f SIZE;
public:
	CProjectile(const sf::Vector2f &pos, const sf::Vector2f &size, const sf::Vector2f &dir, float speed, class CPlayer *pOwner, unsigned int type, unsigned int subtype) noexcept;
	~CProjectile() noexcept;

	virtual void tick() noexcept final;
    virtual void onContact(CEntity *pEntity, const sf::Vector2f &worldPos) noexcept final;

    void setDir(sf::Vector2f dir) noexcept { m_Dir = dir; }
    sf::Vector2f getDir() const noexcept { return m_Dir; }

    void setSpeed(float speed) noexcept { m_Speed = speed; }
    float getSpeed() const noexcept { return m_Speed; }

    unsigned int getProjType() const noexcept { return m_ProjType; }
    unsigned int getProjSubType() const noexcept { return m_ProjSubType; }

    class CPlayer *getOwner() const noexcept { return m_pPlayer; }

protected:
	b2Body *m_pBody;
	float m_Speed;
	sf::Vector2f m_Dir;
	CSystemFx *m_pSystemFx;

private:
	class CPlayer *m_pPlayer;
	unsigned int m_ProjType;
	unsigned int m_ProjSubType;

	static const CB2BodyInfo ms_BodyInfo;
};

#endif
