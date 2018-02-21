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
#include <engine/CGame.hpp>
#include <game/components/CMapRender.hpp>
#include <game/components/CLightRender.hpp>
#include <game/controllers/CControllerMain.hpp>
#include <game/CController.hpp>
#include <engine/CSystemBox2D.hpp>


CController::CController() noexcept
{
	m_pGame = CGame::getInstance();
	m_pGameContext = new CContext();
}
CController::CController(class CContext *pContext) noexcept
{
	m_pGame = CGame::getInstance();
	m_pGameContext = pContext;
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
					if (pMapObj->m_pEntity)
					{
						pMapObj->m_pEntity->destroy();
						pMapObj->m_pEntity = nullptr;
					}
				}
				else if (!pMapObj->m_pEntity)
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

							pMapObj->m_pEntity = Game()->Client()->Controller()->createEmissive(
								worldPosObj,
								degrees,
								sf::Vector2f(globalBounds.width/225.0f, globalBounds.height/225.0f),
								colorLight, alwaysOn, blink, variationSize
							);
						}
						else
						{
							pMapObj->m_pEntity = Game()->Client()->Controller()->createPoint(
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
			} else if (!pMapObj->m_pEntity)
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

	// Weather
	if (Context()->getWeather() == WEATHER_RAIN)
	{
		sf::FloatRect screenArea;
		Game()->Client()->getViewportGlobalBounds(&screenArea, Game()->Client()->Camera());
		const int startX = screenArea.left - 64;
		const int endX = screenArea.width + 64;
		const int startY = screenArea.top - 5;
		const int endY = screenArea.height;

		createRainBack(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 20.0f);
		createRainFront(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 15.0f);

		if (ups::timeGet()-Context()->getTimerStorm() > ups::timeFreq()*upm::randInt(5, 15))
		{
			createStorm();
			Context()->setTimerStorm(ups::timeGet());
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

		createSnowBack(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 5.0f);
		createSnowFront(sf::Vector2f(startX+rand()%(endX-startX), startY+rand()%(endY-startY)), 2.0f);
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

void CController::createImpactSparkMetal(const sf::Vector2f &worldPos) noexcept
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
		pParticle->m_Luminance = true;
		pParticle->m_TextId = CAssetManager::TEXTURE_BULLET_SPARK;
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
		pParticle->m_Vel = upm::floatRand(60.5f, 80.5f);
	}
}

void CController::createBloodSpark(const sf::Vector2f &worldPos, float duration) noexcept
{
	if (!Game()->Client()->canAdd50Hz() || Game()->Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(2.0f, 2.0f);
	pParticle->m_SizeEnd = sf::Vector2f(4.0f, 4.0f);
	pParticle->m_ColorInit = sf::Color::Red;
	pParticle->m_ColorEnd = sf::Color::Red;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = duration;
	pParticle->m_TextId = CAssetManager::TEXTURE_BULLET_SPARK;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
}

void CController::createBlood(const sf::Vector2f &worldPos) noexcept
{
	if (Game()->Client()->isClipped(worldPos, 128.0f))
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
		pParticle->m_Rotation = upm::floatRand(0.0f, 360.0f);
		pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	}
}

void CController::createFireBall(class CEntity *pTarget, const sf::Vector2f &offSet) noexcept
{
	if (!pTarget->getBody() || Game()->Client()->isClipped(CSystemBox2D::b2ToSf(pTarget->getBody()->GetPosition())+offSet, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT, true, CAssetManager::SHADER_BLUR);
	pParticle->m_pTarget = pTarget;
	pParticle->m_SizeInit = sf::Vector2f(32.0f, 32.0f);
	pParticle->m_SizeEnd = sf::Vector2f(64.0f, 64.0f);
	pParticle->m_ColorInit = sf::Color(10, 75, 205);
	pParticle->m_ColorEnd = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd.a = 35;
	pParticle->m_Offset = offSet;
	pParticle->m_Luminance = true;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Duration = 0.75f;
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;
}

void CController::createFireTrailLarge(const sf::Vector2f &worldPos) noexcept
{
	if (!Game()->Client()->canAdd100Hz() || Game()->Client()->isClipped(worldPos, 128.0f))
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
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;

	pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(4.0f, 4.0f);
	pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_ColorInit = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd = sf::Color(251, 216, 4);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
	pParticle->m_Duration = upm::floatRand(0.05f, 0.25f);
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;
}

void CController::createFireTrailSmall(const sf::Vector2f &worldPos) noexcept
{
	if (!Game()->Client()->canAdd100Hz() || Game()->Client()->isClipped(worldPos, 128.0f))
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
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;

	pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(8.0f, 8.0f);
	pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
	pParticle->m_ColorInit = sf::Color(251, 104, 4);
	pParticle->m_ColorEnd = sf::Color(251, 216, 4);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
	pParticle->m_Duration = upm::floatRand(0.001f, 0.05f);
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;
}

void CController::createSmokeCarDamaged(const sf::Vector2f &worldPos, bool fire) noexcept
{
	if (!Game()->Client()->canAdd100Hz() || Game()->Client()->isClipped(worldPos, 128.0f))
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
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;

	if (fire)
	{
		pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
		pParticle->m_Pos = worldPos;
		pParticle->m_SizeInit = sf::Vector2f(4.0f, 4.0f);
		pParticle->m_SizeEnd = sf::Vector2f(12.0f, 12.0f);
		pParticle->m_ColorInit = sf::Color(251, 104, 4);
		pParticle->m_ColorEnd = sf::Color(251, 216, 4);
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
		pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
		pParticle->m_Vel = upm::floatRand(0.001f, 0.01f);
		pParticle->m_Duration = 0.45f;
		pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;
	}
}

void CController::createPoints(const sf::Vector2f &worldPos, int points) noexcept
{
	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_ModeText = true;
	char buff[5];
	if (points >= 0) snprintf(buff, sizeof(buff), "+%d", points);
	else snprintf(buff, sizeof(buff), "%d", points);
	pParticle->m_String = buff;
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(18.0f, 18.0f);
	pParticle->m_SizeEnd = sf::Vector2f(32.0f, 32.0f);
	pParticle->m_ColorInit = sf::Color::Yellow;
	pParticle->m_ColorEnd = (points>=0)?sf::Color::Green:sf::Color::Red;
	pParticle->m_ColorEnd.a = 85;
	pParticle->m_Duration = 0.4f;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-1.0f, 1.0f), upm::floatRand(-1.0f, 1.0f));
	pParticle->m_Vel = upm::floatRand(0.000001f, 0.00001f);
}

