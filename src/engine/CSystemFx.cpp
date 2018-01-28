/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CSystemFx.hpp>
#include <engine/CSystem.hpp>
#include <SFML/Graphics.hpp>
#include "CGame.hpp"

/** PARTICLES **/
CParticle::CParticle(sf::BlendMode blendMode, int render, bool light, int shader) noexcept
{
    m_Pos = m_Dir = m_SizeInit = m_SizeEnd = { 0.0f, 0.0f };
    m_ColorInit = m_ColorEnd = { 255, 255, 255, 255 };
    m_Vel = 0.0f;
    m_VelRot = 0.0f;
    m_Render = render;
    m_ModeText = false;
    m_DoMove = true;
    m_BlendMode = blendMode;
    m_Timer = ups::timeGet();
    m_Disp = VECTOR_ZERO;
    m_Offset = VECTOR_ZERO;
    m_Duration = 1.0f;
    m_pLight = nullptr;
    m_pTarget = nullptr;
    m_Animated = false;
    m_AnimSize = { 0, 0 };
    m_Light = light;
    m_FixedPos = false;
    m_OnlyLight = false;

    CGame *pGame = CGame::getInstance();
    m_String.setFont(pGame->Client()->Assets().getDefaultFont());

    m_ShaderId = shader;
	m_pShader = (m_ShaderId != -1)?pGame->Client()->Assets().getShader(m_ShaderId):nullptr;
}
CParticle::~CParticle() noexcept
{
	if (m_pLight)
		m_pLight->destroy();

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CParticle", "Deleted");
	#endif
}

void CParticle::destroy() noexcept
{
	m_Duration = 0.0f;
}

void CParticle::update() noexcept
{
	CGame *pGame = CGame::getInstance();
    if (pGame->Client()->m_Paused)
    {
    	// TODO: Restar al tiempo pasado el tiempo que se esta pausado
    	return;
    }

	const float elapsedSeconds = (float)(ups::timeGet()-m_Timer)/ups::timeFreq();
    const sf::Vector2f sizeStep(((m_SizeEnd.x-m_SizeInit.x)/m_Duration)*elapsedSeconds, ((m_SizeEnd.y-m_SizeInit.y)/m_Duration)*elapsedSeconds);
    const sf::Vector3f colorStep(((m_ColorEnd.r-m_ColorInit.r)/m_Duration)*elapsedSeconds, ((m_ColorEnd.g-m_ColorInit.g)/m_Duration)*elapsedSeconds, ((m_ColorEnd.b-m_ColorInit.b)/m_Duration)*elapsedSeconds);
    const float colorAlphaStep = ((m_ColorEnd.a-m_ColorInit.a)/m_Duration)*elapsedSeconds;

    if (m_pTarget)
    {
    	if (m_pTarget->isToDelete())
    		m_pTarget = nullptr;
    	else if (m_pTarget->getShape())
    	{
    		m_Pos = m_pTarget->getShape()->getPosition();
    	}
    }

	m_Disp += (m_Dir*m_Vel)*elapsedSeconds;
	m_Pos += m_Disp + m_Offset;

    if (m_Light)
    {
    	static const float reduFactor = 0.03f;
        if (!m_pLight)
        {
    		CSystemLight *pLightEngine = pGame->Client()->getSystem<CSystemLight>();
    		m_pLight = pLightEngine->createPoint(m_Pos, m_SizeInit*reduFactor, m_ColorInit);
    		m_pLight->m_Render = m_Render;
    		m_pLight->m_Scale = sf::Vector2f(sizeStep.x*reduFactor, sizeStep.y*reduFactor);
    		m_pLight->m_Color = sf::Color(m_ColorInit.r+colorStep.x, m_ColorInit.g+colorStep.y, m_ColorInit.b+colorStep.z, m_ColorInit.a+colorAlphaStep);
        }
        else
        {
        	m_pLight->m_Position = m_Pos+(m_FixedPos?pGame->Client()->getView().getCenter():sf::Vector2f(0.0f,0.0f));
        	m_pLight->m_Scale = sf::Vector2f(sizeStep.x*reduFactor, sizeStep.y*reduFactor);
        	m_pLight->m_Color = sf::Color(m_ColorInit.r+colorStep.x, m_ColorInit.g+colorStep.y, m_ColorInit.b+colorStep.z, m_ColorInit.a+colorAlphaStep);
        }
    }

    if (!m_OnlyLight)
    {
		if (!m_ModeText)
		{
			if (!pGame->Client()->m_Paused)
				m_Shape.setRotation(m_Shape.getRotation()+m_VelRot);
			if (m_Animated)
			{
				const int animStep = ((m_AnimSize.x*m_AnimSize.y)/m_Duration)*elapsedSeconds;
				const sf::Vector2i animStepPos = { animStep%m_AnimSize.x, animStep/m_AnimSize.x };
				const sf::Vector2i tileSize = { (int)(m_Shape.getTexture()->getSize().x/m_AnimSize.x), (int)(m_Shape.getTexture()->getSize().y/m_AnimSize.y) };
				m_Shape.setTextureRect(sf::IntRect(animStepPos.x*tileSize.x, animStepPos.y*tileSize.y, tileSize.x, tileSize.y));
			}
			m_Shape.setPosition(m_Pos+(m_FixedPos?pGame->Client()->getView().getCenter():sf::Vector2f(0.0f,0.0f)));
			m_Shape.setSize(m_SizeInit+sizeStep);
			m_Shape.setOrigin((m_SizeInit.x+sizeStep.x)/2.0f, (m_SizeInit.y+sizeStep.y)/2.0f);
			m_Shape.setFillColor(sf::Color(m_ColorInit.r+colorStep.x, m_ColorInit.g+colorStep.y, m_ColorInit.b+colorStep.z, m_ColorInit.a+colorAlphaStep));
		}
		else
		{
			if (!pGame->Client()->m_Paused)
				m_String.setRotation(m_String.getRotation()+m_VelRot);
			m_String.setPosition(m_Pos+(m_FixedPos?pGame->Client()->getView().getCenter():sf::Vector2f(0.0f,0.0f)));
			m_String.setScale((m_SizeInit.x+sizeStep.x)/m_SizeInit.x, (m_SizeInit.y+sizeStep.y)/m_SizeInit.y);
			m_String.setOrigin(m_String.getLocalBounds().width/2.0f, m_String.getLocalBounds().height/2.0f);
			m_String.setFillColor(sf::Color(m_ColorInit.r+colorStep.x, m_ColorInit.g+colorStep.y, m_ColorInit.b+colorStep.z, m_ColorInit.a+colorAlphaStep));
		}
    }
}

