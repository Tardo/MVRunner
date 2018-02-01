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
  m_AssetManager(&m_Zpg)
{
	setFramerateLimit(60);
	m_pGameController = nullptr;
	m_Debug = false;
	m_Paused = false;
	m_FPS = 0;
	m_MousePosition = sf::Vector2i(0, 0);
	m_ViewHud = getDefaultView();
	m_TimerBroadcast = ups::timeGet();
	m_BroadcastDuration = 0.0f;
	m_DeltaTime = 0.0f;
	m_aHelpMsg[0] = 0;
	m_aBroadcastMsg[0] = 0;
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
            	else if (event.key.code == sf::Keyboard::B)
				{
            		CSystemWeather *pSystemWeather = getSystem<CSystemWeather>();
            		int nextWeather = pSystemWeather->getWeather()+1;
            		if (nextWeather >= CSystemWeather::NUM_WEATHERS)
            			nextWeather = 0;
            		pSystemWeather->setWeather(nextWeather);
				}
            }

			if (Controller())
				Controller()->onSystemEvent(&event);
        }

        m_MousePosition = sf::Mouse::getPosition(*this);

        // Update Systems
    	std::deque<CSystem*>::const_iterator itSys = m_vpSystems.cbegin();
    	while (itSys != m_vpSystems.cend())
        	(*itSys++)->update(m_DeltaTime);

        // Update Components
    	std::deque<CComponent*>::const_iterator itComp = m_vpComponents.cbegin();
    	while (itComp != m_vpComponents.cend())
        	(*itComp++)->update(m_DeltaTime);

        // Update Controller
        if (Controller())
        {
        	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        	Controller()->tick();
        }
        else
        	clear(sf::Color::Black);

        // HUD
    	setView(m_ViewHud);
    	drawHUD();
        if (m_Debug)
        	drawDebugInfo();
        draw(m_Menus);

        // Cursor
        drawCursor();

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

void CGameClient::drawHUD() noexcept
{
	if (Menus().getActive() != CMenus::NONE)
		return;

	sf::FloatRect rectArea;
	getViewportGlobalBounds(&rectArea, getHudView());

	sf::Text sfStr;
	sfStr.setFont(Assets().getDefaultFont());

	char aBuff[128];
	sfStr.setCharacterSize(62);
	snprintf(aBuff, sizeof(aBuff), "FPS: %u [Min: %u] [%.2fms]", m_FPS, m_MinFPS, m_DeltaTime*1000.0f);
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 0.0f);
	sfStr.setFillColor(sf::Color::Red);
	draw(sfStr);

	if (ups::timeGet() - m_TimerBroadcast < ups::timeFreq()*m_BroadcastDuration)
	{
		sfStr.setCharacterSize(128);
		sfStr.setString(m_aBroadcastMsg);
		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f+2.0f, 2.0f);
		sfStr.setFillColor(sf::Color::Yellow);
		draw(sfStr);

		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f, 0.0f);
		sfStr.setFillColor(sf::Color::Red);
		draw(sfStr);
	}

	if (m_aHelpMsg[0] != 0)
	{
		sfStr.setCharacterSize(92);
		sfStr.setString(m_aHelpMsg);
		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f+2.0f, 78.0f+2.0f);
		sfStr.setFillColor(sf::Color::Black);
		draw(sfStr);

		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f, 78.0f);
		sfStr.setFillColor(sf::Color::White);
		draw(sfStr);
	}
}

void CGameClient::showBroadcastMessage(const char *pMsg, float duration) noexcept
{
	m_TimerBroadcast = ups::timeGet();
	m_BroadcastDuration = duration;
	strncpy(m_aBroadcastMsg, pMsg, BROADCAST_MAX_LENGTH);
}

void CGameClient::showHelpMessage(const char *pMsg) noexcept
{
	strncpy(m_aHelpMsg, pMsg, HELP_TEXT_MAX_LENGTH);
}

