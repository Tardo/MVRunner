/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/system.hpp>
#include <game/entities/primitives/CB2Polygon.hpp>
#include <game/entities/primitives/CB2Circle.hpp>
#include <game/entities/primitives/CB2Chain.hpp>
#include <game/entities/CSign.hpp>
#include <game/entities/CAmbientSound.hpp>
#include <tmxparser/TmxObject.h>
#include <tmxparser/TmxPolygon.h>
#include <tmxparser/TmxEllipse.h>
#include <tmxparser/TmxPolyline.h>
#include <tmxparser/TmxPoint.h>
#include <Box2D/Box2D.h>
#include <engine/CSystemLight.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemBox2D.hpp>
#include <game/components/CMapRender.hpp>
#include <game/controllers/CControllerMain.hpp>
#include <game/CController.hpp>


CController::CController() noexcept
{
	m_pGame = CGame::getInstance();
	m_pGameContext = new CContext();
	m_TimerStorm = 0;
}
CController::CController(class CContext *pContext) noexcept
{
	m_pGame = CGame::getInstance();
	m_pGameContext = pContext;
	m_TimerStorm = 0;
}
CController::~CController() noexcept
{
	if (m_pGameContext)
		delete m_pGameContext;
	m_pGameContext = nullptr;
}

bool CController::onInit() noexcept
{
	return true;
}

bool CController::onMapTile(unsigned int tileId, const sf::Vector2f &pos, unsigned int tileDir, unsigned int modifierId) noexcept
{
	if (tileId >= ENTITY_OFFSET)
	{
		const unsigned int entityId = tileId-ENTITY_OFFSET;
		if (entityId == ENTITY_SPAWN_PLAYER)
		{
			m_PlayerSpawnPos.m_Pos = pos;
			return true;
		}
	}

	return false;
}

void CController::onMapObject(CMapRenderObject *pMapObj, int objId, const sf::Vector2f &worldPos, const sf::Vector2f &size) noexcept
{
}

void CController::updateCamera(float deltaTime) noexcept
{
	Game()->Client()->setView(Game()->Client()->Camera());
	// Camera Pos
	Game()->Client()->Camera().update(deltaTime);
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

	if (Context()->Map().isMapLoaded())
	{
		// Create Map Objects
		sf::FloatRect screenArea;
		Game()->Client()->getViewportGlobalBounds(&screenArea, Game()->Client()->Camera(), 0.0f); // MARGIN_CREATE_OBJECTS

		std::list<CMapRenderObject*> vObjects = Context()->Map().getObjects()->queryAABB(screenArea);
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
						const Tmx::Color color = lightProps.GetColorProperty("color", Tmx::Color(255, 255, 255, 255));
						const sf::Color colorLight(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());

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
				const Tmx::PropertySet GeomProps = pMapObj->m_pObject->GetProperties();
				const Tmx::Color color = GeomProps.GetColorProperty("color", Tmx::Color(255, 255, 255, 0));
				const sf::Color colorGeom(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha());
				const int onContactFx = GeomProps.GetIntProperty("on_contact_fx", CEntity::FX_NONE);

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

					const CB2BodyInfo bodyInfo = CB2BodyInfo(0.9f, 0.5f, 0.1f, b2Type, CAT_BUILD);
					pMapObj->m_pEntity = new CB2Polygon(
							sf::Vector2f(globalBounds.left, globalBounds.top),
							points,
							colorGeom,
							bodyInfo);
					pMapObj->m_pEntity->m_ContactFx = onContactFx;
					pMapObj->m_pEntity->getBody()->SetAwake(false);
					ups::msgDebug("GameContext", "Polygon Created! [#%d]", objId);
				}
				else
				{
					const Tmx::Ellipse *pEllipse = pMapObj->m_pObject->GetEllipse();
					if (pEllipse)
					{
						const CB2BodyInfo bodyInfo = CB2BodyInfo(0.9f, 0.5f, 0.1f, b2Type, CAT_BUILD);
						pMapObj->m_pEntity = new CB2Circle(
								worldPosObj,
								pEllipse->GetRadiusX(),
								colorGeom,
								bodyInfo);
						pMapObj->m_pEntity->m_ContactFx = onContactFx;
						pMapObj->m_pEntity->getBody()->SetAwake(false);
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

							const CB2BodyInfo bodyInfo = CB2BodyInfo(0.9f, 0.5f, 0.1f, b2Type, CAT_BUILD);
							pMapObj->m_pEntity = new CB2Chain(
									worldPosObj,
									points,
									colorGeom,
									bodyInfo);
							pMapObj->m_pEntity->m_ContactFx = onContactFx;
							pMapObj->m_pEntity->getBody()->SetAwake(false);
							ups::msgDebug("GameContext", "PolyLine Created! [#%d]", objId);
						}
						else
						{
							const CB2BodyInfo bodyInfo = CB2BodyInfo(0.9f, 0.5f, 0.1f, b2Type, CAT_BUILD);
							pMapObj->m_pEntity = new CB2Polygon(
									worldPosObj,
									sizeObj,
									colorGeom,
									bodyInfo);
							pMapObj->m_pEntity->m_ContactFx = onContactFx;
							pMapObj->m_pEntity->getBody()->SetAwake(false);
							ups::msgDebug("GameContext", "Rectangle Created! [#%d]", objId);
						}
					}
				}
			}

			++itob;
		}
	}


	std::vector<CEntity*> &vpEntities = Context()->getEntities();
	std::vector<CParticle*> &vpParticles = Context()->getParticles();

	// Entities
    std::vector<CEntity*>::iterator itE = vpEntities.begin();
    while (itE != vpEntities.end())
    {
    	CEntity *pEnt = (*itE);
    	if (pEnt->isToDelete())
    	{
    		delete pEnt;
    		pEnt = nullptr;
    		itE = vpEntities.erase(itE);
    	}
    	else
    	{
    		(*itE)->tick();
    		++itE;
    	}
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
		{
			(*itP)->update();
			++itP;
		}
	}

	// Weather
	if (Context()->getWeather() == WEATHER_RAIN)
	{
		sf::FloatRect screenArea;
		Game()->Client()->getViewportGlobalBounds(&screenArea, Game()->Client()->Camera());
		const int startX = screenArea.left - 64;
		const int endX = screenArea.width + 64;
		const int startY = screenArea.top - 5;
		const int endY = screenArea.height;

		CSystemFx *pFxEngine = Game()->Client()->getSystem<CSystemFx>();
		pFxEngine->createRainBack(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 20.0f);
		pFxEngine->createRainFront(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 15.0f);

		if (ups::timeGet()-m_TimerStorm > ups::timeFreq()*upm::randInt(5, 15))
		{
			pFxEngine->createStorm();
			m_TimerStorm = ups::timeGet();
		}
	}
	else if (Context()->getWeather() == WEATHER_SNOW)
	{
		sf::FloatRect screenArea;
		Game()->Client()->getViewportGlobalBounds(&screenArea, Game()->Client()->Camera());
		const int startX = screenArea.left - 64;
		const int endX = screenArea.width + 64;
		const int startY = screenArea.top - 5;
		const int endY = screenArea.height;

		CSystemFx *pFxEngine = Game()->Client()->getSystem<CSystemFx>();
		pFxEngine->createSnowBack(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 5.0f);
		pFxEngine->createSnowFront(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 2.0f);
	}
}

void CController::onStart() noexcept
{
	Game()->Client()->Menus().setActive(CMenus::NONE);
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