void CParticle::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (m_OnlyLight)
		return;

	states.blendMode = m_BlendMode;
	if (m_pShader)
	{
		if (m_ShaderId == CAssetManager::SHADER_CHROMATIC_ABERRATION)
		{
			m_pShader->setUniform("offc", sf::Vector3f(0.001f, -0.0012f, 0.0015f));
			m_pShader->setUniform("texture", sf::Shader::CurrentTexture);
		} else if (m_ShaderId == CAssetManager::SHADER_BLUR)
		{
			m_pShader->setUniform("blur_radius_x", 0.065f);
			m_pShader->setUniform("blur_radius_y", 0.065f);
			m_pShader->setUniform("texture", sf::Shader::CurrentTexture);
		}
		states.shader = m_pShader;
	}
	if (!m_ModeText)
		target.draw(m_Shape, states);
	else
		target.draw(m_String, states);
}

/**  ENGINE **/
CSystemFx::CSystemFx() noexcept
{
	m_Add100Hz = false;
	m_Add50Hz = false;
}
CSystemFx::~CSystemFx() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSystemFx", "Deleted");
	#endif
}

bool CSystemFx::init(class CGameClient *pGameClient) noexcept
{
	CSystem::init(pGameClient);
	return true;
}

void CSystemFx::update(float deltaTime) noexcept
{
	static const sf::Int64 time100Hz = ups::timeFreq()/100;
	static const sf::Int64 time50Hz = ups::timeFreq()/50;

	if(m_Timer100Hz.getElapsedTime().asMicroseconds() > time100Hz)
	{
		m_Add100Hz = true;
		m_Timer100Hz.restart();
	}
	else
		m_Add100Hz = false;

	if(m_Timer50Hz.getElapsedTime().asMicroseconds() > time50Hz)
	{
		m_Add50Hz = true;
		m_Timer50Hz.restart();
	}
	else
		m_Add50Hz = false;
}

