/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <base/system.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CLocalization.hpp>
#include <game/CContext.hpp>
#include "CControllerMenu.hpp"
#include <cstring>
#include <engine/CSystemSound.hpp>

CControllerMenu::CControllerMenu() noexcept
: CController(),
  m_CamPos(0.0f, 0.0f)
{ }
CControllerMenu::CControllerMenu(class CContext *pContext) noexcept
: CController(pContext),
  m_CamPos(0.0f, 0.0f)
{ }
CControllerMenu::~CControllerMenu() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CControllerMenu", "Deleted");
	#endif
}

void CControllerMenu::tick() noexcept
{
	CController::tick();
}

void CControllerMenu::updateCamera(float deltaTime) noexcept
{
	Game()->Client()->Camera().setCenter(m_CamPos+sf::Vector2f(cosf(Game()->Client()->getElapsedTime()*0.25f)*64.0f, sinf(Game()->Client()->getElapsedTime()*0.25f)*64.0f));
}

bool CControllerMenu::onInit() noexcept
{
	return CController::onInit();
}

void CControllerMenu::onStart() noexcept
{
	CController::onStart();
	Game()->Client()->getSystem<CSystemSound>()->playBackgroundMusic(CAssetManager::MUSIC_MAIN_MENU);
}

