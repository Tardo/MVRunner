/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_WATER
#define H_ENTITY_WATER

#include "primitives/CB2Polygon.hpp"


class CWater final : public CB2Polygon
{
public:
	CWater(const sf::Vector2f &pos, const sf::Vector2f &size, float rot) noexcept;
	~CWater() noexcept;

	virtual void tick() noexcept final;
	virtual void onSensorIn(CEntity *pEntity) noexcept final;
	virtual void onSensorOut(CEntity *pEntity) noexcept final;

protected:
	std::vector<CEntity*> m_vInEntities;

private:
	b2Fixture *m_pFixture;

	static const CB2BodyInfo ms_BodyInfo;
};

#endif
