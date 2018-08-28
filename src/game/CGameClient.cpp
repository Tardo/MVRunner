/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <Box2D/Box2D.h>
#include <base/system.hpp>
#include <game/controllers/CControllerMain.hpp>
#include <game/controllers/CControllerMenu.hpp>
#include <game/CContext.hpp>
#include <game/CGameClient.hpp>
#include <engine/CLocalization.hpp>
#include <cstdlib>
#include <ctime>
#include <cmath>


CGameClient::CGameClient() noexcept
: sf::RenderWindow(sf::VideoMode(g_Config.m_ScreenWidth, g_Config.m_ScreenHeight), "", g_Config.m_FullScreen?sf::Style::Fullscreen:sf::Style::Close),
  m_AssetManager(&m_Zpg),
  m_Camera(this),
  m_MapRenderBack(this, RENDER_BACK),
  m_MapRenderFront(this, RENDER_FRONT),
  m_Menus(this),
  m_UI(this),
  m_PlayerRender(this),
  m_ItemRender(this),
  m_FluidRender(this),
  m_ParticleRenderBack(this, RENDER_BACK),
  m_ParticleRenderFront(this, RENDER_FRONT),
  m_ParticleRenderForeground(this, RENDER_FOREGROUND),
  m_Controls(this),
  m_DebuggerRender(this),
  m_LightRender(this, RENDER_FRONT)
{
	setFramerateLimit(60);
	m_pGameController = nullptr;
	m_Debug = false;
	m_Paused = false;
	m_FPS = 0;
	m_ViewHud = getDefaultView();
	m_DeltaTime = 0.0f;
	m_MinFPS = 9999;
	m_RenderMode = RENDER_MODE_NORMAL;

	m_TimerGame = ups::timeGet();
	m_Add100Hz = false;
	m_Add50Hz = false;
}
CGameClient::~CGameClient() noexcept
{
	reset();
	m_Zpg.close();

	#ifdef DEBUG_DESTRUCTORS
    ups::msgDebug("CGameClient", "Deleted");
	#endif
}


void CGameClient::run() noexcept
{
	ups::msgDebug("CGameClient", "Initializing game...");
	if (!init())
	{
		ups::msgDebug("CGameClient", "Error initializing game... closing!");
		close();
	}
	else
		ups::msgDebug("CGameClient", "Game initialized successfully :)");

	sf::Int64 TimerFPS = ups::timeGet();
	sf::Int64 LTime=0, CTime=0;
    while (isOpen())
    {
    	LTime = CTime;
    	CTime = ups::timeGet()-m_TimerGame;
    	m_DeltaTime = (CTime-LTime)/(float)ups::timeFreq();

        sf::Event event;
        while (pollEvent(event))
        {
        	if (event.type == sf::Event::Closed)
        		close();
        	else
        		m_Controls.processEvent(event);
        }

        // Timing Control Flags
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

    	// Magic!
    	doUpdate();
        doRender();

        // FPS
        static unsigned int fpsCounter = 0;
        if(ups::timeGet()-TimerFPS >= ups::timeFreq())
        {
        	m_FPS = fpsCounter;
        	if (m_FPS < m_MinFPS)
        		m_MinFPS = m_FPS;
        	fpsCounter = 0;
        	TimerFPS = ups::timeGet();
        }
        else
        {
        	++fpsCounter;
        }
    }
}

void CGameClient::doUpdate()
{
	if (Controller() && Controller()->Context()->getPlayer()->getCharacter())
	{
		const sf::Vector2f &pos = CSystemBox2D::b2ToSf(Controller()->Context()->getPlayer()->getCharacter()->getBody()->GetPosition());
		m_SystemSound.setListenerPosition(pos);
	}

    // Update Systems
	std::deque<ISystem*>::const_iterator itSys = m_vpSystems.cbegin();
	while (itSys != m_vpSystems.cend())
    	(*itSys++)->update(m_DeltaTime);

	// Update Camera
	m_Camera.update(m_DeltaTime);

    // Update Controller
    if (Controller())
    	Controller()->tick();
}

