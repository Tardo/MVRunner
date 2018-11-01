/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/client/CConfig.hpp>
#include <engine/client/CClient.hpp>
#include "CMap.hpp"

CMap::CMap() noexcept
{
	m_pGameLayer = nullptr;
	m_pGameModifiersLayer = nullptr;
	m_GameLayerIndex = -1;
	m_GameModifiersLayerIndex = -1;
	m_pNoise = nullptr;
	m_pSkipCount = nullptr;
	m_TileLayersCount = 0;
	m_TilesetsCount = 0u;
	m_pShadowsMap = nullptr;
	m_MapLoaded = false;
	m_vpObjects = nullptr;
}
CMap::~CMap() noexcept
{
	reset();
}

bool CMap::loadMap(const char *pMap) noexcept
{
	ups::msgDebug("CMapRender", "Loading map...");
	sf::Clock timeLoadMap;
	reset();

	ParseText(std::string(pMap));
	//m_pMap->ParseFile(path);
    if (HasError())
    {
        ups::msgDebug("CMapRender","TMX Error #%d: %s", GetErrorCode(), GetErrorText().c_str());
        return false;
    }

    m_vpObjects = new CQuadTree<CMapRenderObject*>(sf::FloatRect(0.0f, 0.0f, GetWidth()*GetTileWidth(), GetHeight()*GetTileHeight()));
    m_MapLoaded = init();
	ups::msgDebug("CMapRender", "Map Loaded in %.3f seconds", timeLoadMap.getElapsedTime().asMilliseconds()/1000.0f);
	return m_MapLoaded;
}