void CController::createSmokeImpact(const sf::Vector2f &worldPos, const sf::Vector2f &dir, float vel) noexcept
{
	if (Game()->Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(8.0f, 8.0f);
	pParticle->m_SizeEnd = sf::Vector2f(92.0f, 92.0f);
	pParticle->m_ColorInit = sf::Color(200, 200, 200, 200);
	pParticle->m_ColorEnd = sf::Color::White;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-0.3f:0.3f;
	pParticle->m_Dir = dir;
	pParticle->m_Vel = vel;
	pParticle->m_VelType = CParticle::VEL_LINEAL;
	pParticle->m_Duration = upm::floatRand(0.25f, 0.5f);
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;
}

void CController::createRainBack(const sf::Vector2f &worldPos, float rainVel) noexcept
{
	if (!Game()->Client()->canAdd100Hz())
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_BACK);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(8.0f, 8.0f);
	pParticle->m_SizeEnd = sf::Vector2f(1.0f, 1.0f);
	pParticle->m_ColorInit = pParticle->m_ColorEnd = sf::Color(230,120,255,220);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 3.0f;
	pParticle->m_Vel = rainVel;
	pParticle->m_VelType = CParticle::VEL_LINEAL;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(0.35f, 0.4f), 1.0f);
	pParticle->m_TextId = CAssetManager::TEXTURE_POINT_LIGHT;
}

void CController::createRainFront(const sf::Vector2f &worldPos, float rainVel) noexcept
{
	if (!Game()->Client()->canAdd50Hz())
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(2.0f, 32.0f);
	pParticle->m_SizeEnd = sf::Vector2f(1.0f, 8.0f);
	pParticle->m_ColorInit = pParticle->m_ColorEnd = sf::Color(230,120,255,220);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 3.0f;
	pParticle->m_Vel = rainVel;
	pParticle->m_VelType = CParticle::VEL_LINEAL;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(0.35f, 0.4f), 1.0f);
	pParticle->m_TextId = CAssetManager::TEXTURE_POINT_LIGHT;
}

void CController::createStorm() noexcept
{
	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_Pos = sf::Vector2f(0.0f,0.0f);
	const sf::Vector2f size = sf::Vector2f(Game()->Client()->getView().getSize().x, Game()->Client()->getView().getSize().y);
	pParticle->m_SizeInit = size;
	pParticle->m_SizeEnd = size;
	pParticle->m_ColorInit.a = 80;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_FixedPos = true;
	pParticle->m_Duration = 0.8f;
}

void CController::createSnowBack(const sf::Vector2f &worldPos, float snowVel) noexcept
{
	if (!Game()->Client()->canAdd100Hz() || Game()->Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(8.0f, 8.0f);
	pParticle->m_SizeEnd = sf::Vector2f(1.0f, 1.0f);
	pParticle->m_ColorInit = pParticle->m_ColorEnd = sf::Color(255,255,255,220);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 2.0f;
	pParticle->m_Vel = snowVel;
	pParticle->m_VelType = CParticle::VEL_LINEAL;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-0.25f, 0.4f), 1.0f);
	pParticle->m_TextId = CAssetManager::TEXTURE_POINT_LIGHT;
}

