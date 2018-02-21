/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENTITY_CPARTICLE
#define H_ENTITY_CPARTICLE

#include <game/CEntity.hpp>

class CParticle final : public CEntity
{
public:
	enum
	{
		VEL_LINEAL=0,
		VEL_INCREASE,
	};

    CParticle(sf::BlendMode blendMode = sf::BlendAlpha, int render = RENDER_FRONT, bool luminance = false, int shader = -1) noexcept;
    virtual ~CParticle() noexcept;

    virtual void tick() noexcept final;

    sf::Vector2f m_Pos;
    sf::Vector2f m_Dir;
    sf::Vector2f m_SizeInit;
    sf::Vector2f m_SizeEnd;
    sf::Vector2i m_AnimSize;
    sf::Color m_ColorInit;
    sf::Color m_ColorEnd;
    std::string m_String;
    sf::BlendMode m_BlendMode;
    float m_Vel;
    float m_VelRot;
    float m_Rotation;
    float m_Duration;
    int m_Render;
    bool m_ModeText;
    bool m_DoMove;
    bool m_Animated;
    bool m_FixedPos;
    bool m_Luminance;
    int m_VelType;
    int m_TextId;
    sf::Int64 m_Timer;
    class CEntity *m_pTarget;
    sf::Vector2f m_Offset;

private:
    sf::Vector2f m_Disp;
};

#endif
