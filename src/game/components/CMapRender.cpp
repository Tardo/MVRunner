/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <SFML/OpenGL.hpp>
#include <base/math.hpp>
#include <engine/CGame.hpp>
#include <engine/CSystemWeather.hpp>
#include <tmxparser/TmxTile.h>
#include "CMapRender.hpp"
#include <game/CController.hpp>
#include <vector>

CMapRender::CMapRender() noexcept
: CComponent()
{
	m_pMap = nullptr;
	m_pGameLayer = nullptr;
	m_pGameModifiersLayer = nullptr;
	m_GameLayerIndex = -1;
	m_GameModifiersLayerIndex = -1;
	m_vpObjects = nullptr;
	m_pNoise = nullptr;
	m_pSkipCount = nullptr;
	m_TileLayersCount = 0;
	m_TilesetsCount = 0u;
	m_MapLoaded = false;
	m_pShadowsMap = nullptr;
}
CMapRender::~CMapRender() noexcept
{
	reset();
	#ifdef DEBUG_DESTRUCTORS
    ups::msgDebug("CMapRender", "Deleted");
	#endif
}

bool CMapRender::loadMap(const char *pMap) noexcept
{
	ups::msgDebug("CMapRender", "Loading map...");
	sf::Clock timeLoadMap;
	reset();

	m_pMap = new Tmx::Map();
	m_pMap->ParseText(std::string(pMap));
	//m_pMap->ParseFile(path);
    if (m_pMap->HasError())
    {
        ups::msgDebug("CMapRender","TMX Error #%d: %s", m_pMap->GetErrorCode(), m_pMap->GetErrorText().c_str());
        return false;
    }

    m_vpObjects = new CQuadTree<CMapRenderObject*>(sf::FloatRect(0.0f, 0.0f, m_pMap->GetWidth()*m_pMap->GetTileWidth(), m_pMap->GetHeight()*m_pMap->GetTileHeight()));
    m_MapLoaded = init();
	ups::msgDebug("CMapRender", "Map Loaded in %.3f seconds", timeLoadMap.getElapsedTime().asMilliseconds()/1000.0f);
	return m_MapLoaded;
}

