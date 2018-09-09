/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CZONE
#define H_ENTITY_CZONE

#include "primitives/CB2Polygon.hpp"
#include <vector>

class CZone final : public CB2Polygon
{
	static const sf::Vector2f SLAVE_SIZE;

public:
	CZone(const sf::Vector2f &pos, const sf::Vector2f &size, float rot, float gravityScale) noexcept;
	~CZone() noexcept;

	virtual void tick() noexcept final;
	virtual void onSensorIn(CEntity *pEntity) noexcept final;
	virtual void onSensorOut(CEntity *pEntity) noexcept final;

private:
	b2Fixture *m_pFixture;
	float m_GravityScale;

	static const CB2BodyInfo ms_BodyInfo;
};

#endif
