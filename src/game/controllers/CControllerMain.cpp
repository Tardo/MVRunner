/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <base/system.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>
#include <engine/CSystemLight.hpp>
#include <engine/CSystemSound.hpp>
#include <game/entities/CCharacter.hpp>
#include <game/entities/CFire.hpp>
#include <game/entities/CProjectile.hpp>
#include <game/entities/primitives/CB2Circle.hpp>
#include <game/controllers/CControllerMain.hpp>
#include <game/CContext.hpp>
#include <cstring>

CControllerMain::CControllerMain() noexcept
: CController()
{
	m_TimerArrow = ups::timeGet();
	m_Timer10Hz = ups::timeGet();
	m_Timer15Hz = ups::timeGet();
	m_Add10Hz = false;
}
CControllerMain::CControllerMain(class CContext *pContext) noexcept
: CController(pContext)
{
	m_TimerArrow = ups::timeGet();
	m_Timer10Hz = ups::timeGet();
	m_Timer15Hz = ups::timeGet();
	m_Add10Hz = false;
}
CControllerMain::~CControllerMain() noexcept
{
	Game()->Client()->getSystem<CSystemSound>()->stopBackgroundMusic();

	m_vSpawnsCharacter.clear();
	m_vTeleports.clear();

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CControllerTest", "Deleted");
	#endif
}

void CControllerMain::onSystemEvent(sf::Event *pEvent) noexcept
{ }

void CControllerMain::tick() noexcept
{
	CController::tick();

	CPlayer *pMainPlayer = Context()->getPlayer();

	const sf::Int64 CTime = ups::timeGet();
	static const sf::Int64 time10Hz = ups::timeFreq()/10;
	if(CTime - m_Timer10Hz > time10Hz)
	{
		m_Add10Hz = true;
		m_Timer10Hz = CTime;
	}
	else
		m_Add10Hz = false;


	if (Game()->Client()->hasFocus() && Game()->Client()->Menus().getActive() == CMenus::NONE)
	{
		static bool pressedButtonUse = false;
		//static bool pressedButtonLight = false;

		if (pMainPlayer->getCharacter())
		{
			CCharacter *pChar = pMainPlayer->getCharacter();
			if (pChar && pChar->isAlive())
			{
				const sf::Vector2f &charPos = pChar->getShape()->getPosition();
				const int tileId = Game()->Client()->MapRender().getMapTileIndex(Game()->Client()->MapRender().getMapPos(charPos), Game()->Client()->MapRender().getGameLayer());

				// If the camera is in travel not execute player commands
				if (!(Game()->Client()->Camera().getStatus()&CCamera::TRAVEL))
				{
					// Player Character Movement
					upm::degToDir(pChar->getShape()->getRotation());
					sf::Vector2f dir = upm::vectorNormalize(Game()->Client()->mapPixelToCoords(Game()->Client()->m_MousePosition, Game()->Client()->Camera()) - charPos);
					pChar->getBody()->SetTransform(pChar->getBody()->GetPosition(), upm::degToRad(upm::vectorAngle(dir)+90.0f));
					bool turbo = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

					int moveState = CCharacter::MOVE_STATE_STOP;
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
						moveState = CCharacter::MOVE_STATE_UP;
					/*else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
						moveState = CCharacter::MOVE_STATE_DOWN;*/
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
						moveState |= CCharacter::MOVE_STATE_LEFT;
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
						moveState |= CCharacter::MOVE_STATE_RIGHT;

					if (moveState != CCharacter::MOVE_STATE_STOP)
						pChar->move(moveState, turbo);

					// Player Use Action
					if (!pressedButtonUse && sf::Keyboard::isKeyPressed(sf::Keyboard::E))
					{
						const sf::Vector2f charEndPos = charPos+dir*(CCharacter::SIZE+g_Config.m_CharacterHitDistance);
						CEntity *pEnt = Game()->Client()->getSystem<CSystemBox2D>()->checkIntersectLine(charPos, charEndPos);
						if (pEnt)
						{
							// TODO: Implement Usable Stuff
						}
						pressedButtonUse = true;
					}
					else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E))
						pressedButtonUse = false;

					// Player Select Weapon
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0))
						pChar->setActiveWeapon(-1);
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
					{
						pChar->giveWeapon(WEAPON_GRENADE_LAUNCHER, 10, 10);
						pChar->setActiveWeapon(WEAPON_GRENADE_LAUNCHER);
					}

					// Player Shoot
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
						pChar->doFire();

					// Player Light
					if (m_pPlayerLight)
						m_pPlayerLight->m_Position = charPos;
				}

				// Interact with the map environment
				if (tileId == TILE_TELEPORT_IN)
				{
					const int modifierId = Game()->Client()->MapRender().getMapTileIndex(Game()->Client()->MapRender().getMapPos(charPos), Game()->Client()->MapRender().getGameModifiersLayer());
					int teleId = -1;
					if (modifierId >= TILE_1 && modifierId <= TILE_25)
						teleId = (modifierId-TILE_1)+1;
					std::map<int, CTeleport>::iterator it = m_vTeleports.find(teleId);
					if (it != m_vTeleports.cend())
					{
						Game()->Client()->Camera().moveTo((*it).second.m_PosOut, 2.0f);
						pChar->getBody()->SetTransform(CSystemBox2D::sfToB2((*it).second.m_PosOut), pChar->getBody()->GetAngle());
					}
				}
			}
		}
	}
}

void CControllerMain::onStart() noexcept
{
	CController::onStart();
	Game()->Client()->getSystem<CSystemSound>()->playBackgroundMusic(CAssetManager::MUSIC_FIRST_FLOOR);
}

bool CControllerMain::onMapTile(unsigned int tileId, const sf::Vector2f &pos, unsigned int tileDir, unsigned int modifierId) noexcept
{
	if (tileId < ENTITY_OFFSET)
	{
		if (tileId == TILE_TELEPORT_IN || tileId == TILE_TELEPORT_OUT)
		{
			int teleId = -1;
			if (modifierId >= TILE_1 && modifierId <= TILE_25)
				teleId = (modifierId-TILE_1)+1;
			if (teleId > 0)
			{
				std::map<int, CTeleport>::iterator it = m_vTeleports.find(teleId);
				if (it != m_vTeleports.cend())
				{
					if (tileId == TILE_TELEPORT_OUT)
						(*it).second.m_PosOut = pos;
					else
						(*it).second.m_Pos = pos;
				}
				else
				{
					std::pair<std::map<int, CTeleport>::iterator, bool> rec = m_vTeleports.insert(std::make_pair(teleId, CTeleport(Game()->Client()->MapRender().getTileDirectionVector(tileDir))));
					if (tileId == TILE_TELEPORT_OUT)
						(*rec.first).second.m_PosOut = pos;
					else
						(*rec.first).second.m_Pos = pos;
				}
				return true;
			}
		}
	}
	else
	{
		const unsigned int entityId = tileId - ENTITY_OFFSET;
		if (entityId == ENTITY_SPAWN_CHARACTER)
		{
			m_vSpawnsCharacter.push_back(pos);
			return true;
		}
	}

	return CController::onMapTile(tileId, pos, tileDir, modifierId);
}

void CControllerMain::onCharacterDeath(CCharacter *pVictim, CPlayer *pKiller) noexcept
{
	CController::onCharacterDeath(pVictim, pKiller);
}