void CSystemFx::createSpirit(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	for (int i=0; i<6; i++)
	{
		CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
		pParticle->m_Pos = worldPos;
		pParticle->m_Light = true;
		pParticle->m_SizeInit = sf::Vector2f(2.0f, 2.0f);
		pParticle->m_SizeEnd = sf::Vector2f(6.0f, 6.0f);
		pParticle->m_ColorInit = sf::Color::White;
		pParticle->m_ColorEnd = sf::Color::Blue;
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 0.8f;
		pParticle->m_VelRot = 0.15f;
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
		pParticle->m_Vel = upm::floatRand(0.5f, 1.0f);
		Client()->Controller()->Context()->addParticle(pParticle);
	}
}

void CSystemFx::createSpiritTrail(const sf::Vector2f &worldPos) noexcept
{
	if (Client()->isClipped(worldPos, 128.0f))
		return;

	for (int i=0; i<6; i++)
	{
		CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_BACK);
		pParticle->m_Pos = worldPos;
		pParticle->m_Light = true;
		pParticle->m_SizeInit = sf::Vector2f(4.0f, 4.0f);
		pParticle->m_SizeEnd = sf::Vector2f(1.0f, 1.0f);
		pParticle->m_ColorInit = sf::Color::Blue;
		pParticle->m_ColorEnd = sf::Color::White;
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 0.65f;
		pParticle->m_VelRot = 0;
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
		pParticle->m_Vel = upm::floatRand(0.0008f, 0.001f);
		Client()->Controller()->Context()->addParticle(pParticle);
	}
}

void CSystemFx::createImpactSparkMetal(const sf::Vector2f &worldPos) noexcept
{
	for (int i=0; i<6; i++)
	{
		CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
		pParticle->m_Pos = worldPos;
		pParticle->m_SizeInit = sf::Vector2f(32.0f, 32.0f);
		pParticle->m_SizeEnd = VECTOR_ZERO;
		pParticle->m_ColorInit = sf::Color::White;
		pParticle->m_ColorEnd = sf::Color::White;
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 0.15f;
		pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_BULLET_SPARK));
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
		pParticle->m_Vel = upm::floatRand(60.5f, 80.5f);
		Client()->Controller()->Context()->addParticle(pParticle);
	}

	createPointLight(worldPos, 0.05f, 2.2f, sf::Color(255, 210, 90, 90), sf::Color(232, 171, 8, 0));
}

void CSystemFx::createBloodSpark(const sf::Vector2f &worldPos, float duration) noexcept
{
	if (!m_Add50Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(2.0f, 2.0f);
	pParticle->m_SizeEnd = sf::Vector2f(4.0f, 4.0f);
	pParticle->m_ColorInit = sf::Color::Red;
	pParticle->m_ColorEnd = sf::Color::Red;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = duration;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_BULLET_SPARK));
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createBlood(const sf::Vector2f &worldPos) noexcept
{
	if (Client()->isClipped(worldPos, 128.0f))
		return;

	for (int i=0; i<2; i++)
	{
		CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_BACK);
		pParticle->m_Pos = worldPos;
		pParticle->m_SizeInit = sf::Vector2f(20.0f, 20.0f);
		pParticle->m_SizeEnd = sf::Vector2f(40.0f, 40.0f);
		pParticle->m_ColorInit = sf::Color::Red;
		pParticle->m_ColorInit.a = 200;
		pParticle->m_ColorEnd = sf::Color::Red;
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 15.0f;
		pParticle->m_Shape.setRotation(upm::floatRand(0.0f, 360.0f));
		pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
		Client()->Controller()->Context()->addParticle(pParticle);
	}
}

