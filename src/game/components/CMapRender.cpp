/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <SFML/OpenGL.hpp>
#include <base/math.hpp>
#include <engine/client/CClient.hpp>
#include <tmxparser/TmxTile.h>
#include "CMapRender.hpp"
#include <game/CController.hpp>
#include <vector>

CMapRender::CMapRender(CGameClient *pGameClient, int render) noexcept
: CComponent(pGameClient)
{
	m_Render = render;
}
CMapRender::~CMapRender() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
    ups::msgDebug("CMapRender", "Deleted");
	#endif
}

void CMapRender::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (!Client()->Controller() || !Client()->Controller()->Context())
		return;

	target.setView(Client()->Camera());

	CMap &Map = Client()->Controller()->Context()->Map();
	if (!Map.isMapLoaded())
		return;

	const sf::IntRect mapBounds = Map.getMapBounds(Client()->Camera());

	if (Client()->getRenderMode() == RENDER_MODE_NORMAL || Client()->getRenderMode() == RENDER_MODE_NORMALMAP)
	{
		if (m_Render == RENDER_BACK)
		{
			unsigned int layerIndex = 0u;
			std::vector<Tmx::Layer*>::const_iterator it, eit;
			it = Map.GetLayers().cbegin()+layerIndex;
			eit = Map.GetLayers().cbegin()+Map.getGameLayerIndex();
			while (it != eit)
			{
				if ((*it)->IsVisible() && (*it)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
				{
					const sf::Color backColor = CMap::tmxToSf(Map.GetProperties().GetColorProperty("shadow_background_color", Tmx::Color(32, 32, 32, 255)));
					renderTilemap(target, states, mapBounds, layerIndex, backColor);
				}

				++layerIndex;
				++it;
			}
		} else if (m_Render == RENDER_FRONT)
		{
			unsigned int layerIndex = Map.getGameLayerIndex()+1;
			std::vector<Tmx::Layer*>::const_iterator it, eit;
			it = Map.GetLayers().cbegin()+layerIndex;
			eit = Map.GetLayers().cend();
			while (it != eit)
			{
				if ((*it)->IsVisible() && (*it)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
				{
					const sf::Color frontColor = CMap::tmxToSf(Map.GetProperties().GetColorProperty("shadow_foreground_color", Tmx::Color(86, 86, 86, 255)));
					renderTilemap(target, states, mapBounds, layerIndex, frontColor);
				}

				++layerIndex;
				++it;
			}
		}
	}
	else if (Client()->getRenderMode() == RENDER_MODE_LIGHTING || Client()->getRenderMode() == RENDER_MODE_LIQUID)
	{
		if (m_Render == RENDER_FRONT)
		{
			unsigned int layerIndex = Map.getGameLayerIndex()+1;
			std::vector<Tmx::Layer*>::const_iterator it, eit;
			it = Map.GetLayers().cbegin()+layerIndex;
			eit = Map.GetLayers().cend();
			while (it != eit)
			{
				if ((*it)->IsVisible() && (*it)->GetLayerType() == Tmx::TMX_LAYERTYPE_TILE)
					renderTilemap(target, states, mapBounds, layerIndex, sf::Color::Black);

				++layerIndex;
				++it;
			}
		}
	}
}

void CMapRender::renderTilemap(sf::RenderTarget& target, sf::RenderStates states, const sf::IntRect &mapBounds, int layerIndex, const sf::Color &lightColor) const noexcept
{
	CMap &Map = Client()->Controller()->Context()->Map();
	std::vector<Tmx::Layer*>::const_iterator it = Map.GetLayers().cbegin()+layerIndex;
	const Tmx::TileLayer *pTileLayer = static_cast<const Tmx::TileLayer*>((*it));
	const sf::Vector2i tileSize(Map.GetTileWidth(), Map.GetTileHeight());

	sf::VertexArray tilemap(sf::Quads, 4);

    for (int y=mapBounds.top; y<mapBounds.height; ++y)
    {
    	for (int x=mapBounds.left; x<mapBounds.width; ++x)
		{
    		const sf::Vector2f tilePos = sf::Vector2f(x*tileSize.x, y*tileSize.y);
    		const int tileIndex = y*Map.GetWidth()+x;

			const Tmx::MapTile &curTile = pTileLayer->GetTile(tileIndex);
			if (curTile.tilesetId == -1)
			{
				x += Map.m_pSkipCount[layerIndex][tileIndex];
				continue;
			}

			const Tmx::Tileset *pTileset = Map.GetTileset(curTile.tilesetId);
			int tileId = curTile.gid-pTileset->GetFirstGid();
			if (tileId<=0)
			{
				x += Map.m_pSkipCount[layerIndex][tileIndex];
				continue;
			}

			const Tmx::Tile *pTile = pTileset->GetTile(tileId);

			bool luminance = false;
			bool block_light = true;
			if (pTile)
			{
				luminance = pTile->GetProperties().GetBoolProperty("luminance", false);
				block_light = pTile->GetProperties().GetBoolProperty("block_light", true);
				if ((luminance && Client()->getRenderMode() != RENDER_MODE_LIGHTING))
					continue;
				else if ((Client()->getRenderMode() == RENDER_MODE_LIQUID || Client()->getRenderMode() == RENDER_MODE_LIGHTING) && !luminance && !block_light)
					continue;
			}

			if (pTile && pTile->IsAnimated() && pTile->GetFrameCount())
			{
				CTileAnimInfo *pTileAnimInfo = &(*(*Map.m_mmTileAnims.find(curTile.tilesetId)).second.find(tileId)).second;
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

			const sf::Color colorQuad = (luminance && Client()->getRenderMode() == RENDER_MODE_LIGHTING)?sf::Color::White:lightColor;

			vQuad[0].color = colorQuad;
			vQuad[1].color = colorQuad;
			vQuad[2].color = colorQuad;
			vQuad[3].color = colorQuad;

			const sf::Texture *pTexture = nullptr;
			if (Client()->getRenderMode() == RENDER_MODE_NORMALMAP)
			{

				const unsigned int normalmap_id = pTileLayer->GetProperties().GetIntProperty("normalmap_id", -1);
				if (normalmap_id >= 0)
					pTexture = Map.m_vpTextures[normalmap_id];
			}
			else
				pTexture = Map.m_vpTextures[curTile.tilesetId];
			if (pTexture)
			{
				const int dd = pTexture->getSize().x/tileSize.x;
				const int tu = tileId%dd;
				const int tv = tileId/dd;

				vQuad[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
				vQuad[1].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
				vQuad[2].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
				vQuad[3].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
				states.texture = pTexture;
				target.draw(tilemap, states);
				states.texture = nullptr;
			}
			else
				target.draw(tilemap, states);

			x += Map.m_pSkipCount[layerIndex][tileIndex];
		}
    }
}
