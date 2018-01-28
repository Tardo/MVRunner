/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/system.hpp>
#include <game/entities/primitives/CB2Polygon.hpp>
#include <game/entities/primitives/CB2Circle.hpp>
#include <game/entities/primitives/CB2Chain.hpp>
#include <game/entities/CSign.hpp>
#include <game/entities/CBox.hpp>
#include <game/entities/CAmbientSound.hpp>
#include <tmxparser/TmxObject.h>
#include <tmxparser/TmxPolygon.h>
#include <tmxparser/TmxEllipse.h>
#include <tmxparser/TmxPolyline.h>
#include <tmxparser/TmxPoint.h>
#include <Box2D/Box2D.h>
#include <engine/CSystemLight.hpp>
#include <engine/CSystemWeather.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/components/CMapRender.hpp>
#include <game/controllers/CControllerMain.hpp>
#include <game/CController.hpp>


CController::CController() noexcept
{
	m_pGame = CGame::getInstance();
	m_pGameContext = new CContext();
	m_pPlayerLight = nullptr;
}
CController::CController(class CContext *pContext) noexcept
{
	m_pGame = CGame::getInstance();
	m_pGameContext = pContext;
	m_pPlayerLight = nullptr;
}
CController::~CController() noexcept
{
	if (m_pGameContext)
		delete m_pGameContext;
	m_pGameContext = nullptr;

	if (m_pPlayerLight)
		m_pPlayerLight->destroy();
	m_pPlayerLight = nullptr;
}

bool CController::onMapTile(unsigned int tileId, const sf::Vector2f &pos, unsigned int tileDir, unsigned int modifierId) noexcept
{
	if (tileId >= ENTITY_OFFSET)
	{
		const unsigned int entityId = tileId-ENTITY_OFFSET;
		if (entityId == ENTITY_SPAWN_PLAYER)
		{
			m_PlayerSpawnPos.m_Pos = pos;
			m_PlayerSpawnPos.m_Dir = Game()->Client()->MapRender().getTileDirectionVector(tileDir);
			return true;
		}
	}

	return false;
}

void CController::onMapObject(CMapRenderObject *pMapObj, int objId, const sf::Vector2f &worldPos, const sf::Vector2f &size) noexcept
{
}

void CController::onSystemEvent(sf::Event *pEvent) noexcept
{ }

void CController::updateCamera(float deltaTime) noexcept
{
	Game()->Client()->setView(Game()->Client()->Camera());
	// Camera Pos
	Game()->Client()->Camera().update(deltaTime);
	CCharacter *pChar = Context()->getPlayer()->getCharacter();
	if (pChar)
	{
		Game()->Client()->Camera().setSmoothZoom(pChar->isAlive()?g_Config.m_ZoomCharacter:g_Config.m_ZoomDead);
		if (!pChar->isAlive())
			Game()->Client()->Camera().rotate(32.0f*deltaTime);
		else
			Game()->Client()->Camera().setRotation(0.0f);
	}
}

bool CController::isStaticObject(const char *pType) const noexcept
{
	static const unsigned int s_NumTypes = 3;
	static const char s_aStaticTypes[s_NumTypes][18] = {
		"static\0",
		"dynamic\0",
		"kinematic\0",
	};

	for (unsigned int i=0; i<s_NumTypes; i++)
	{
		if (ups::strNCaseCmp(pType, s_aStaticTypes[i], 18) == 0)
			return true;
	}

	return false;
}

