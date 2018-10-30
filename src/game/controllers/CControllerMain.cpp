/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/math.hpp>
#include <base/system.hpp>
#include <engine/CAssetManager.hpp>
#include <engine/CSystemBox2D.hpp>
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

	m_vPlayerCanonSpawns.clear();
	m_vTeleports.clear();

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CControllerMain", "Deleted");
	#endif
}

void CControllerMain::onSystemEvent(sf::Event *pEvent) noexcept
{ }

bool CControllerMain::onInit() noexcept
{
	loadMap("data/map.tmx");

	return CController::onInit();
}

void CControllerMain::onReset() noexcept
{
	Game()->Client()->getSystem<CSystemSound>()->stopBackgroundMusic();

	m_vPlayerCanonSpawns.clear();
	m_vTeleports.clear();

	return CController::onReset();
}

void CControllerMain::tick() noexcept
{
	CController::tick();

	CPlayer *pMainPlayer = Context()->getPlayer();

	if (Game()->Client()->hasFocus())
	{
		static bool pressedButtonUse = false;
		//static bool pressedButtonLight = false;

		if (pMainPlayer->getCharacter())
		{
			CCharacter *pChar = pMainPlayer->getCharacter();
			if (pChar && pChar->isAlive())
			{
				const sf::Vector2f &charPos = CSystemBox2D::b2ToSf(pChar->getBody()->GetPosition());
				int charState = pChar->getCharacterState();

				/** PLAYER ACTIONS **/
				// If the camera is in travel not execute player commands
				if (!(Game()->Client()->Camera().getStatus()&CCamera::TRAVEL) && Game()->Client()->Menus().getActive() == CMenus::NONE)
				{
					const sf::Vector2f dir = upm::vectorNormalize(Game()->Client()->mapPixelToCoords(Game()->Client()->UI().getMousePos(), Game()->Client()->Camera()) - charPos);

					// Player Character Movement
					if (!(charState&CCharacter::STATE_FREEZED))
					{
						int moveState = CCharacter::MOVE_STATE_STOP;
						if (Game()->Client()->Controls().isKeyPressed("jump"))
							moveState = CCharacter::MOVE_STATE_UP;
						if (Game()->Client()->Controls().isKeyPressed("left"))
							moveState |= CCharacter::MOVE_STATE_LEFT;
						else if (Game()->Client()->Controls().isKeyPressed("right"))
							moveState |= CCharacter::MOVE_STATE_RIGHT;

						pChar->setMoveState(moveState);

						static bool hookUsed = false;
						// Player Shoot & Hook
						if (Game()->Client()->Controls().isMousePressed("fire") && !(charState&CCharacter::STATE_ROTATE))
							pChar->doFire();
						else if (!hookUsed && Game()->Client()->Controls().isMousePressed("hook") && !(charState&CCharacter::STATE_ROTATE))
						{
							pChar->doHook(true);
							hookUsed = true;
						}
						else if (!Game()->Client()->Controls().isMousePressed("hook"))
						{
							hookUsed = false;
							if (pChar->m_HookState != CCharacter::HOOK_STATE_RETRACTED || pChar->m_HookState != CCharacter::HOOK_STATE_RETRACTING)
								pChar->doHook(false);
						}
					}

					// Player Select Weapon
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0))
						pChar->setActiveWeapon(-1);
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
					{
						pChar->giveWeapon(WEAPON_GRENADE_LAUNCHER, -1, -1);
						pChar->setActiveWeapon(WEAPON_GRENADE_LAUNCHER);
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
					{
						pChar->giveWeapon(WEAPON_JET_PACK, -1, -1);
						pChar->setActiveWeapon(WEAPON_JET_PACK);
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
					{
						pChar->giveWeapon(WEAPON_VISCOSITY_LAUNCHER, -1, -1);
						pChar->setActiveWeapon(WEAPON_VISCOSITY_LAUNCHER);
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
				}

				/** PLAYER STATE **/
				// Unfreeze
				if ((charState&CCharacter::STATE_FREEZED) && ups::timeGet()-m_TimerFreezed > ups::timeFreq()*g_Config.m_TimeFreeze)
				{
					charState &= ~CCharacter::STATE_FREEZED;
					pChar->setCharacterState(charState);
				}

				const int tileId = Context()->Map().getMapTileIndex(Context()->Map().getMapPos(charPos), Context()->Map().getGameLayer());
				/** PLAYER MAP ENVIRONMENT **/
				// Interact with the map environment
				if (tileId == TILE_TELEPORT_IN)
				{
					const int modifierId = Context()->Map().getMapTileIndex(Context()->Map().getMapPos(charPos), Context()->Map().getGameModifiersLayer());
					int teleId = -1;
					if (modifierId >= TILE_1 && modifierId <= TILE_25)
						teleId = (modifierId-TILE_1)+1;
					std::map<int, CTeleport>::iterator it = m_vTeleports.find(teleId);
					if (it != m_vTeleports.cend())
					{
						//Game()->Client()->Camera().moveTo((*it).second.m_PosOut, 2.0f);
						pChar->teleport((*it).second.m_PosOut);
					}
				}
				else if (tileId == TILE_STATE_ROTATE)
				{
					charState |= CCharacter::STATE_ROTATE;
					pChar->setCharacterState(charState);
					//pChar->getBody()->SetFixedRotation(false);
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
					//pChar->getBody()->SetFixedRotation(true);
				}
				else if (tileId == TILE_SPEED_SOFT)
				{
					const sf::Vector2f tileDir = Context()->Map().getTileDirectionVector(Context()->Map().getMapPos(charPos));
					pChar->getBody()->ApplyLinearImpulse(CSystemBox2D::sfToB2(tileDir*g_Config.m_SpeedSoftImpulse), pChar->getBody()->GetWorldCenter(), true);
				}
				else if (tileId == TILE_SPEED_HARD)
				{
					const sf::Vector2f tileDir = Context()->Map().getTileDirectionVector(Context()->Map().getMapPos(charPos));
					pChar->getBody()->ApplyLinearImpulse(CSystemBox2D::sfToB2(tileDir*g_Config.m_SpeedHardImpulse), pChar->getBody()->GetWorldCenter(), true);
				}
				else if (tileId >= TILE_1 && tileId <= TILE_25)
				{
					m_LastCheckPoint = charPos;
				}
				else if (tileId == TILE_DEAD)
				{
					//pChar->getBody()->SetFixedRotation(true);
					pChar->kill();
				}
				else if (tileId == TILE_START_TIME || tileId == TILE_END_TIME)
				{
					Game()->Client()->runPlayerTime(tileId == TILE_START_TIME);
				}
			}
		}

		/** SPAWN CANONS **/
		std::vector<CSpawnCanon>::iterator it = m_vPlayerCanonSpawns.begin();
		while (it != m_vPlayerCanonSpawns.end())
		{
			if (ups::timeGet()-(*it).m_Timer > ups::timeFreq()*(*it).m_Duration)
			{
				new CProjectile((*it).m_Pos+(*it).m_Dir*80.0f, sf::Vector2f(64.0f, 40.0f), 0.0f, (*it).m_Dir, 0x0, WEAPON_CANON_BALL, WEAPON_CANON_BALL);
				(*it).m_Timer = ups::timeGet();
			}
			++it;
		}
	}
}

void CControllerMain::onStart() noexcept
{
	CController::onStart();
	// Spawn Player
	m_LastCheckPoint = m_PlayerSpawnPos.m_Pos;
	Context()->getPlayer()->createCharacter(m_PlayerSpawnPos.m_Pos);
	Game()->Client()->Camera().setZoom(g_Config.m_ZoomCharacter);

	Game()->Client()->Camera().setTarget(Context()->getPlayer()->getCharacter());
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
			}
		}
	}
	else
	{
		const unsigned int entityId = tileId - ENTITY_OFFSET;
		if (entityId == ENTITY_SPAWN_PLAYER)
		{
			m_PlayerSpawnPos.m_Pos = pos;
		}
		else if (entityId == ENTITY_CANON)
		{
			m_vPlayerCanonSpawns.push_back(CSpawnCanon(pos, Context()->Map().getTileDirectionVector(tileDir), (modifierId-TILE_MOD_TIMER_1_SEC)+1));
		}
	}

	return CController::onMapTile(tileId, pos, tileDir, modifierId);
}

void CControllerMain::onCharacterDeath(CCharacter *pVictim, CPlayer *pKiller) noexcept
{
	CController::onCharacterDeath(pVictim, pKiller);

	if (pVictim->getType() == CEntity::CHARACTER)
	{
		CSystemBox2D *pSystemBox2D = Game()->Client()->getSystem<CSystemBox2D>();
		pSystemBox2D->createBlood(CSystemBox2D::b2ToSf(pVictim->getBody()->GetPosition()), CCharacter::SIZE);

		pVictim->getBody()->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
		pVictim->getBody()->SetAngularVelocity(0.0f);
		pVictim->getBody()->SetTransform(CSystemBox2D::sfToB2(m_LastCheckPoint), pVictim->getBody()->GetAngle());
		pVictim->setVisible(true);
		pVictim->setAlive(true);
	}
}