void CGameClient::doRender()
{
    // Render Components
    // Normal Mode
	const bool mapLoaded = Controller()->Context()->Map().isMapLoaded();
    m_RenderPhaseTexture.clear(mapLoaded?CMap::tmxToSf(Controller()->Context()->Map().GetBackgroundColor()):sf::Color::Black);
    renderComponentsPhase(RENDER_MODE_NORMAL);
    draw(m_RenderPhase);

	// Lighting Mode
    m_RenderPhaseTexture.clear(sf::Color::Black);
    renderComponentsPhase(RENDER_MODE_LIGHTING);
    draw(m_RenderPhase, sf::BlendAdd);

    // Bloom Effect
	sf::Shader *pShader = Assets().getShader(CAssetManager::SHADER_BLOOM);
	if (pShader)
	{
		pShader->setUniform("iChannel0", sf::Shader::CurrentTexture);
		pShader->setUniform("iResolution", sf::Vector2f(m_RenderPhase.getTexture()->getSize().x, m_RenderPhase.getTexture()->getSize().y));
		static const int sIters = 8;
		for (int i=0; i<sIters; ++i)
		{
			pShader->setUniform("direction", (i%2 == 0)?sf::Vector2f((sIters-i-1)*upm::floatRand(0.6f, 0.8f), 0):sf::Vector2f(0, (sIters-i-1)*upm::floatRand(0.6f, 0.8f)));
			m_RenderPhaseTexture.draw(m_RenderPhase, pShader);
			m_RenderPhaseTexture.display();
		}

		draw(m_RenderPhase, sf::BlendAdd);
	}

    display();
}

void CGameClient::renderComponentsPhase(int mode)
{
    setRenderMode(mode);
    std::deque<CComponent*>::const_iterator itComp = m_vpComponents.cbegin();
	while (itComp != m_vpComponents.cend())
		m_RenderPhaseTexture.draw(*(*itComp++));
	m_RenderPhaseTexture.display();
}

void CGameClient::reset() noexcept
{
	if (m_pGameController != nullptr)
		delete m_pGameController;
	m_pGameController = nullptr;
}

bool CGameClient::init() noexcept
{
	setVerticalSyncEnabled(g_Config.m_VSync);
	setMouseCursorVisible(g_Config.m_CursorShow);
	setMouseCursorGrabbed(g_Config.m_CursorGrab);

	m_RenderPhaseTexture.create(g_Config.m_ScreenWidth, g_Config.m_ScreenHeight);
	m_RenderPhase.setTexture(m_RenderPhaseTexture.getTexture(), true);

	if (!sf::Shader::isAvailable())
		g_Config.m_UseShaders = false;

	m_Zpg.open("assets.zpg");

	if (!g_l10n.load(g_Config.m_Lang, &m_Zpg))
		ups::msgDebug("CGame", "Can't found selected language! using english...");

	char title[128];
	snprintf(title, sizeof(title), "MVRunner");
	setTitle(title);

	/** LOADING ASSETS **/
	sf::Thread thread(&CAssetManager::load, &m_AssetManager);
	thread.launch();

	sf::FloatRect rectArea;
	getViewportGlobalBounds(&rectArea, getHudView());
	while (!m_AssetManager.isLoaded())
	{
		if (m_AssetManager.hasErrors())
		{
			ups::msgDebug("CGameClient", "ERROR: Can't load assets!");
			return false;
		}

		clear(sf::Color::Black);
		setView(getHudView());

		sf::Text text;
		text.setFont(Assets().getDefaultFont());
		text.setCharacterSize(94);
		text.setFillColor(sf::Color::White);

		text.setString(_("LOADING"));
		float textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-textW/2.0f, rectArea.height/2-160.0f);
		draw(text);

		const float boxSize = rectArea.width/2.0f;
		sf::RectangleShape box(sf::Vector2f(boxSize, 100.0f));
		box.setFillColor(sf::Color::Transparent);
		box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(4.0f);
		box.setPosition(boxSize-boxSize/2.0f, rectArea.height/2-100.0f/2.0f);
		draw(box);

		const float ww = (m_AssetManager.getLoadedNum()/(float)(CAssetManager::NUM_TOTAL))*boxSize;
		box.setSize(sf::Vector2f(ww-8.0f, 100.0f-8.0f));
		box.setFillColor(sf::Color::White);
		box.setOutlineThickness(0.0f);
		box.setPosition(boxSize-boxSize/2.0f+4.0f, rectArea.height/2-100.0f/2.0f+4.0f);
		draw(box);

		text.setCharacterSize(42);
		text.setString(m_AssetManager.getCurrentLoadAssetPath());
		textW = text.getLocalBounds().width;
		text.setPosition(rectArea.width/2.0f-textW/2.0f, rectArea.height/2-100.0f/2.0f + 80.0f);
		draw(text);

		display();
	}
	/**/

	m_vpComponents.push_back(&m_MapRenderBack);
	m_vpComponents.push_back(&m_ParticleRenderBack);
	m_vpComponents.push_back(&m_PlayerRender);
	m_vpComponents.push_back(&m_ParticleRenderFront);
	m_vpComponents.push_back(&m_ItemRender);
	m_vpComponents.push_back(&m_FluidRender);
	m_vpComponents.push_back(&m_LightRender);
	m_vpComponents.push_back(&m_MapRenderFront);
	m_vpComponents.push_back(&m_ParticleRenderForeground);
	m_vpComponents.push_back(&m_DebuggerRender);
	m_vpComponents.push_back(&m_Menus);
	m_vpComponents.push_back(&m_UI);

	m_SystemSound.setAssetManager(&Assets());

	m_vpSystems.push_back(&m_SystemSound); 		// Sound: sound spatialization
	m_vpSystems.push_back(&m_SystemBox2D); 		// Box2D: for realistic physics

	std::deque<ISystem*>::iterator itEng = m_vpSystems.begin();
	while (itEng != m_vpSystems.end())
	{
    	if (!(*itEng)->init())
    		return false;
    	++itEng;
	}

	return initializeGameMode("menu");
}

