/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_MAP
#define H_GAME_MAP

#include <tmxparser/TmxMap.h>
#include <tmxparser/TmxTileLayer.h>
#include <tmxparser/TmxLayer.h>
#include <tmxparser/TmxImage.h>
#include <tmxparser/TmxTileset.h>
#include <tmxparser/TmxObjectGroup.h>
#include <tmxparser/TmxObject.h>
#include <tmxparser/TmxTile.h>
#include <base/system.hpp>
#include <game/CEntity.hpp>
#include <engine/CNoise.hpp>
#include <engine/CQuadTree.hpp>
#include <engine/CSystemLight.hpp>
#include <engine/CSystemLight.hpp>

enum
{
	TILE_UP=0,
	TILE_DOWN,
	TILE_LEFT,
	TILE_RIGHT,

	TILE_SOLID=1,
	TILE_DEAD,
	TILE_STATE_FREEZE=9,
	TILE_STATE_CLEAN=11,
	TILE_TELEPORT_IN=26,
	TILE_TELEPORT_OUT,
	TILE_STATE_ROTATE,
	TILE_SPEED_SOFT=64,
	TILE_SPEED_HARD,

	/** Used in Modifiers Layer **/
	TILE_1=35,
	TILE_2,
	TILE_3,
	TILE_4,
	TILE_5,
	TILE_6,
	TILE_7,
	TILE_8,
	TILE_9,
	TILE_10,
	TILE_11,
	TILE_12,
	TILE_13,
	TILE_14,
	TILE_15,
	TILE_16,
	TILE_17,
	TILE_18,
	TILE_19,
	TILE_20,
	TILE_21,
	TILE_22,
	TILE_23,
	TILE_24,
	TILE_25,

	ENTITY_NULL=0,
	ENTITY_SPAWN_PLAYER,
	ENTITY_SPAWN,
	ENTITY_SPAWN_ARROW,
	ENTITY_SPAWN_CHARACTER,
	ENTITY_SPAWN_BOSS,
	ENTITY_SPAWN_FIRE,
	ENTITY_LASER_HARD=24,
	NUM_ENTITIES,
	ENTITY_OFFSET=255-16*4
};

class CMapRenderObject final
{
public:
	CMapRenderObject() noexcept
	{
		m_pEntity = nullptr;
		m_pLight = nullptr;
		m_pObject = nullptr;
		m_Offset = sf::Vector2f(0.0f, 0.0f);
	}

	CMapRenderObject(const Tmx::Object *pObj, const sf::Vector2f &offset = sf::Vector2f(0.0f, 0.0f)) noexcept
	{
		m_pEntity = nullptr;
		m_pLight = nullptr;
		m_pObject = pObj;
		m_Offset = offset;
	}

	sf::Vector2f getPosition() const noexcept
	{
		if (m_pEntity && m_pEntity->getBody())
			return CSystemBox2D::b2ToSf(m_pEntity->getBody()->GetPosition());
		else if (m_pLight)
			return m_pLight->m_Position;
		else if (m_pObject)
		{
			const sf::FloatRect globalBounds(
				m_pObject->GetX(), m_pObject->GetY(),
				m_pObject->GetWidth(), m_pObject->GetHeight()
			);
			return m_Offset + sf::Vector2f(globalBounds.left+m_pObject->GetWidth()/2.0f, globalBounds.top+m_pObject->GetHeight()/2.0f);
		}

		return VECTOR_ZERO;
	}

	void destroy() noexcept
	{
		if (m_pEntity)
			m_pEntity->destroy();
		if (m_pLight)
			m_pLight->destroy();
	}

	CEntity *m_pEntity;
	CLight *m_pLight;
	sf::Vector2f m_Offset;
	const Tmx::Object *m_pObject;
};

class CTileAnimInfo final
{
public:
	CTileAnimInfo() noexcept
	{
		m_TimerAnim = 0;
		m_CurFrame = 0;
	}

	sf::Uint64 m_TimerAnim;
	int m_CurFrame;
};

class CMap : public Tmx::Map
{
public:
	CMap() noexcept;
	virtual ~CMap() noexcept;

	CQuadTree<CMapRenderObject*>* getObjects() noexcept { return m_vpObjects; }
	bool isMapLoaded() const noexcept { return m_MapLoaded; }

	CMapRenderObject* getObject(int id) noexcept;

	bool loadMap(const char *path) noexcept;
	void reset() noexcept;

	const Tmx::TileLayer* getGameLayer() const noexcept { return m_pGameLayer; }
	const Tmx::TileLayer* getGameModifiersLayer() const noexcept { return m_pGameModifiersLayer; }
	int getGameLayerIndex() const noexcept { return m_GameLayerIndex; }
	int getGameModifiersLayerIndex() const noexcept { return m_GameModifiersLayerIndex; }
	int getWorldTileIndex(const sf::Vector2f &worldPos, const Tmx::TileLayer *pLayer) noexcept;
	int getMapTileIndex(const sf::Vector2i &mapPos, const Tmx::TileLayer *pLayer) noexcept;
	int getTileDirection(const sf::Vector2i &mapPos) noexcept;
	const sf::Vector2f getTileDirectionVector(const sf::Vector2i &mapPos) noexcept;
	const sf::Vector2f getTileDirectionVector(unsigned int tileDir) noexcept;

	sf::IntRect getMapBounds(const sf::View &camera) noexcept;

	const sf::Sprite& getFrontMap(const sf::View &camera, const sf::Color &colorLight = sf::Color::White) noexcept;

	inline const sf::Vector2i getMapPos(const sf::Vector2f &worldPos) const noexcept
	{
		return sf::Vector2i((int)(worldPos.x/GetTileWidth()), (int)(worldPos.y/GetTileHeight()));
	}
	inline const sf::Vector2f getWorldPos(const sf::Vector2i &mapPos) const noexcept
	{
		return sf::Vector2f(mapPos.x*GetTileWidth(), mapPos.y*GetTileHeight());
	}

	int **m_pSkipCount;
	std::map<int, std::map<int, CTileAnimInfo>> m_mmTileAnims;
	std::vector<sf::Texture*> m_vpTextures;

protected:
	bool m_MapLoaded;

private:
	CQuadTree<CMapRenderObject*> *m_vpObjects;
	CPerlinOctave *m_pNoise;
	const Tmx::TileLayer *m_pGameLayer;
	const Tmx::TileLayer *m_pGameModifiersLayer;
	int m_GameLayerIndex;
	int m_GameModifiersLayerIndex;

	int *m_pShadowsMap;
	unsigned int m_TilesetsCount;
	unsigned int m_TileLayersCount;

	sf::RenderTexture m_RenderTexture;
	sf::Sprite m_RenderSprite;

	bool init() noexcept;
	void renderTilemap(const sf::IntRect &mapBounds, int layerIndex, const sf::Color &lightColor, int renderType) noexcept;
	void analyzeLayerTile(int layerIndex) noexcept;
};

#endif