void CGameClient::drawDebugInfo() noexcept
{
	if (!Controller() || !Controller()->Context())
		return;

	CSystemLight *pSystemLight = getSystem<CSystemLight>();
	CSystemSound *pSystemSound = getSystem<CSystemSound>();
	CSystemWeather *pSystemWeather = getSystem<CSystemWeather>();

	sf::FloatRect rectArea;
	getViewportGlobalBounds(&rectArea, getHudView());

	char aBuff[128];
	sf::Text sfStr;
	sfStr.setFont(Assets().getDefaultFont());
	sfStr.setCharacterSize(62);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Entidades: %lu", Controller()->Context()->getNumEntities());
	#else
	snprintf(aBuff, sizeof(aBuff), "Entidades: %u", Controller()->Context()->getNumEntities());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 30.0f);
	sfStr.setFillColor(sf::Color::Red);
	draw(sfStr);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Particulas: %lu", Controller()->Context()->getNumParticles());
	#else
	snprintf(aBuff, sizeof(aBuff), "Particulas: %u", Controller()->Context()->getNumParticles());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 60.0f);
	sfStr.setFillColor(sf::Color::Red);
	draw(sfStr);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Luces: %lu", pSystemLight->getLights().size());
	#else
	snprintf(aBuff, sizeof(aBuff), "Luces: %u", m_SystemLight.getLights().size());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 90.0f);
	sfStr.setFillColor(sf::Color::Red);
	draw(sfStr);

	#ifdef __LP64__
	snprintf(aBuff, sizeof(aBuff), "Sonidos: %d", pSystemSound->getNumPlayingSound());
	#else
	snprintf(aBuff, sizeof(aBuff), "Sonidos: %d", m_SystemSound.getNumPlayingSound());
	#endif
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 120.0f);
	sfStr.setFillColor(sf::Color::Red);
	draw(sfStr);

	if (pSystemWeather->getWeather() == CSystemWeather::WEATHER_RAIN)
			snprintf(aBuff, sizeof(aBuff), "Clima: Lluvia");
	else
		snprintf(aBuff, sizeof(aBuff), "Clima: Soleado");
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 150.0f);
	sfStr.setFillColor(sf::Color::Red);
	draw(sfStr);
}

void CGameClient::drawCursor() noexcept
{
	sf::Vertex line[] =
	{
		sf::Vertex(sf::Vector2f(m_MousePosition.x-10.0f, m_MousePosition.y), sf::Color::White),
		sf::Vertex(sf::Vector2f(m_MousePosition.x+10.0f, m_MousePosition.y), sf::Color::White),
		sf::Vertex(sf::Vector2f(m_MousePosition.x, m_MousePosition.y-10.0f), sf::Color::White),
		sf::Vertex(sf::Vector2f(m_MousePosition.x, m_MousePosition.y+10.0f), sf::Color::White)
	};

	draw(line, 4, sf::Lines);
}