void CController::createSnowFront(const sf::Vector2f &worldPos, float snowVel) noexcept
{
	if (!Game()->Client()->canAdd50Hz() || Game()->Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle = new CParticle(sf::BlendAlpha, RENDER_FOREGROUND);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(18.0f, 18.0f);
	pParticle->m_SizeEnd = sf::Vector2f(1.0f, 1.0f);
	pParticle->m_ColorInit = pParticle->m_ColorEnd = sf::Color(255,255,255,220);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 2.0f;
	pParticle->m_Vel = snowVel;
	pParticle->m_VelType = CParticle::VEL_LINEAL;
	pParticle->m_Dir = sf::Vector2f(upm::floatRand(-0.25f, 0.4f), 1.0f);
	pParticle->m_TextId = CAssetManager::TEXTURE_POINT_LIGHT;
}

void CController::createExplosionCar(const sf::Vector2f &worldPos, bool ring) noexcept
{
	if (Game()->Client()->isClipped(worldPos, 128.0f))
		return;

	CParticle *pParticle;

	if (ring)
	{
		pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
		pParticle->m_Pos = worldPos;
		pParticle->m_SizeInit = sf::Vector2f(26.0f, 26.0f);
		pParticle->m_SizeEnd = sf::Vector2f(95.0f, 95.0f);
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 0.35f;
		pParticle->m_Luminance = true;
		pParticle->m_TextId = CAssetManager::TEXTURE_EXPLOSION_RING;

		pParticle = new CParticle();
		pParticle->m_Pos = worldPos;
		pParticle->m_SizeInit = sf::Vector2f(26.0f, 26.0f);
		pParticle->m_SizeEnd = sf::Vector2f(175.0f, 175.0f);
		pParticle->m_ColorEnd.a = 0;
		pParticle->m_Duration = 0.35f;
		pParticle->m_Luminance = true;
		pParticle->m_TextId = CAssetManager::TEXTURE_EXPLOSION_RING;
	}

	pParticle = new CParticle(sf::BlendAdd, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(32.0f, 32.0f);
	pParticle->m_SizeEnd = sf::Vector2f(272.0f, 272.0f);
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_Duration = 0.6f;
	pParticle->m_TextId = CAssetManager::TEXTURE_EXPLOSION_ANIM;
	pParticle->m_Vel = 0.0f;
	pParticle->m_VelRot = 1.0;
	pParticle->m_Luminance = true;
	pParticle->m_Animated = true;
	pParticle->m_AnimSize = { 4, 4 };

	pParticle = new CParticle(sf::BlendAlpha, RENDER_FRONT);
	pParticle->m_Pos = worldPos;
	pParticle->m_SizeInit = sf::Vector2f(15.0f, 15.0f);
	pParticle->m_SizeEnd = sf::Vector2f(275.0f, 275.0f);
	pParticle->m_ColorInit = sf::Color(125, 125, 125, 120);
	pParticle->m_ColorEnd = sf::Color::Black;
	pParticle->m_ColorEnd.a = 0;
	pParticle->m_VelRot = upm::randInt(0, 2) == 1?-2.45f:2.45f;
	pParticle->m_Duration = 0.4f;
	pParticle->m_TextId = CAssetManager::TEXTURE_SMOKE_WHITE;
}


CLight* CController::createPoint(const sf::Vector2f &worldPos, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(Game()->Client()->Assets().getTexture(CAssetManager::TEXTURE_POINT_LIGHT), worldPos, 0.0f, scale, color, alwaysOn, blink, variationSize, RENDER_FRONT);
	pLight->m_Origin = sf::Vector2f(0.5f, 0.5f);
	return pLight;
}

CLight* CController::createSpot(const sf::Vector2f &worldPos, float degrees, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(Game()->Client()->Assets().getTexture(CAssetManager::TEXTURE_SPOT_LIGHT), worldPos, degrees, scale, color, alwaysOn, blink, variationSize, RENDER_FRONT);
	pLight->m_Origin = sf::Vector2f(0.5f, 1.0f);
	return pLight;
}

CLight* CController::createEmissive(const sf::Vector2f &worldPos, float degrees, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(Game()->Client()->Assets().getTexture(CAssetManager::TEXTURE_EMISSIVE_LIGHT), worldPos, degrees, scale, color, alwaysOn, blink, variationSize, RENDER_FRONT);
	pLight->m_Origin = sf::Vector2f(0.5f, 0.5f);
	return pLight;
}

CLight* CController::createCustom(int textId, const sf::Vector2f &worldPos, const sf::Vector2f &origin, float degrees, const sf::Vector2f &scale, const sf::Color &color, bool alwaysOn, float blink, float variationSize) noexcept
{
	CLight *pLight = new CLight(Game()->Client()->Assets().getTexture(textId), worldPos, degrees, scale, color, alwaysOn, blink, variationSize, RENDER_FRONT);
	pLight->m_Origin = origin;
	return pLight;
}
