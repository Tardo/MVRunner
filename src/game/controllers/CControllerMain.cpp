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
	m_LastCheckPoint = VECTOR_ZERO;
	m_TimerFreezed = 0;
}
CControllerMain::CControllerMain(class CContext *pContext) noexcept
: CController(pContext)
{
	m_LastCheckPoint = VECTOR_ZERO;
	m_TimerFreezed = 0;
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
				int charState = pChar->getCharacterState();

				// If the camera is in travel not execute player commands
				if (!(Game()->Client()->Camera().getStatus()&CCamera::TRAVEL))
				{
					const sf::Vector2f dir = upm::vectorNormalize(Game()->Client()->mapPixelToCoords(Game()->Client()->m_MousePosition, Game()->Client()->Camera()) - charPos);
					if (pChar->getBody()->IsFixedRotation())
						pChar->getBody()->SetTransform(pChar->getBody()->GetPosition(), upm::degToRad(upm::vectorAngle(dir)+90.0f));

					// Player Character Movement
					if (!(charState&CCharacter::STATE_FREEZED))
					{
						int moveState = CCharacter::MOVE_STATE_STOP;
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
							moveState = CCharacter::MOVE_STATE_UP;
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
							moveState |= CCharacter::MOVE_STATE_LEFT;
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
							moveState |= CCharacter::MOVE_STATE_RIGHT;

						pChar->move(moveState);

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
					}

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

					// Player Light
					if (m_pPlayerLight)
						m_pPlayerLight->m_Position = charPos;
				}

				// Unfreeze
				if ((charState&CCharacter::STATE_FREEZED) && ups::timeGet()-m_TimerFreezed > ups::timeFreq()*4.0f)
				{
					charState &= ~CCharacter::STATE_FREEZED;
					pChar->setCharacterState(charState);
				}

				const int tileId = Game()->Client()->MapRender().getMapTileIndex(Game()->Client()->MapRender().getMapPos(charPos), Game()->Client()->MapRender().getGameLayer());
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
				else if (tileId == TILE_STATE_ROTATE)
				{
					charState |= CCharacter::STATE_ROTATE;
					pChar->setCharacterState(charState);
					pChar->getBody()->SetFixedRotation(false);
				}
				else if (tileId == TILE_STATE_FREEZE)
				{
					charState |= CCharacter::STATE_FREEZED;
					pChar->setCharacterState(charState);
					m_TimerFreezed = ups::timeGet();
				}
				else if (tileId == TILE_STATE_CLEAN)
				{
					pChar->setCharacterState(CCharacter::STATE_NORMAL);
					pChar->getBody()->SetFixedRotation(true);
				}
				else if (tileId == TILE_SPEED_SOFT)
				{
					const sf::Vector2f tileDir = Game()->Client()->MapRender().getTileDirectionVector(Game()->Client()->MapRender().getMapPos(charPos));
					pChar->getBody()->ApplyLinearImpulseToCenter(CSystemBox2D::sfToB2(tileDir*g_Config.m_SpeedSoftImpulse), true);
				}
				else if (tileId >= TILE_1 && tileId <= TILE_25)
				{
					m_LastCheckPoint = charPos;
				}
				else if (tileId == TILE_DEAD)
				{
					pChar->getBody()->SetFixedRotation(true);
					pChar->getBody()->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					pChar->getBody()->SetAngularVelocity(0.0f);
					Game()->Client()->getSystem<CSystemFx>()->createFireBall(pChar);
					pChar->getBody()->SetTransform(CSystemBox2D::sfToB2(m_LastCheckPoint), pChar->getBody()->GetAngle());
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
					std::pair<std::map<int, CTeleport>::iterator, bool> rec = m_vTeleports.insert(std::make_pair(teleId, CTeleport()));
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