void CController::tick() noexcept
{
	CSystemLight *pLightEngine = Game()->Client()->getSystem<CSystemLight>();

	// Camera Pos
	updateCamera(Game()->Client()->getDeltaTime());

	if (Game()->Client()->MapRender().isMapLoaded())
	{
		// Create Map Objects
		sf::FloatRect screenArea;
		Game()->Client()->getViewportGlobalBounds(&screenArea, Game()->Client()->Camera(), 0.0f); // MARGIN_CREATE_OBJECTS

		std::list<CMapRenderObject*> vObjects = Game()->Client()->MapRender().getObjects()->queryAABB(screenArea);
		//ups::msgDebug("CONTEXT", "Num: %d", vObjects.size());
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
				pMapObj->m_pObject->GetX()+pMapObj->m_Offset.x, pMapObj->m_pObject->GetY()+pMapObj->m_Offset.y,
				pMapObj->m_pObject->GetWidth(), pMapObj->m_pObject->GetHeight()
			);
			const sf::Vector2f worldPosObj(globalBounds.left+pMapObj->m_pObject->GetWidth()/2.0f, globalBounds.top+pMapObj->m_pObject->GetHeight()/2.0f);
			const sf::Vector2f sizeObj(globalBounds.width, globalBounds.height);
			const int objId = pMapObj->m_pObject->GetId();

			// Dynamic Objects
			if (!isStaticObject(pMapObj->m_pObject->GetType().c_str()))
			{
				bool canCreate = false;
				if (pMapObj->m_pObject->GetPolyline())
				{
					const Tmx::Polyline *pPolyline = pMapObj->m_pObject->GetPolyline();
					if (pPolyline)
					{
						for (int i=0; i<pPolyline->GetNumPoints(); i++)
						{
							const sf::Vector2f pointPos = sf::Vector2f(globalBounds.left+pPolyline->GetPoint(i).x, globalBounds.top+pPolyline->GetPoint(i).y);
							if (!Game()->Client()->isClipped(pointPos, MARGIN_CREATE_OBJECTS))
							{
								canCreate = true;
								break;
							}
						}
					}
				}
				else if (pMapObj->m_pObject->GetPolygon())
				{
					const Tmx::Polygon *pPoly = pMapObj->m_pObject->GetPolygon();
					if (pPoly)
					{
						for (int i=0; i<pPoly->GetNumPoints(); i++)
						{
							const sf::Vector2f pointPos = sf::Vector2f(globalBounds.left+pPoly->GetPoint(i).x, globalBounds.top+pPoly->GetPoint(i).y);
							if (!Game()->Client()->isClipped(pointPos, MARGIN_CREATE_OBJECTS))
							{
								canCreate = true;
								break;
							}
						}
					}
				}
				else
					canCreate = !Game()->Client()->isClipped(globalBounds, MARGIN_CREATE_OBJECTS);

				if (!canCreate)
				{
					if (pMapObj->m_pLight)
					{
						pMapObj->m_pLight->destroy();
						pMapObj->m_pLight = nullptr;
					}
					if (pMapObj->m_pEntity)
					{
						pMapObj->m_pEntity->destroy();
						pMapObj->m_pEntity = nullptr;
					}
				}
				else if (!pMapObj->m_pLight && !pMapObj->m_pEntity)
				{
					if (pMapObj->m_pObject->GetType().compare("light") == 0)
					{
						const Tmx::PropertySet &lightProps = pMapObj->m_pObject->GetProperties();
						const float blink = lightProps.GetFloatProperty("blink", 0.0f);
						const float variationSize = lightProps.GetFloatProperty("variation_size", 0.0f);
						sf::Color colorLight;
						colorLight.r = lightProps.GetIntProperty("color_r", 255);
						colorLight.g = lightProps.GetIntProperty("color_g", 255);
						colorLight.b = lightProps.GetIntProperty("color_b", 255);
						colorLight.a = lightProps.GetIntProperty("color_a", 255);

						std::string lightType = lightProps.GetStringProperty("type");
						bool alwaysOn = lightProps.GetBoolProperty("always_on", false);

						if (lightType.compare("emissive") == 0)
						{
							std::string ligthDirection = lightProps.GetStringProperty("direction");
							float degrees = 0.0f;
							if (ligthDirection.compare("left") == 0)
								degrees = -90.0f;
							else if (ligthDirection.compare("right") == 0)
								degrees = 90.0f;
							else if (ligthDirection.compare("bottom") == 0)
								degrees = 180.0f;

							pMapObj->m_pLight = pLightEngine->createEmissive(
								worldPosObj,
								degrees,
								sf::Vector2f(globalBounds.width/225.0f, globalBounds.height/225.0f),
								colorLight, alwaysOn, blink, variationSize
							);
						}
						else
						{
							pMapObj->m_pLight = pLightEngine->createPoint(
								worldPosObj,
								sf::Vector2f(globalBounds.width/256.0f, globalBounds.height/256.0f),
								colorLight, alwaysOn, blink, variationSize
							);
						}
					}
					else if (pMapObj->m_pObject->GetType().compare("smoke") == 0)
					{
						const sf::Vector2f pos(globalBounds.left+globalBounds.width/2.0f, globalBounds.top+globalBounds.height/2.0f);
					}
					else if (pMapObj->m_pObject->GetType().compare("box") == 0)
					{
						pMapObj->m_pEntity = new CBox(worldPosObj, sizeObj, VECTOR_ZERO, 0.0f, BOX_WOOD, 2u);
					}
					else if (pMapObj->m_pObject->GetType().compare("sign") == 0)
					{
						const Tmx::PropertySet &signProps = pMapObj->m_pObject->GetProperties();
						pMapObj->m_pEntity = new CSign(worldPosObj, sizeObj, signProps.GetStringProperty("message", "").c_str());
					}
					else if (pMapObj->m_pObject->GetType().compare("sound") == 0)
					{
						const float minDist = upm::min(sizeObj.x, sizeObj.y);
						const Tmx::PropertySet &soundProps = pMapObj->m_pObject->GetProperties();
						pMapObj->m_pEntity = new CAmbientSound(worldPosObj, soundProps.GetIntProperty("sound_id", -1), minDist/2.0f, soundProps.GetIntProperty("loop", 1), soundProps.GetFloatProperty("volume", 100.0f));
					}
				}
			} else if (!pMapObj->m_pLight && !pMapObj->m_pEntity)
			{
				const Tmx::PropertySet GeoProps = pMapObj->m_pObject->GetProperties();
				const sf::Color GeoColor(
					GeoProps.GetIntProperty("color_r", 255),
					GeoProps.GetIntProperty("color_g", 255),
					GeoProps.GetIntProperty("color_b", 255),
					GeoProps.GetIntProperty("color_a", 0)
				);

				b2BodyType b2Type = b2_dynamicBody;
				if (pMapObj->m_pObject->GetType().compare("static") == 0)
					b2Type = b2_staticBody;
				else if (pMapObj->m_pObject->GetType().compare("kinematic") == 0)
					b2Type = b2_kinematicBody;

				const Tmx::Polygon *pPoly = pMapObj->m_pObject->GetPolygon();
				if (pPoly)
				{
					std::vector<sf::Vector2f> points;
					for (int i=0; i<pPoly->GetNumPoints(); i++)
						points.push_back(sf::Vector2f(pPoly->GetPoint(i).x, pPoly->GetPoint(i).y));

					const CB2BodyInfo bodyInfo = CB2BodyInfo(0.1f, 0.5f, 0.1f, b2Type, CAT_BUILD);
					pMapObj->m_pEntity = new CB2Polygon(
							sf::Vector2f(globalBounds.left, globalBounds.top),
							1,
							points,
							GeoColor,
							bodyInfo);
					pMapObj->m_pEntity->getShape()->setOrigin(globalBounds.width/2.0f, globalBounds.height/2.0f);
					ups::msgDebug("GameContext", "Polygon Created! [#%d]", objId);
				}
				else
				{
					const Tmx::Ellipse *pEllipse = pMapObj->m_pObject->GetEllipse();
					if (pEllipse)
					{
						const CB2BodyInfo bodyInfo = CB2BodyInfo(0.1f, 0.5f, 0.1f, b2Type, CAT_BUILD);
						pMapObj->m_pEntity = new CB2Circle(
								worldPosObj,
								1,
								pEllipse->GetRadiusX(),
								GeoColor,
								bodyInfo);
						ups::msgDebug("GameContext", "Ellipse Created: %s [#%d]", pMapObj->m_pObject->GetType().c_str(), objId);
					}
					else
					{
						const Tmx::Polyline *pPolyline = pMapObj->m_pObject->GetPolyline();
						if (pPolyline)
						{
							std::vector<sf::Vector2f> points;
							for (int i=0; i<pPolyline->GetNumPoints(); ++i)
								points.push_back(sf::Vector2f(pPolyline->GetPoint(i).x, pPolyline->GetPoint(i).y));

							const CB2BodyInfo bodyInfo = CB2BodyInfo(0.1f, 0.5f, 0.1f, b2Type, CAT_BUILD);
							pMapObj->m_pEntity = new CB2Chain(
									worldPosObj,
									1,
									points,
									GeoColor,
									bodyInfo);
							ups::msgDebug("GameContext", "PolyLine Created! [#%d]", objId);
						}
						else
						{
							const CB2BodyInfo bodyInfo = CB2BodyInfo(0.1f, 0.5f, 0.1f, b2Type, CAT_BUILD);
							pMapObj->m_pEntity = new CB2Polygon(
									worldPosObj,
									1,
									sizeObj,
									GeoColor,
									bodyInfo);
							ups::msgDebug("GameContext", "Rectangle Created! [#%d]", objId);
						}
					}
				}
			}

			++itob;
		}
	}


	std::vector<CEntity*> &vpEntities = Context()->getAllEntities();
	std::vector<CParticle*> &vpParticles = Context()->getAllParticles();

	/** DELETE **/
	// Entities
    std::vector<CEntity*>::iterator itE = vpEntities.begin();
    while (itE != vpEntities.end())
    {
    	CEntity *pEnt = (*itE);
    	if (pEnt->isToDelete())
    	{
    		// Is a MapObject Entity?
    		if (Game()->Client()->MapRender().isMapLoaded())
    		{
				// TODO: Add a flag for know if is a map object entity
				std::list<CMapRenderObject*> mapObjs = Game()->Client()->MapRender().getObjects()->queryAll();
				std::list<CMapRenderObject*>::iterator itObj = mapObjs.begin();
				while (itObj != mapObjs.end())
				{
					if ((*itObj)->m_pEntity == pEnt)
					{
						(*itObj)->m_pEntity = nullptr;
						break;
					}
					++itObj;
				}
    		}

    		delete pEnt;
    		pEnt = nullptr;
    		itE = vpEntities.erase(itE);
    	}
    	else
    		++itE;
    }
    // Particles
	std::vector<CParticle*>::iterator itP = vpParticles.begin();
	while (itP != vpParticles.end())
	{
		CParticle *pParticle = (*itP);
		if (pParticle->m_Duration != -1.0f && ups::timeGet()-pParticle->m_Timer > ups::timeFreq()*pParticle->m_Duration)
		{
			delete pParticle;
			pParticle = nullptr;
			itP = vpParticles.erase(itP);
			continue;
		}
		else
			++itP;
	}

	// Immutable Lists
	const std::vector<CEntity*> vpInmEntities(vpEntities.begin(), vpEntities.end());
	const std::vector<CParticle*> vpInmParticles(vpParticles.begin(), vpParticles.end());

	/** UPDATE **/
    // Entities
	std::vector<CEntity*>::const_iterator itEnt = vpInmEntities.cbegin();
    while (itEnt != vpInmEntities.cend())
    	(*itEnt++)->tick();
	// Particles
    std::vector<CParticle*>::const_iterator itPar = vpInmParticles.cbegin();
	while (itPar != vpInmParticles.cend())
		(*itPar++)->update();


	/** RENDER **/
	Game()->Client()->setView(m_pGame->Client()->getDefaultView());

	if (!Game()->Client()->m_Debug && Game()->Client()->MapRender().isMapLoaded())
	{
		// Map - Background
		const sf::Sprite &BackMap = Game()->Client()->MapRender().getBackMap(Game()->Client()->Camera(), sf::Color(128,128,128));

		// Render Map Background with Weather or not
		CSystemWeather *pWeatherEngine = m_pGame->Client()->getSystem<CSystemWeather>();
		if (pWeatherEngine->getWeather() == CSystemWeather::WEATHER_RAIN)
		{
			const sf::Sprite &WaterSprite = pWeatherEngine->getWeatherMap(Game()->Client()->Camera());
			sf::Shader *pWaterShader = Game()->Client()->Assets().getShader(CAssetManager::SHADER_WATER);
			pWaterShader->setUniform("wave_phase", Game()->Client()->getElapsedTime());
			pWaterShader->setUniform("texture_water", *WaterSprite.getTexture());
			Game()->Client()->draw(BackMap, pWaterShader);
		}
		else
		{
			Game()->Client()->draw(BackMap);
		}
	}

	Game()->Client()->setView(Game()->Client()->Camera());

	// Draw Entities (Lo que mas debajo esta...)
	itEnt = vpInmEntities.cbegin();
	while (itEnt != vpInmEntities.cend())
	{
		CEntity *pEnt = (*itEnt++);
		if (pEnt->getZLevel() == -1)
			Game()->Client()->draw(*pEnt);
	}

	// Particles
	itPar = vpInmParticles.cbegin();
	while (itPar != vpInmParticles.cend())
	{
		CParticle *pParticle = (*itPar++);
		if (pParticle->m_Render == RENDER_BACK)
			Game()->Client()->draw(*reinterpret_cast<sf::Drawable*>(pParticle));
	}

	// Draw Entities
	itEnt = vpInmEntities.cbegin();
	while (itEnt != vpInmEntities.cend())
	{
		CEntity *pEnt = (*itEnt++);
		if (pEnt->getZLevel() != -1)
			Game()->Client()->draw(*pEnt);
	}


	// Particles Front
	itPar = vpInmParticles.cbegin();
	while (itPar != vpInmParticles.cend())
	{
		CParticle *pParticle = (*itPar++);
		if (pParticle->m_Render == RENDER_FRONT)
			Game()->Client()->draw(*reinterpret_cast<sf::Drawable*>(pParticle));
	}

	Game()->Client()->setView(m_pGame->Client()->getDefaultView());

    // Render Light
    Game()->Client()->draw(pLightEngine->getLightmap(Game()->Client()->Camera()), sf::BlendMultiply);

	// Map - Front
    if (!Game()->Client()->m_Debug && Game()->Client()->MapRender().isMapLoaded())
    {
    	//sf::Shader *pAbbShader = Game()->Client()->Assets().getShader(CAssetManager::SHADER_CHROMATIC_ABERRATION);
    	//pAbbShader->setUniform("offc", sf::Vector3f(0.001f, -0.0012f, 0.0015f));
    	//pAbbShader->setUniform("texture", sf::Shader::CurrentTexture);
    	sf::Shader *pBlurShader = Game()->Client()->Assets().getShader(CAssetManager::SHADER_BLUR);
    	pBlurShader->setUniform("blur_radius_x", 0.0008f);
    	pBlurShader->setUniform("blur_radius_y", 0.0008f);
    	pBlurShader->setUniform("texture", sf::Shader::CurrentTexture);
    	Game()->Client()->draw(Game()->Client()->MapRender().getFrontMap(Game()->Client()->Camera(), pLightEngine->getTimeColor()), pBlurShader);
    }

    Game()->Client()->setView(Game()->Client()->Camera());

	// Particles Foreground
	itPar = vpInmParticles.cbegin();
	while (itPar != vpInmParticles.cend())
	{
		CParticle *pParticle = (*itPar++);
		if (pParticle->m_Render == RENDER_FOREGROUND)
			Game()->Client()->draw(*reinterpret_cast<sf::Drawable*>(pParticle));
	}
}

void CController::onStart() noexcept
{
	Game()->Client()->Menus().setActive(CMenus::NONE);
	// Spawn Player
	Context()->getPlayer()->createCharacter(m_PlayerSpawnPos.m_Pos, m_PlayerSpawnPos.m_Dir);
	Game()->Client()->Camera().setZoom(g_Config.m_ZoomCharacter);

	Game()->Client()->Camera().setTarget(Context()->getPlayer()->getCharacter());
}

void CController::onResetGame() noexcept
{
	if (m_pGameContext)
	{
		delete m_pGameContext;
		m_pGameContext = 0x0;
	}

	m_pGameContext = new CContext();
	onStart();
}

void CController::onCharacterDeath(CCharacter *pChar, CPlayer *pKiller) noexcept
{

}