void CGameClient::reset() noexcept
{
	MapRender().reset();
	if (m_pGameController != nullptr)
		delete m_pGameController;
	m_pGameController = nullptr;
	m_TimerBroadcast = ups::timeGet();
	m_BroadcastDuration = 0.0f;
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

		m_MousePosition = sf::Mouse::getPosition(*this);
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

	m_vpComponents.push_back(&m_MapRender);
	m_vpComponents.push_back(&m_Menus);
	m_vpComponents.push_back(&m_UI);
	m_vpComponents.push_back(&m_Camera);

	std::deque<CComponent*>::iterator itComp = m_vpComponents.begin();
	while (itComp != m_vpComponents.end())
    	(*itComp++)->m_pGameClient = this;

	m_vpSystems.push_back(&m_SystemSound); 		// Sound: sound spatialization
	m_vpSystems.push_back(&m_SystemBox2D); 		// Box2D: for realistic physics
	m_vpSystems.push_back(&m_SystemFx); 		// Effects: Particle System
	m_vpSystems.push_back(&m_SystemWeather);	// Weather: Weather System
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
    bool needLoadMap = false;

    if (ups::strCaseCmp(pGameType, "menu") == 0)
    	m_pGameController = new CControllerMenu();
    else if (ups::strCaseCmp(pGameType, "main") == 0)
    {
    	m_pGameController = new CControllerMain();
    	needLoadMap = true;
    }
    else
	{
		ups::msgDebug("CGame", "Oops... Invalid game mode!");
		return false;
	}

    if (needLoadMap)
    {
    	unsigned long fileSize = 0;
    	const unsigned char *pData = Storage().getFileData("data/map.tmx", &fileSize);
        if (!MapRender().loadMap(Zpg::toString(pData, fileSize).c_str()))
        {
        	ups::msgDebug("CGame", "Oops... Error loading map!");
        	return false;
        }

		if (MapRender().isMapLoaded())
		{
			// Analizar Game Layer
			const Tmx::TileLayer *pGameLayer = MapRender().getGameLayer();
			const Tmx::TileLayer *pGameModifiersLayer = MapRender().getGameModifiersLayer();
			if (pGameLayer)
			{
				ups::msgDebug("CGame", "Analyzing map entities (%dx%d)...", pGameLayer->GetWidth(), pGameLayer->GetHeight());
				for (int i=0; i<pGameLayer->GetHeight(); i++)
				{
					for (int e=0; e<pGameLayer->GetWidth(); e++)
					{
						const Tmx::MapTile &curTile = pGameLayer->GetTile(e, i);
						if (curTile.tilesetId < 0)
							continue;

						const Tmx::Tileset *pTileset = MapRender().getMap()->GetTileset(curTile.tilesetId);
						const int tileIndex = (curTile.gid - pTileset->GetFirstGid());
						if (tileIndex<=0)
							continue;

						sf::Vector2f worldPos(
							e*MapRender().getMap()->GetTileWidth()+MapRender().getMap()->GetTileWidth()/2.0f,
							i*MapRender().getMap()->GetTileHeight()+MapRender().getMap()->GetTileHeight()/2.0f
						);

						unsigned int modifierId = 0;
						if (pGameModifiersLayer)
						{
							const Tmx::MapTile &curModifierTile = pGameModifiersLayer->GetTile(e, i);
							if (curModifierTile.tilesetId >= 0)
							{
								const Tmx::Tileset *pTilesetModifiers = MapRender().getMap()->GetTileset(curModifierTile.tilesetId);
								modifierId = (curModifierTile.gid - pTilesetModifiers->GetFirstGid());
							}
						}

						m_pGameController->onMapTile(tileIndex, worldPos, MapRender().getTileDirection(sf::Vector2i(e, i)), modifierId);
					}
				}
			}

			// Map Objects
			std::list<CMapRenderObject*> vObjects = MapRender().getObjects()->queryAll();
			std::list<CMapRenderObject*>::const_iterator itob = vObjects.cbegin();
			while (itob != vObjects.cend())
			{
				CMapRenderObject *pMapObj = (*itob);
				if (!pMapObj || (pMapObj && !pMapObj->m_pObject))
				{
					++itob;
					continue;
				}

				const sf::FloatRect globalBounds(
					pMapObj->m_pObject->GetX(), pMapObj->m_pObject->GetY(),
					pMapObj->m_pObject->GetWidth(), pMapObj->m_pObject->GetHeight()
				);
				const sf::Vector2f worldPosObj(globalBounds.left+pMapObj->m_pObject->GetWidth()/2.0f, globalBounds.top+pMapObj->m_pObject->GetHeight()/2.0f);
				const sf::Vector2f sizeObj(globalBounds.width, globalBounds.height);
				const int objId = pMapObj->m_pObject->GetId();

				m_pGameController->onMapObject(pMapObj, objId, worldPosObj, sizeObj);
				++itob;
			}
		}
    }

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