void CSystemFx::createFireBall(CEntity *pTarget, const sf::Vector2f &offSet) noexcept
{
	if (Client()->isClipped(pTarget->getShape()->getPosition()+offSet, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT, true, CAssetManager::SHADER_BLUR);
	pParticle->m_pTarget = pTarget;
	pParticle->m_SizeInit = sf::Vector2f(10.0f, 10.0f);
	pParticle->m_SizeEnd = sf::Vector2f(32.0f, 32.0f);
	pParticle->m_ColorInit = sf::Color(10, 75, 205);
	pParticle->m_ColorEnd = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd.a = 35;
	pParticle->m_Offset = offSet;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Duration = 0.75f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createFireTrailLarge(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(4.0f, 4.0f);
	pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_ColorInit = sf::Color(125, 125, 125, 200);
	pParticle->m_ColorEnd = sf::Color::Black;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
	pParticle->m_Duration = upm::floatRand(0.1f, 0.55f);
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);

	pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_Light = true;
	pParticle->m_SizeInit = sf::Vector2f(4.0f, 4.0f);
	pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_ColorInit = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd = sf::Color(251, 216, 4);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
	pParticle->m_Duration = upm::floatRand(0.05f, 0.25f);
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createFireTrailSmall(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(8.0f, 8.0f);
	pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_ColorInit = sf::Color(125, 125, 125, 200);
	pParticle->m_ColorEnd = sf::Color::Black;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
	pParticle->m_Duration = upm::floatRand(0.05f, 0.1f);
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);

	pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_Light = true;
	pParticle->m_SizeInit = sf::Vector2f(8.0f, 8.0f);
	pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_ColorInit = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd = sf::Color(251, 216, 4);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
	pParticle->m_Duration = upm::floatRand(0.001f, 0.05f);
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createSmokeCarDamaged(const sf::Vector2f &worldPos, bool fire) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = VECTOR_ZERO;
	pParticle->m_SizeEnd = sf::Vector2f(42.0f, 42.0f);
	pParticle->m_ColorInit = sf::Color(125, 125, 125, 200);
	pParticle->m_ColorEnd = sf::Color::Black;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
	pParticle->m_Duration = 1.0f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);

	if (fire)
	{
		pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
		pParticle->m_Pos = worldPos;
		pParticle->m_Light = true;
		pParticle->m_SizeInit = sf::Vector2f(4.0f, 4.0f);
		pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
		pParticle->m_ColorInit = sf::Color(251, 104, 4);
		pParticle->m_ColorEnd = sf::Color(251, 216, 4);
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
		pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
		pParticle->m_Duration = 0.45f;
		pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
		Client()->Controller()->Context()->addParticle(pParticle);
	}
}

void CSystemFx::createPoints(const sf::Vector2f &worldPos, int points) noexcept
{
	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_ModeText = true;
	char buff[5];
	if (points >= 0) snprintf(buff, sizeof(buff), "+%d", points);
	else snprintf(buff, sizeof(buff), "%d", points);
	pParticle->m_String.setString(buff);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(18.0f, 18.0f);
	pParticle->m_SizeEnd = sf::Vector2f(32.0f, 32.0f);
	pParticle->m_ColorInit = sf::Color::Yellow;
	pParticle->m_ColorEnd = (points>=0)?sf::Color::Green:sf::Color::Red;
	pParticle->m_ColorEnd.a = 85;
	pParticle->m_Duration = 0.4f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.000001f, 0.00001f);
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createSmokeImpact(const sf::Vector2f &worldPos) noexcept
{
	if (Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_SizeEnd = sf::Vector2f(36.0f, 36.0f);
	pParticle->m_ColorInit = sf::Color(200, 200, 200, 200);
	pParticle->m_ColorEnd = sf::Color::White;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-0.3f:0.3f;
	pParticle->m_Duration = upm::floatRand(0.5f, 1.0f);
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}









void CSystemFx::createSpitFireCarDamaged(const sf::Vector2f &worldPos, const sf::Vector2f &dir) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = VECTOR_ZERO;
	pParticle->m_SizeEnd = sf::Vector2f(24.0f, 24.0f);
	pParticle->m_ColorInit = sf::Color(125, 125, 125, 200);
	pParticle->m_ColorEnd = sf::Color::Black;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = dir;
	pParticle->m_Vel = upm::floatRand(0.2f, 1.0f);
	pParticle->m_Duration = 1.0f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);

	pParticle = new CParticle(sf::BlendAdd, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_Light = true;
	pParticle->m_SizeInit = sf::Vector2f(4.0f, 4.0f);
	pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_ColorInit = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd = sf::Color(251, 216, 4);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = dir;
	pParticle->m_Vel = upm::floatRand(0.2f, 1.0f);
	pParticle->m_Duration = 0.45f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createExplosionCar(const sf::Vector2f &worldPos, bool ring) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle;

	if (ring)
	{
		pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
		pParticle->m_Pos = worldPos;
		pParticle->m_SizeInit = sf::Vector2f(26.0f, 26.0f);
		pParticle->m_SizeEnd = sf::Vector2f(95.0f, 95.0f);
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 1.7f;
		pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_EXPLOSION_RING));
		Client()->Controller()->Context()->addParticle(pParticle);

		pParticle = new CParticle();
		pParticle->m_Pos = worldPos;
		pParticle->m_SizeInit = sf::Vector2f(26.0f, 26.0f);
		pParticle->m_SizeEnd = sf::Vector2f(175.0f, 175.0f);
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 1.7f;
		pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_EXPLOSION_RING));
		Client()->Controller()->Context()->addParticle(pParticle);
	}

	pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(32.0f, 32.0f);
	pParticle->m_SizeEnd = sf::Vector2f(172.0f, 172.0f);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 2.2f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_EXPLOSION_ANIM));
	pParticle->m_Vel = 0.0f;
	pParticle->m_VelRot = 2.75;
	pParticle->m_Animated = true;
	pParticle->m_AnimSize = { 4, 4 };
	Client()->Controller()->Context()->addParticle(pParticle);

	pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(15.0f, 15.0f);
	pParticle->m_SizeEnd = sf::Vector2f(175.0f, 175.0f);
	pParticle->m_ColorInit = sf::Color(125, 125, 125, 120);
	pParticle->m_ColorEnd = sf::Color::Black;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Duration = 4.0f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);

	CParticle *pParticleLight = new CParticle(sf::BlendAdd, RENDER_FRONT, true);
	pParticleLight->m_Pos = worldPos;
	pParticle->m_Light = true;
	pParticleLight->m_SizeInit = sf::Vector2f(1.0f, 1.0f);
	pParticleLight->m_SizeEnd = sf::Vector2f(2.0f, 2.0f);
	pParticleLight->m_ColorInit = sf::Color(255, 210, 90, 200);
	pParticleLight->m_ColorEnd = sf::Color(232, 171, 8);
	pParticleLight->m_ColorEnd.a = 0;
	pParticleLight->m_Duration = 4.0f;
	Client()->Controller()->Context()->addParticle(pParticleLight);
}

