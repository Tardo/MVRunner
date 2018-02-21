/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CLIGHT
#define H_ENTITY_CLIGHT

#include <game/CEntity.hpp>

class CLight final : public CEntity
{
	friend class CLightRender;
public:
    CLight(sf::Texture *pTexture, const sf::Vector2f &nposition, float angle, const sf::Vector2f &nscale, const sf::Color &ncolor, bool alwaysOn, float blink, float variationSize, int render = RENDER_FRONT) noexcept;
    ~CLight() noexcept;

    virtual void tick() noexcept final;

    sf::FloatRect getLocalBounds() const { return m_Sprite.getLocalBounds(); }
    const sf::Sprite& getSprite() const { return m_Sprite; }

    sf::Vector2f m_Position;
    sf::Vector2f m_Scale;
    sf::Vector2f m_Origin;
    sf::Color m_Color;
    float m_Blink;
    float m_Degrees;
    int m_Render;
    bool m_Active;
    bool m_AlwaysOn;

private:
    bool m_Blinked;
    sf::Int64 m_TimerBlink;
    sf::Sprite m_Sprite;
    float m_VariationSize;
};

#endif
