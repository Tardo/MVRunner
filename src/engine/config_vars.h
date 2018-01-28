/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef ENGINE_CONFIG_VARS_H
#define ENGINE_CONFIG_VARS_H
#undef ENGINE_CONFIG_VARS_H  // this file will be included several times

MACRO_CONFIG_STR(GameVer, game_version, 16, "0.0.1a", 0)

MACRO_CONFIG_STR(Lang, lang, 18, "spanish", CConfig::SAVE)

MACRO_CONFIG_BOOL(Sfx, sfx, true, CConfig::SAVE)
MACRO_CONFIG_BOOL(Music, music, false, CConfig::SAVE)
MACRO_CONFIG_BOOL(VSync, vsync, true, CConfig::SAVE)
MACRO_CONFIG_BOOL(FullScreen, fullscreen, false, CConfig::SAVE)
MACRO_CONFIG_INT(ScreenWidth, screen_width, 1024, 1, 1920, CConfig::SAVE)
MACRO_CONFIG_INT(ScreenHeight, screen_height, 768, 1, 1080, CConfig::SAVE)

MACRO_CONFIG_FLOAT(CameraVibrationSwordKillDuration, camera_vibration_sword_kill_duration, 0.25f, 0.1f, 999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CameraVibrationSwordKillIntensity, camera_vibration_sword_kill_intensity, 2.3f, 0.1f, 999.9f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(ZoomCharacter, zoom_character, 1.7f, 0.1f, 4.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(ZoomDead, zoom_dead, 0.3f, 0.1f, 9.9f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(MapArrowTimeSpawn, map_arrow_time_spawn, 3.0f, 0.1f, 999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(MapArrowSpeed, map_arrow_speed, 180.0f, 0.1f, 999.9f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(CharacterHitDistance, character_hit_distance, 20.0f, 0.1f, 999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterMaxVelocity, character_max_velocity, 280.0f, 0.1f, 9999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterImpulse, character_impulse, 2.0f, 0.1f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterJumpImpulse, character_jump_impulse, 10.0f, 0.1f, 999.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherEnergy, weapon_grenade_launcher_energy, 8.5f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherRadius, weapon_grenade_launcher_radius, 192.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherFireDelay, weapon_grenade_fire_delay, 0.45f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherLifeTime, weapon_grenade_life_time, 2.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherSpeed, weapon_grenade_speed, 5.0f, 0.0f, 999.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(BotsSpawnMargin, bots_spawn_margin, 1000.0f, 200.0f, 9999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(HitBoxGoreDuration, hitbox_gore_duration, 4.0f, 0.0f, 999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(ShowTileAnimDuration, show_tile_anim_duration, 24.0f, 0.1f, 999.9f, CConfig::SAVE)


#endif