void CSystemFx::createExplosionRocket(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle;

	pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_SizeEnd = sf::Vector2f(64.0f, 64.0f);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 2.2f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_EXPLOSION_ANIM));
	pParticle->m_Vel = 0;
	pParticle->m_VelRot = 2.75;
	pParticle->m_Animated = true;
	pParticle->m_AnimSize = { 4, 4 };
	Client()->Controller()->Context()->addParticle(pParticle);

	pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(15.0f, 15.0f);
	pParticle->m_SizeEnd = sf::Vector2f(95.0f, 95.0f);
	pParticle->m_ColorInit = sf::Color(125, 125, 125, 200);
	pParticle->m_ColorEnd = sf::Color::Black;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Duration = 2.0f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);

	CParticle *pParticleLight = new CParticle(sf::BlendAdd, RENDER_FRONT, true);
	pParticle->m_Light = true;
	pParticleLight->m_Pos = worldPos;
	pParticleLight->m_SizeInit = sf::Vector2f(0.2f, 0.2f);
	pParticleLight->m_ColorInit = sf::Color(255, 210, 90, 100);
	pParticleLight->m_ColorEnd = sf::Color(232, 171, 8);
	pParticleLight->m_SizeEnd = sf::Vector2f(0.5f, 0.5f);
	pParticleLight->m_ColorEnd.a = 0;
	pParticleLight->m_Duration = 1.2f;
	Client()->Controller()->Context()->addParticle(pParticleLight);
}

void CSystemFx::createPointLight(const sf::Vector2f &worldPos, float radius, float duration, sf::Color colorI, sf::Color colorE) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticleLight = new CParticle(sf::BlendAdd, RENDER_BACK, true);
	pParticleLight->m_Pos = worldPos;
	pParticleLight->m_ColorInit = colorI;
	pParticleLight->m_ColorEnd = colorE;
	pParticleLight->m_SizeInit = sf::Vector2f(radius, radius);
	pParticleLight->m_SizeEnd = sf::Vector2f(radius, radius);
	pParticleLight->m_Duration = duration;
	pParticleLight->m_OnlyLight = true;
	Client()->Controller()->Context()->addParticle(pParticleLight);
}

