/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CSIGN
#define H_ENTITY_CSIGN

#include "primitives/CB2Polygon.hpp"
#include <vector>

#define TEXT_MAX_LENGTH	128

class CSign final : public CB2Polygon
{
public:
	CSign(const sf::Vector2f &pos, const sf::Vector2f &size, const char *pText) noexcept;
	~CSign() noexcept;

	virtual void tick() noexcept final;
	virtual void onSensorIn(CEntity *pEntity) noexcept final;
	virtual void onSensorOut(CEntity *pEntity) noexcept final;

protected:
	b2Body *m_pBody;
	char m_aText[TEXT_MAX_LENGTH];

private:
	static const CB2BodyInfo ms_BodyInfo;
};

#endif
