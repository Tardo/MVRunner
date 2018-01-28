/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_SYSTEM_LIGHT
#define H_ENGINE_SYSTEM_LIGHT

#include <SFML/Graphics.hpp>
#include <engine/CSystem.hpp>
#include <game/CEntity.hpp>
#include <list>

class CLight final
{
	friend class CSystemLight;
public:
    CLight(int textId, const sf::Vector2f &nposition, float angle, const sf::Vector2f &nscale, const sf::Color &ncolor, bool alwaysOn, float blink, float variationSize, int render = RENDER_FRONT) noexcept;
    ~CLight() noexcept;

    void destroy() noexcept { m_ToDelete = true; }

    sf::FloatRect getLocalBounds() const { return m_Sprite.getLocalBounds(); }

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
    int m_TextID;
    int m_MaskTextID;
    bool m_ToDelete;
    bool m_Blinked;
    sf::Int64 m_TimerBlink;
    sf::Sprite m_Sprite;
    float m_VariationSize;
};

class CSystemLight final : public CSystem
{
public:
	CSystemLight() noexcept;
	virtual ~CSystemLight() noexcept;

	virtual bool init(class CGameClient *pGameClient) noexcept final;
	virtual void update(float deltaTime) noexcept final;

	const sf::Sprite& getLightmap(const sf::View &camera) noexcept;
	const std::list<CLight*>& getLights() const noexcept { return m_vpLights; }
	CLight* createPoint(const sf::Vector2f &worldPos, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;
	CLight* createSpot(const sf::Vector2f &worldPos, float angle, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;
	CLight* createEmissive(const sf::Vector2f &worldPos, float angle, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;
	CLight* createCustom(int textId, const sf::Vector2f &worldPos, const sf::Vector2f &origin, float angle, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn = false, float blink = 0.0f, float variationSize = 0.0f) noexcept;

	std::vector<CLight*> getNearLights(const sf::Vector2f &worldPos) noexcept;

	sf::Color getTimeColor() noexcept;
	sf::Vector2f getShadowOffset() const noexcept { return m_ShadowOffset; }
	sf::Color getShadowColor() const noexcept { return m_ShadowColor; }

private:
	std::list<CLight*> m_vpLights;
	sf::Vector2f m_CamOffset;
	sf::Sprite m_Light;
	sf::Color m_ColorClear;
	sf::Color m_ShadowColor;
	sf::Vector2f m_ShadowOffset;

	sf::RenderTexture m_LightMapTexture;
	sf::Sprite m_Lightmap;
	class CSystemWeather *m_pWeatherEngine;
};

#endif