bool CMap::init() noexcept
{
	m_pGameLayer = nullptr;
	m_GameLayerIndex = -1;

	// Procesar Imágenes
	{
		CClient *pGame = CClient::getInstance();
		std::vector<Tmx::Tileset*>::const_iterator it = GetTilesets().cbegin();
		while (it != GetTilesets().cend())
		{
			const Tmx::Tileset* pTileSet = (*it);
			ups::msgDebug("CMapRender", "Loading '%s' image...", pTileSet->GetImage()->GetSource().c_str());
			sf::Texture *pTexture = new sf::Texture();
			char aPath[512];
			snprintf(aPath, sizeof(aPath), "data/%s", pTileSet->GetImage()->GetSource().c_str());
			unsigned long fileSize = 0;
			const unsigned char *pData = pGame->Client()->Storage().getFileData(aPath, &fileSize);
			pTexture->loadFromMemory(pData, fileSize);
			m_vpTextures.push_back(pTexture);
			++it;
		}
	}

	// Obtain relevant layers
	{
		std::vector<Tmx::Layer*>::const_iterator itl = GetLayers().cbegin();
		int counter = 0;

		while (itl != GetLayers().cend())
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


	const unsigned int totalMapSize = GetWidth()*GetHeight();
	// Pre-calculate skip values
	{
		unsigned int layerIndex = 0;
		m_pSkipCount = new int*[m_TileLayersCount];
		std::vector<Tmx::Layer*>::const_iterator itl = GetLayers().cbegin();
		while (itl != GetLayers().cend())
		{
			if ((*itl)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
			{
				m_pSkipCount[layerIndex] = new int[totalMapSize];
				for (int y=0; y<GetHeight(); y++)
			    {
					for (int x=0; x<GetWidth(); x++)
					{
						int dd = x+1;
						for (;dd<GetWidth();dd++)
						{
							const int ti = y*GetWidth()+dd;
							const Tmx::MapTile &curTile = static_cast<const Tmx::TileLayer*>((*itl))->GetTile(ti);
							if (curTile.tilesetId == -1)
								continue;

							const Tmx::Tileset *pTileset = GetTileset(curTile.tilesetId);
							const int tileId = curTile.gid-pTileset->GetFirstGid();
							if (tileId>0)
								break;
						}
						const int tileIndex = y*GetWidth()+x;
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
		std::vector<Tmx::Tileset*>::const_iterator itl = GetTilesets().cbegin();
		while (itl != GetTilesets().cend())
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
		std::vector<Tmx::Layer*>::const_iterator it = GetLayers().cbegin();
		while (it != GetLayers().cend())
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

void CMap::reset() noexcept
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

    if (m_pNoise)
    {
    	delete m_pNoise;
    	m_pNoise = nullptr;
    }

    m_MapLoaded = false;
}

sf::IntRect CMap::getMapBounds(const sf::View &camera) noexcept
{
	const sf::Vector2i tileSize(GetTileWidth(), GetTileHeight());
	sf::FloatRect screenArea;
	CClient *pGame = CClient::getInstance();
	pGame->Client()->getViewportGlobalBounds(&screenArea, camera);
	return sf::IntRect(
			upm::clamp((int)(screenArea.left/tileSize.x-1), 0, GetWidth()),
			upm::clamp((int)(screenArea.top/tileSize.y-1), 0, GetHeight()),
			upm::clamp((int)(screenArea.width/tileSize.x+1), 0, GetWidth()),
			upm::clamp((int)(screenArea.height/tileSize.y+1), 0, GetHeight())
	);
}

void CMap::analyzeLayerTile(int layerIndex) noexcept
{
	std::vector<Tmx::Layer*>::const_iterator it = GetLayers().cbegin()+layerIndex;
	const Tmx::TileLayer *pTileLayer = static_cast<const Tmx::TileLayer*>((*it));
	const sf::Vector2i tileSize(GetTileWidth(), GetTileHeight());

    for (int y=0; y<GetHeight(); ++y)
    {
    	for (int x=0; x<GetWidth(); ++x)
		{
    		const sf::Vector2f tilePos = sf::Vector2f(x*tileSize.x, y*tileSize.y);
    		const int tileIndex = y*GetWidth()+x;

			const Tmx::MapTile &curTile = pTileLayer->GetTile(tileIndex);
			if (curTile.tilesetId == -1)
			{
				x += m_pSkipCount[layerIndex][tileIndex];
				continue;
			}

			const Tmx::Tileset *pTileset = GetTileset(curTile.tilesetId);
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
					CMapRenderObject *pObj = new CMapRenderObject(pTile->GetObject(i), tilePos);
					m_vpObjects->insert(pObj->getPosition(), pObj);
				}
			}
		}
    }
}

CMapRenderObject* CMap::getObject(int id) noexcept
{
	std::list<CMapRenderObject*> vObjects = m_vpObjects->queryAll();
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

int CMap::getWorldTileIndex(const sf::Vector2f &worldPos, const Tmx::TileLayer *pLayer) noexcept
{
	sf::Vector2i tpos = getMapPos(worldPos);
	return getMapTileIndex(tpos, pLayer);
}

int CMap::getMapTileIndex(const sf::Vector2i &mapPos, const Tmx::TileLayer *pLayer) noexcept
{
	if (mapPos.x < 0.0f || mapPos.y < 0.0f || mapPos.x >= pLayer->GetWidth() || mapPos.y >= pLayer->GetHeight())
		return -1;

	const Tmx::MapTile &curTile = pLayer->GetTile(mapPos.x, mapPos.y);
	if (curTile.tilesetId < 0)
		return -1;

	const Tmx::Tileset *pTileset = GetTileset(curTile.tilesetId);
	return curTile.gid - pTileset->GetFirstGid();
}

int CMap::getTileDirection(const sf::Vector2i &mapPos) noexcept
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

const sf::Vector2f CMap::getTileDirectionVector(const sf::Vector2i &mapPos) noexcept
{
	return getTileDirectionVector(getTileDirection(mapPos));
}

const sf::Vector2f CMap::getTileDirectionVector(unsigned int tilePos) noexcept
{
	const sf::Vector2f velDirs[4] = {
		{0.0f, -1.0f},
		{0.0f, 1.0f},
		{-1.0f, 0.0f},
		{1.0f, 0.0f}
	};

	return velDirs[tilePos];
}
