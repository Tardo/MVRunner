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
	m_TextureId = -1;
	m_ContactFx = FX_NONE;
	m_SelfDelete = true;

	CGame *pGame = CGame::getInstance();
	m_Id = pGame->Client()->Controller()->Context()->addEntity(this);
}

CEntity::~CEntity()
{
	if (m_pBody)
	{
		CGame *pGame = CGame::getInstance();
		CSystemBox2D *pSystemBox2D = pGame->Client()->getSystem<CSystemBox2D>();
		if (pGame->Client()->Controller()->Context()->getPlayer()->getCharacter() && !pGame->Client()->Controller()->Context()->getPlayer()->getCharacter()->isToDelete())
		{
			b2Joint *pJoint = pGame->Client()->Controller()->Context()->getPlayer()->getCharacter()->m_pHookJoint;
			if (pJoint && (pJoint->GetBodyA() == m_pBody || pJoint->GetBodyB() == m_pBody))
			{
				pSystemBox2D->destroyJoint(pJoint);
				pGame->Client()->Controller()->Context()->getPlayer()->getCharacter()->m_pHookJoint = nullptr;
				pGame->Client()->Controller()->Context()->getPlayer()->getCharacter()->m_HookState = CCharacter::HOOK_STATE_RETRACTING;
			}
		}
		pSystemBox2D->destroyBody(m_pBody);
		m_pBody = nullptr;
	}
}