bool CGameClient::initializeGameMode(const char *pGameType) noexcept
{
	reset();

	if (!pGameType)
	{
		ups::msgDebug("CGameClient", "Invalid Game Mode!");
		return false;
	}

    ups::msgDebug("CGameClient", "Initializing game mode...");
    if (ups::strCaseCmp(pGameType, "menu") == 0)
    	m_pGameController = new CControllerMenu();
    else if (ups::strCaseCmp(pGameType, "main") == 0)
    	m_pGameController = new CControllerMain();
    else
	{
		ups::msgDebug("CGameClient", "Oops... Invalid game mode!");
		return false;
	}
    m_pGameController->onInit();

	ups::msgDebug("CGameClient", "Starting game mode...");
	m_pGameController->onStart();

	return true;
}

void CGameClient::setRenderMode(int mode) noexcept
{
	m_RenderMode = mode;
}

void CGameClient::getViewportGlobalBounds(sf::FloatRect *pRect, const sf::View &view, float margin) noexcept
{
	const sf::Vector2f halfSize(view.getSize()/2.0f);
	pRect->left = (view.getCenter().x - halfSize.x) - margin;
    pRect->width = (view.getCenter().x + halfSize.x) + margin;
    pRect->top = (view.getCenter().y - halfSize.y) - margin;
    pRect->height = (view.getCenter().y + halfSize.y) + margin;
}

bool CGameClient::isClipped(const sf::Vector2f &worldPos, float margin) noexcept
{
	if (!Controller() || !Controller()->Context())
		return false;

	sf::FloatRect screenArea;
	getViewportGlobalBounds(&screenArea, Camera(), margin);
	screenArea.width -= screenArea.left;
	screenArea.height -= screenArea.top;

	return !screenArea.contains(worldPos);
}

bool CGameClient::isClipped(const sf::FloatRect &worldRect, float margin) noexcept
{
	if (!Controller() || !Controller()->Context())
			return false;

	sf::FloatRect screenArea;
	getViewportGlobalBounds(&screenArea, Camera(), margin);
	screenArea.width -= screenArea.left;
	screenArea.height -= screenArea.top;

	return !worldRect.intersects(screenArea);
}

bool CGameClient::isClipped(const std::vector<sf::Vector2f> &points, float margin) noexcept
{
	if (!Controller() || !Controller()->Context())
			return false;

	sf::FloatRect screenArea;
	getViewportGlobalBounds(&screenArea, Camera(), margin);
	screenArea.width -= screenArea.left;
	screenArea.height -= screenArea.top;

	std::vector<sf::Vector2f>::const_iterator itp = points.begin();
	while  (itp != points.end())
	{
		if (screenArea.contains(*itp))
			return false;
		++itp;
	}

	return true;
}
