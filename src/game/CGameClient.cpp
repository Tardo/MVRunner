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
  m_MapRenderBack(RENDER_BACK),
  m_MapRenderFront(RENDER_FRONT),
  m_ParticleRenderBack(RENDER_BACK),
  m_ParticleRenderFront(RENDER_FRONT),
  m_ParticleRenderForeground(RENDER_FOREGROUND)
{
	setFramerateLimit(60);
	m_pGameController = nullptr;
	m_Debug = false;
	m_Paused = false;
	m_FPS = 0;
	m_ViewHud = getDefaultView();
	m_DeltaTime = 0.0f;
	m_MinFPS = 9999;

	m_TimerGame = ups::timeGet();
}
CGameClient::~CGameClient() noexcept
{
	reset();
	m_Zpg.close();

	#ifdef DEBUG_DESTRUCTORS
    ups::msgDebug("CGame", "Deleted");
	#endif
}


void CGameClient::run() noexcept
{
	ups::msgDebug("CGame", "Initializing game...");
	if (!init())
	{
		ups::msgDebug("CGame", "Error initializing game... closing!");
		close();
	}
	else
		ups::msgDebug("CGame", "Game initialized successfully :)");

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
            else if (event.type == sf::Event::KeyPressed)
            {
            	if (event.key.code == sf::Keyboard::Escape)
            	{
					if (Controller() && !dynamic_cast<CControllerMenu*>(Controller()))
					{
						if (Menus().getActive() != CMenus::NONE)
							Menus().setActive(CMenus::NONE);
						else
							Menus().setActive(CMenus::MAIN);
					}
					else if (Menus().getActive() != CMenus::MAIN)
						Menus().setActive(CMenus::MAIN);
            	}
            	else if (event.key.code == sf::Keyboard::F7)
            		m_Debug = !m_Debug;
            	else if (event.key.code == sf::Keyboard::R)
            	{
            		initializeGameMode("main");
            	}
            }

			if (Controller())
				Controller()->onSystemEvent(&event);
        }

        // Update Systems
    	std::deque<CSystem*>::const_iterator itSys = m_vpSystems.cbegin();
    	while (itSys != m_vpSystems.cend())
        	(*itSys++)->update(m_DeltaTime);

    	// Update Camera
    	m_Camera.update(m_DeltaTime);

    	// Update UI
    	m_UI.update();

        // Update Controller
        if (Controller())
        {
        	clear(CMap::tmxToSf(Controller()->Context()->Map().GetBackgroundColor()));
        	Controller()->tick();
        }
        else
        	clear(sf::Color::Black);

        // Render
        // Update Components
        std::deque<CComponent*>::const_iterator itComp = m_vpComponents.cbegin();
    	while (itComp != m_vpComponents.cend())
        	draw(*reinterpret_cast<sf::Drawable*>((*itComp++)));

        // Magia!
        display();

        // Calcular FPS
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

void CGameClient::reset() noexcept
{
	if (m_pGameController != nullptr)
		delete m_pGameController;
	m_pGameController = nullptr;
}

bool CGameClient::init() noexcept
{
	setVerticalSyncEnabled(g_Config.m_VSync);
	setMouseCursorVisible(false);
	setMouseCursorGrabbed(true);

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
			ups::msgDebug("CGAME", "ERROR: Can't load assets!");
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

	m_Camera.m_pGameClient = this;

	m_vpComponents.push_back(&m_MapRenderBack);
	m_vpComponents.push_back(&m_ParticleRenderBack);
	m_vpComponents.push_back(&m_PlayerRender);
	m_vpComponents.push_back(&m_ParticleRenderFront);
	m_vpComponents.push_back(&m_ItemRender);
	m_vpComponents.push_back(&m_MapRenderFront);
	m_vpComponents.push_back(&m_ParticleRenderForeground);
	m_vpComponents.push_back(&m_Menus);
	m_vpComponents.push_back(&m_UI);

	std::deque<CComponent*>::iterator itComp = m_vpComponents.begin();
	while (itComp != m_vpComponents.end())
    	(*itComp++)->m_pGameClient = this;

	m_vpSystems.push_back(&m_SystemSound); 		// Sound: sound spatialization
	m_vpSystems.push_back(&m_SystemBox2D); 		// Box2D: for realistic physics
	m_vpSystems.push_back(&m_SystemFx); 		// Effects: Particle System
	m_vpSystems.push_back(&m_SystemLight);		// Lights: Light System

	std::deque<CSystem*>::iterator itEng = m_vpSystems.begin();
	while (itEng != m_vpSystems.end())
	{
    	if (!(*itEng)->init(this))
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
		ups::msgDebug("CGame", "Invalid Game Mode!");
		return false;
	}

    ups::msgDebug("CGame", "Initializing game mode...");
    if (ups::strCaseCmp(pGameType, "menu") == 0)
    	m_pGameController = new CControllerMenu();
    else if (ups::strCaseCmp(pGameType, "main") == 0)
    	m_pGameController = new CControllerMain();
    else
	{
		ups::msgDebug("CGame", "Oops... Invalid game mode!");
		return false;
	}
    m_pGameController->onInit();

	ups::msgDebug("CGame", "Starting game mode...");
	m_pGameController->onStart();

	return true;
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

	return !screenArea.contains(worldPos);
}

bool CGameClient::isClipped(const sf::FloatRect &worldRect, float margin) noexcept
{
	if (!Controller() || !Controller()->Context())
			return false;

	sf::FloatRect screenArea;
	getViewportGlobalBounds(&screenArea, Camera(), margin);

	return !worldRect.intersects(screenArea);
}

bool CGameClient::isClipped(const std::vector<sf::Vector2f> &points, float margin) noexcept
{
	if (!Controller() || !Controller()->Context())
			return false;

	sf::FloatRect screenArea;
	getViewportGlobalBounds(&screenArea, Camera(), margin);

	std::vector<sf::Vector2f>::const_iterator itp = points.begin();
	while  (itp != points.end())
	{
		if (screenArea.contains(*itp))
			return false;
		++itp;
	}

	return true;
}
