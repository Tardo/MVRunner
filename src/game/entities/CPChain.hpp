/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CANTENNA
#define H_ENTITY_CANTENNA

#include "primitives/CB2Polygon.hpp"
#include <vector>

class CPChain final : public CEntity
{
	static const sf::Vector2f SLAVE_SIZE;

public:
	CPChain(const sf::Vector2f &pos, const sf::Color &color = sf::Color::White, int textureId = -1) noexcept;
	~CPChain() noexcept;

	virtual void tick() noexcept final;

private:
	static const CB2BodyInfo ms_BodyInfo;
	static const CB2BodyInfo ms_BodyInfoBase;
	int m_TextureId;

	CB2Polygon *m_apPolygons[NUM_SLAVES];
};

#endif