bool CMapRender::init() noexcept
{
	m_pGameLayer = nullptr;
	m_GameLayerIndex = -1;

	// Crear lienzos
	if (!m_RenderTexture.create(g_Config.m_ScreenWidth, g_Config.m_ScreenHeight))
		return false;
	m_RenderSprite.setTexture(m_RenderTexture.getTexture());

	// Procesar Imágenes
	{
		std::vector<Tmx::Tileset*>::const_iterator it = m_pMap->GetTilesets().cbegin();
		while (it != m_pMap->GetTilesets().cend())
		{
			const Tmx::Tileset* pTileSet = (*it);
			ups::msgDebug("CMapRender", "Loading '%s' image...", pTileSet->GetImage()->GetSource().c_str());
			sf::Texture *pTexture = new sf::Texture();
			char aPath[512];
			snprintf(aPath, sizeof(aPath), "data/%s", pTileSet->GetImage()->GetSource().c_str());
			unsigned long fileSize = 0;
			const unsigned char *pData = Client()->Storage().getFileData(aPath, &fileSize);
			pTexture->loadFromMemory(pData, fileSize);
			m_vpTextures.push_back(pTexture);
			++it;
		}
	}

	// Obtain relevant layers
	{
		std::vector<Tmx::Layer*>::const_iterator itl = m_pMap->GetLayers().cbegin();
		int counter = 0;

		while (itl != m_pMap->GetLayers().cend())
		{
			if ((*itl)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
			{
				if  ((*itl)->GetName().compare("Game") == 0)
				{
					m_pGameLayer = (const Tmx::TileLayer*)(*itl);
					m_GameLayerIndex = counter;
				}
				else if ((*itl)->GetName().compare("GameModifiers") == 0)
				{
					m_pGameModifiersLayer = (const Tmx::TileLayer*)(*itl);
					m_GameModifiersLayerIndex = counter;
				}

				++m_TileLayersCount;
			}
			else if ((*itl)->GetLayerType() == Tmx::TMX_LAYERTYPE_OBJECTGROUP)
			{
				const Tmx::ObjectGroup *pObjGroup = (const Tmx::ObjectGroup*)(*itl);
				std::vector<Tmx::Object*>::const_iterator itob = pObjGroup->GetObjects().begin();
				while (itob != pObjGroup->GetObjects().end())
				{
					CMapRenderObject *pObj = new CMapRenderObject(*itob);
					m_vpObjects->insert(pObj->getPosition(), pObj);

					++itob;
				}
			}

			++counter;
			++itl;
		}
	}


	const unsigned int totalMapSize = m_pMap->GetWidth()*m_pMap->GetHeight();
	// Pre-calculate skip values
	{
		unsigned int layerIndex = 0;
		m_pSkipCount = new int*[m_TileLayersCount];
		std::vector<Tmx::Layer*>::const_iterator itl = m_pMap->GetLayers().cbegin();
		while (itl != m_pMap->GetLayers().cend())
		{
			if ((*itl)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
			{
				m_pSkipCount[layerIndex] = new int[totalMapSize];
				for (int y=0; y<m_pMap->GetHeight(); y++)
			    {
					for (int x=0; x<m_pMap->GetWidth(); x++)
					{
						int dd = x+1;
						for (;dd<m_pMap->GetWidth();dd++)
						{
							const int ti = y*m_pMap->GetWidth()+dd;
							const Tmx::MapTile &curTile = static_cast<const Tmx::TileLayer*>((*itl))->GetTile(ti);
							if (curTile.tilesetId == -1)
								continue;

							const Tmx::Tileset *pTileset = m_pMap->GetTileset(curTile.tilesetId);
							const int tileId = curTile.gid-pTileset->GetFirstGid();
							if (tileId>0)
								break;
						}
						const int tileIndex = y*m_pMap->GetWidth()+x;
						m_pSkipCount[layerIndex][tileIndex] = dd-x-1;
					}
			    }
				++layerIndex;
			}

			++itl;
		}
	}

	// Initialize Tile Animations
	{
		std::vector<Tmx::Tileset*>::const_iterator itl = m_pMap->GetTilesets().cbegin();
		while (itl != m_pMap->GetTilesets().cend())
		{
			std::map<int, CTileAnimInfo> mTileAnims;
			std::vector<Tmx::Tile*>::const_iterator itt = (*itl)->GetTiles().cbegin();
			while (itt != (*itl)->GetTiles().cend())
			{
				CTileAnimInfo tileInfo;
				tileInfo.m_TimerAnim = ups::timeGet();
				mTileAnims.insert(std::make_pair((*itt)->GetId(), tileInfo));
				++itt;
			}

			m_mmTileAnims.insert(std::make_pair(m_TilesetsCount, mTileAnims));
			++m_TilesetsCount;
			++itl;
		}
	}

	// Analyze Layers
	{
		unsigned int layerIndex = 0u;
		std::vector<Tmx::Layer*>::const_iterator it = m_pMap->GetLayers().cbegin();
		while (it != m_pMap->GetLayers().cend())
		{
			if ((*it)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
				analyzeLayerTile(layerIndex);

			++layerIndex;
			++it;
		}
	}


	m_pNoise = new CPerlinOctave(7, 1234);

	ups::msgDebug("CMapRender", "Num. Mapres: %d", m_vpTextures.size());
	ups::msgDebug("CMapRender", "Num. Objects: %d", m_vpObjects->queryAll().size());
	return true;
}

void CMapRender::update(float deltaTime) noexcept
{ }

void CMapRender::reset() noexcept
{
	std::vector<sf::Texture*>::iterator it = m_vpTextures.begin();
	while (it != m_vpTextures.end())
	{
		delete (*it);
		*it = nullptr;
		++it;
	}
	m_vpTextures.clear();

	if (m_vpObjects)
	{
		std::list<CMapRenderObject*> vObjects = m_vpObjects->queryAll();
		std::list<CMapRenderObject*>::const_iterator ito = vObjects.cbegin();
		while (ito != vObjects.cend())
		{
			CMapRenderObject *pObj = (*ito);
			pObj->destroy();
			delete pObj;
			pObj = nullptr;
			++ito;
		}
		m_vpObjects->clear();
		delete m_vpObjects;
		m_vpObjects = nullptr;
	}

    if (m_pSkipCount)
	{
		for (unsigned int i=0; i<m_TileLayersCount; i++)
			delete [] m_pSkipCount[i];
		delete [] m_pSkipCount;
	}
    m_pSkipCount = nullptr;
    m_TileLayersCount = 0u;

    std::map<int, std::map<int, CTileAnimInfo>>::iterator itta = m_mmTileAnims.begin();
    while (itta != m_mmTileAnims.end())
    	(*itta++).second.clear();
    m_mmTileAnims.clear();
    m_TilesetsCount = 0u;

    if (m_pMap)
    {
    	delete m_pMap;
    	m_pMap = nullptr;
    }

    if (m_pNoise)
    {
    	delete m_pNoise;
    	m_pNoise = nullptr;
    }

    m_MapLoaded = false;
}

const sf::Sprite& CMapRender::getBackMap(const sf::View &camera, const sf::Color &colorLight, bool normal) noexcept
{
	m_RenderTexture.clear(sf::Color::Transparent);
	m_RenderTexture.setView(camera);

	const sf::IntRect mapBounds = getMapBounds(camera);

	unsigned int layerIndex = 0u;
	std::vector<Tmx::Layer*>::const_iterator it, eit;
	it = m_pMap->GetLayers().cbegin()+layerIndex;
	eit = m_pMap->GetLayers().cbegin()+m_GameLayerIndex;
	while (it != eit)
	{
		//const Tmx::PropertySet &tileProps = (*it)->GetProperties();
		if ((*it)->IsVisible() && (*it)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
		{
			renderTilemap(mapBounds, layerIndex, colorLight, normal?NORMAL:DEFAULT);
		}

		++layerIndex;
		++it;
	}

	m_RenderTexture.display();
	return m_RenderSprite;
}


const sf::Sprite& CMapRender::getFrontMap(const sf::View &camera, const sf::Color &colorLight) noexcept
{
	m_RenderTexture.clear(sf::Color::Transparent);
	m_RenderTexture.setView(camera);

	const sf::IntRect mapBounds = getMapBounds(camera);

	unsigned int layerIndex = m_GameLayerIndex+1;
	std::vector<Tmx::Layer*>::const_iterator it, eit;
	it = m_pMap->GetLayers().cbegin()+layerIndex;
	eit = m_pMap->GetLayers().cend();
	while (it != eit)
	{
		if ((*it)->IsVisible() && (*it)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
		{
			renderTilemap(mapBounds, layerIndex, colorLight, DEFAULT);
		}

		++layerIndex;
		++it;
	}

	m_RenderTexture.display();
	return m_RenderSprite;
}

sf::IntRect CMapRender::getMapBounds(const sf::View &camera) noexcept
{
	const sf::Vector2i tileSize(m_pMap->GetTileWidth(), m_pMap->GetTileHeight());
	sf::FloatRect screenArea;
	Client()->getViewportGlobalBounds(&screenArea, camera);
	return sf::IntRect(
			upm::clamp((int)(screenArea.left/tileSize.x-1), 0, m_pMap->GetWidth()),
			upm::clamp((int)(screenArea.top/tileSize.y-1), 0, m_pMap->GetHeight()),
			upm::clamp((int)(screenArea.width/tileSize.x+1), 0, m_pMap->GetWidth()),
			upm::clamp((int)(screenArea.height/tileSize.y+1), 0, m_pMap->GetHeight())
	);
}

void CMapRender::analyzeLayerTile(int layerIndex) noexcept
{
	std::vector<Tmx::Layer*>::const_iterator it = m_pMap->GetLayers().cbegin()+layerIndex;
	const Tmx::TileLayer *pTileLayer = static_cast<const Tmx::TileLayer*>((*it));
	const sf::Vector2i tileSize(m_pMap->GetTileWidth(), m_pMap->GetTileHeight());

	ups::msgDebug("TT", "Analizando Mapa");

    for (int y=0; y<m_pMap->GetHeight(); ++y)
    {
    	for (int x=0; x<m_pMap->GetWidth(); ++x)
		{
    		const sf::Vector2f tilePos = sf::Vector2f(x*tileSize.x, y*tileSize.y);
    		const int tileIndex = y*m_pMap->GetWidth()+x;

			const Tmx::MapTile &curTile = pTileLayer->GetTile(tileIndex);
			if (curTile.tilesetId == -1)
			{
				x += m_pSkipCount[layerIndex][tileIndex];
				continue;
			}

			const Tmx::Tileset *pTileset = m_pMap->GetTileset(curTile.tilesetId);
			int tileId = curTile.gid-pTileset->GetFirstGid();
			if (tileId<=0)
			{
				x += m_pSkipCount[layerIndex][tileIndex];
				continue;
			}

			const Tmx::Tile *pTile = pTileset->GetTile(tileId);
			if (pTile && pTile->HasObjects())
			{
				for (int i=0; i<pTile->GetNumObjects(); ++i)
				{
					ups::msgDebug("TT", "Creando Tile");
					CMapRenderObject *pObj = new CMapRenderObject(pTile->GetObject(i), tilePos);
					m_vpObjects->insert(pObj->getPosition(), pObj);
				}
			}
		}
    }
}

void CMapRender::renderTilemap(const sf::IntRect &mapBounds, int layerIndex, const sf::Color &lightColor, int renderType) noexcept
{
	std::vector<Tmx::Layer*>::const_iterator it = m_pMap->GetLayers().cbegin()+layerIndex;
	const Tmx::TileLayer *pTileLayer = static_cast<const Tmx::TileLayer*>((*it));
	const sf::Vector2i tileSize(m_pMap->GetTileWidth(), m_pMap->GetTileHeight());

	sf::VertexArray tilemap(sf::Quads, 4);

    for (int y=mapBounds.top; y<mapBounds.height; ++y)
    {
    	for (int x=mapBounds.left; x<mapBounds.width; ++x)
		{
    		const sf::Vector2f tilePos = sf::Vector2f(x*tileSize.x, y*tileSize.y);
    		const int tileIndex = y*m_pMap->GetWidth()+x;

			const Tmx::MapTile &curTile = pTileLayer->GetTile(tileIndex);
			if (curTile.tilesetId == -1)
			{
				x += m_pSkipCount[layerIndex][tileIndex];
				continue;
			}

			const Tmx::Tileset *pTileset = m_pMap->GetTileset(curTile.tilesetId);
			int tileId = curTile.gid-pTileset->GetFirstGid();
			if (tileId<=0)
			{
				x += m_pSkipCount[layerIndex][tileIndex];
				continue;
			}

			const Tmx::Tile *pTile = pTileset->GetTile(tileId);
			if (pTile && pTile->IsAnimated() && pTile->GetFrameCount())
			{
				CTileAnimInfo *pTileAnimInfo = &(*(*m_mmTileAnims.find(curTile.tilesetId)).second.find(tileId)).second;
				const Tmx::AnimationFrame &animFrame = pTile->GetFrames()[pTileAnimInfo->m_CurFrame];
				if (ups::timeGet()-pTileAnimInfo->m_TimerAnim > ups::timeFreq()*animFrame.GetDuration()*0.001f)
				{
					if (++pTileAnimInfo->m_CurFrame >= pTile->GetFrameCount())
						pTileAnimInfo->m_CurFrame = 0;
					pTileAnimInfo->m_TimerAnim = ups::timeGet();
				}

				tileId = animFrame.GetTileID();
			}

			sf::Vertex* vQuad = &tilemap[0];

			vQuad[0].position = tilePos;
			vQuad[1].position = sf::Vector2f(tilePos.x, (tilePos.y+tileSize.y)); //4
			vQuad[2].position = sf::Vector2f((tilePos.x+tileSize.x), (tilePos.y+tileSize.y)); // 1
			vQuad[3].position = sf::Vector2f((tilePos.x+tileSize.x), tilePos.y); // 2

			if (curTile.flippedVertically)
			{
				sf::Vector2f Tmp = vQuad[0].position;
				vQuad[0].position = vQuad[1].position;
				vQuad[1].position = Tmp;
				Tmp = vQuad[2].position;
				vQuad[2].position = vQuad[3].position;
				vQuad[3].position = Tmp;
			}
			if (curTile.flippedHorizontally)
			{
				sf::Vector2f Tmp = vQuad[0].position;
				vQuad[0].position = vQuad[3].position;
				vQuad[3].position = Tmp;
				Tmp = vQuad[1].position;
				vQuad[1].position = vQuad[2].position;
				vQuad[2].position = Tmp;
			}
			if (curTile.flippedDiagonally)
			{
				sf::Vector2f Tmp = vQuad[1].position;
				vQuad[1].position = vQuad[3].position;
				vQuad[3].position = Tmp;
			}

			sf::Color colorQuad = lightColor;
			if (renderType == DEFAULT && layerIndex > m_GameLayerIndex && m_pGameClient->Controller() && m_pGameClient->Controller()->m_pPlayerLight)
			{
				const float distLigth = (0.6f - upm::clamp(upm::vectorLength(m_pGameClient->Controller()->m_pPlayerLight->m_Position - tilePos)/m_pGameClient->Controller()->m_pPlayerLight->getLocalBounds().width, 0.0f, 0.6f));
				colorQuad = sf::Color(upm::max((sf::Uint8)(255*distLigth), lightColor.r),
									  upm::max((sf::Uint8)(255*distLigth), lightColor.g),
									  upm::max((sf::Uint8)(255*distLigth), lightColor.b));
			}

			vQuad[0].color = colorQuad;
			vQuad[1].color = colorQuad;
			vQuad[2].color = colorQuad;
			vQuad[3].color = colorQuad;

			const sf::Texture *pTexture = m_vpTextures[(renderType==NORMAL && curTile.tilesetId != 0)?curTile.tilesetId+1:curTile.tilesetId];
			if (pTexture)
			{
				const int dd = pTexture->getSize().x/tileSize.x;
				const int tu = tileId%dd;
				const int tv = tileId/dd;

				vQuad[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
				vQuad[1].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
				vQuad[2].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
				vQuad[3].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
				m_RenderTexture.draw(tilemap, pTexture);
				//renderTile3D(sf::Vector2i(x, y), pTexture, tileId);
			}
			else
				m_RenderTexture.draw(tilemap);

			x += m_pSkipCount[layerIndex][tileIndex];
		}
    }
}

CMapRenderObject* CMapRender::getObject(int id) noexcept
{
	std::list<CMapRenderObject*> vObjects = Client()->MapRender().getObjects()->queryAll();
	std::list<CMapRenderObject*>::const_iterator cit = vObjects.cbegin();
	while (cit != vObjects.cend())
	{
		if ((*cit)->m_pObject->GetId() == id)
		{
			return (*cit);
			break;
		}
		++cit;
	}

	return nullptr;
}

int CMapRender::getWorldTileIndex(const sf::Vector2f &worldPos, const Tmx::TileLayer *pLayer) noexcept
{
	sf::Vector2i tpos = getMapPos(worldPos);
	return getMapTileIndex(tpos, pLayer);
}

int CMapRender::getMapTileIndex(const sf::Vector2i &mapPos, const Tmx::TileLayer *pLayer) noexcept
{
	if (mapPos.x < 0.0f || mapPos.y < 0.0f || mapPos.x >= pLayer->GetWidth() || mapPos.y >= pLayer->GetHeight())
		return -1;

	const Tmx::MapTile &curTile = pLayer->GetTile(mapPos.x, mapPos.y);
	if (curTile.tilesetId < 0)
		return -1;

	const Tmx::Tileset *pTileset = m_pMap->GetTileset(curTile.tilesetId);
	return curTile.gid - pTileset->GetFirstGid();
}

int CMapRender::getTileDirection(const sf::Vector2i &mapPos) noexcept
{
	const Tmx::MapTile *pMapTile = &m_pGameLayer->GetTile(mapPos.x, mapPos.y);
	if (pMapTile->flippedVertically && pMapTile->flippedHorizontally && pMapTile->flippedDiagonally)
		return TILE_RIGHT;
	else if (pMapTile->flippedHorizontally && pMapTile->flippedDiagonally)
		return TILE_RIGHT;
	else if (pMapTile->flippedVertically && pMapTile->flippedDiagonally)
		return TILE_LEFT;
	else if (pMapTile->flippedVertically)
		return TILE_DOWN;
	else if (pMapTile->flippedDiagonally)
		return TILE_LEFT;

	return TILE_UP;
}

const sf::Vector2f CMapRender::getTileDirectionVector(const sf::Vector2i &mapPos) noexcept
{
	return getTileDirectionVector(getTileDirection(mapPos));
}

const sf::Vector2f CMapRender::getTileDirectionVector(unsigned int tilePos) noexcept
{
	const sf::Vector2f velDirs[4] = {
		{0.0f, -1.0f},
		{0.0f, 1.0f},
		{-1.0f, 0.0f},
		{1.0f, 0.0f}
	};

	return velDirs[tilePos];
}
