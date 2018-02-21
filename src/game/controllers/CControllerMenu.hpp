/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_CONTROLLER_MENU
#define H_GAME_CONTROLLER_MENU

#include <engine/CGame.hpp>
#include <game/CContext.hpp>
#include <game/CController.hpp>

class CControllerMenu final : public CController
{
public:
	CControllerMenu() noexcept;
	CControllerMenu(class CContext *pContext) noexcept;
	virtual ~CControllerMenu() noexcept final;

	virtual void tick() noexcept final;
	virtual void updateCamera(float deltaTime) noexcept final;
	virtual bool onInit() noexcept final;
	virtual void onStart() noexcept final;

private:
	sf::Vector2f m_CamPos;
};

#endif