void CSystemFx::createMarkWheel(const sf::Vector2f &worldPos, float angle) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = pParticle->m_SizeEnd = sf::Vector2f(5.0f, 5.0f);
	pParticle->m_ColorInit = sf::Color(45, 45, 45, 200);
	pParticle->m_ColorEnd = sf::Color(45, 45, 45, 0);
	pParticle->m_Duration = 20.0f;
	pParticle->m_Shape.setRotation(angle);
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createWaterSplash(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add50Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = VECTOR_ZERO;
	const float sizeE = 5.0f+(rand()%10);
	pParticle->m_SizeEnd = sf::Vector2f(sizeE, sizeE);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 0.7f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_EXPLOSION_RING));
	pParticle->m_Vel = 0.0f;
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createRain(const sf::Vector2f &worldPos, float rainVel) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(2.0f, 2.0f);
	pParticle->m_SizeEnd = sf::Vector2f(1.0f, 1.0f);
	pParticle->m_ColorInit = pParticle->m_ColorEnd = sf::Color(230,120,255,220);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 1.0f;
	pParticle->m_Vel = rainVel;
	pParticle->m_Dir = sf::Vector2f(1.0f,1.0f);
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_POINT_LIGHT));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createStorm() noexcept
{
	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_Pos = sf::Vector2f(0.0f,0.0f);
	pParticle->m_FixedPos = true;
	const sf::Vector2f size = sf::Vector2f(Client()->getView().getSize().x, Client()->getView().getSize().y);
	pParticle->m_SizeInit = size;
	pParticle->m_SizeEnd = size;
	pParticle->m_ColorInit.a = 80;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 0.8f;
	pParticle->m_Shape.setFillColor(sf::Color::Red);
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createSnow(const sf::Vector2f &worldPos, float snowVel) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(2.0f, 2.0f);
	pParticle->m_SizeEnd = sf::Vector2f(1.0f, 1.0f);
	pParticle->m_ColorInit = pParticle->m_ColorEnd = sf::Color(255,255,255,220);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 2.0f;
	pParticle->m_Vel = snowVel;
	pParticle->m_Dir = sf::Vector2f(1.0f,1.0f);
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_POINT_LIGHT));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createSnowSplash(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	const float sizeI = 1.0f+(rand()%2);
	pParticle->m_SizeInit = pParticle->m_SizeEnd = sf::Vector2f(sizeI,sizeI);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 1.7f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_POINT_LIGHT));
	pParticle->m_Vel = 0.0f;
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createSnowTrail(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add100Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = pParticle->m_SizeEnd = sf::Vector2f(3.0f,3.0f);
	pParticle->m_ColorInit = pParticle->m_ColorEnd = sf::Color(120,120,120,120);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 12.0f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_POINT_LIGHT));
	pParticle->m_Vel = 0.0f;
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createGasolineFire(const sf::Vector2f &worldPos) noexcept
{
	if (!m_Add50Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_Light = true;
	pParticle->m_SizeInit = sf::Vector2f(2.0f, 2.0f);
	pParticle->m_SizeEnd = sf::Vector2f(8.0f, 8.0f);
	pParticle->m_ColorInit = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd = sf::Color(251, 216, 4);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.05f, 0.2f);
	pParticle->m_Duration = 1.45f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createHighlightItem(const sf::Vector2f &worldPos, sf::Color color, int rederZone) noexcept
{
	if (!m_Add50Hz || Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAdd, rederZone);
	pParticle->m_Pos = worldPos;
	pParticle->m_Light = true;
	pParticle->m_SizeInit = sf::Vector2f(2.0f, 2.0f);
	pParticle->m_SizeEnd = VECTOR_ZERO;
	pParticle->m_ColorInit = color;
	pParticle->m_ColorEnd = sf::Color(color.r+40, color.g+30, color.b+20);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-0.6f, 0.6f), -1.0f);
	pParticle->m_Vel = upm::floatRand(0.1f, 0.4f);
	pParticle->m_Duration = 2.45f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}

void CSystemFx::createHighlightItemSimple(const sf::Vector2f &worldPos, sf::Color color, float size, int rederZone) noexcept
{
	if (Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAdd, rederZone);
	pParticle->m_Pos = worldPos;
	pParticle->m_Light = true;
	pParticle->m_SizeInit = sf::Vector2f(size, size);
	pParticle->m_SizeEnd = sf::Vector2f(size+2.0f, size+2.0f);
	pParticle->m_ColorInit = color;
	pParticle->m_ColorEnd = sf::Color(color.r+40, color.g+30, color.b+20);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Duration = 0.0f;
	pParticle->m_Shape.setTexture(Client()->Assets().getTexture(CAssetManager::TEXTURE_SMOKE_WHITE));
	Client()->Controller()->Context()->addParticle(pParticle);
}
