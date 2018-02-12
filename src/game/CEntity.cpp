/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CEntity.hpp"
#include <base/math.hpp>
#include <base/system.hpp>
#include <engine/CGame.hpp>
#include <game/CGameClient.hpp>


CEntity::CEntity(int type, const sf::Color color)
{
	m_Id = -1;
	m_Health = -1;
	m_Type = type;
	m_ToDelete = false;
	m_TimerStateAction = 0;
	m_pBody = nullptr;
	m_Color = color;
	m_ContactFx = FX_NONE;
}

CEntity::~CEntity()
{
	if (m_pBody)
	{
		CGame *pGame = CGame::getInstance();
		pGame->Client()->getSystem<CSystemBox2D>()->destroyBody(m_pBody);
		m_pBody = nullptr;
	}
}


