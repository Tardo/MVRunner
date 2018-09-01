/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef ENGINE_CONFIG_VARS_H
#define ENGINE_CONFIG_VARS_H
#undef ENGINE_CONFIG_VARS_H  // this file will be included several times

MACRO_CONFIG_STR(Lang, lang, 18, "spanish", CConfig::SAVE)

MACRO_CONFIG_BOOL(Sfx, sfx, true, CConfig::SAVE)
MACRO_CONFIG_BOOL(Music, music, false, CConfig::SAVE)
MACRO_CONFIG_BOOL(VSync, vsync, true, CConfig::SAVE)
MACRO_CONFIG_BOOL(FullScreen, fullscreen, false, CConfig::SAVE)
MACRO_CONFIG_BOOL(UseShaders, use_shaders, true, CConfig::SAVE)
MACRO_CONFIG_INT(ScreenWidth, screen_width, 1024, 1, 1920, CConfig::SAVE)
MACRO_CONFIG_INT(ScreenHeight, screen_height, 768, 1, 1080, CConfig::SAVE)
MACRO_CONFIG_BOOL(CursorGrab, cursor_grab, true, CConfig::SAVE)
MACRO_CONFIG_BOOL(CursorShow, cursor_show, false, CConfig::SAVE)

MACRO_CONFIG_FLOAT(SpeedSoftImpulse, speed_soft_impulse, 12.0f, 0.1f, 99.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(SpeedHardImpulse, speed_hard_impulse, 20.0f, 0.1f, 99.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(TimeFreeze, time_freeze, 4.0f, 0.1f, 99.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(ZoomCharacter, zoom_character, 1.7f, 0.1f, 4.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(CharacterHitDistance, character_hit_distance, 20.0f, 0.1f, 999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterMaxVelocity, character_max_velocity, 480.0f, 0.1f, 9999.9f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterImpulse, character_impulse, 21.4f, 0.1f, 99.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterJumpImpulse, character_jump_impulse, 700.0f, 0.1f, 999.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(CharacterHookFlyVel, character_hook_fly_vel, 55.0f, 0.1f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterHookRetractVel, character_hook_retract_vel, 1.0f, 0.1f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterHookForce, character_hook_force, 10.0f, 0.1f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterHookFrequency, character_hook_frequency, 0.8f, 0.1f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterHookDampingRatio, character_hook_damping_ratio, 6.5f, 0.1f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(CharacterHookMaxLength, character_hook_max_length, 15.0f, 0.1f, 999.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(AimLineLength, aim_line_length, 120.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_COLOR(AimLineColor, aim_line_color, 3357088255, CConfig::SAVE)

MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherEnergy, weapon_grenade_launcher_energy, 12.0f, 0.0f, 9999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherRadius, weapon_grenade_launcher_radius, 192.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherFireDelay, weapon_grenade_launcher_fire_delay, 0.45f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherLifeTime, weapon_grenade_launcher_life_time, 8.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponGrenadeLauncherSpeed, weapon_grenade_launcher_speed, 44.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponJetPackEnergy, weapon_jet_pack_energy, 10.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponJetPackFireDelay, weapon_jet_pack_fire_delay, 0.03f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponJetPackLifeTime, weapon_jet_pack_life_time, 0.1f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponJetPackSpeed, weapon_jet_pack_speed, 20.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponCanonBallEnergy, weapon_canon_energy, 12.5f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponCanonBallLifeTime, weapon_canon_life_time, 2.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponCanonBallSpeed, weapon_canon_ball_speed, 10.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponViscosityLauncherEnergy, weapon_viscosity_launcher_energy, 12.0f, 0.0f, 9999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponViscosityLauncherRadius, weapon_viscosity_launcher_radius, 192.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponViscosityLauncherFireDelay, weapon_viscosity_launcher_fire_delay, 0.45f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponViscosityLauncherLifeTime, weapon_viscosity_launcher_life_time, 8.0f, 0.0f, 999.0f, CConfig::SAVE)
MACRO_CONFIG_FLOAT(WeaponViscosityLauncherSpeed, weapon_viscosity_launcher_speed, 44.0f, 0.0f, 999.0f, CConfig::SAVE)

MACRO_CONFIG_FLOAT(HitBoxGoreDuration, hitbox_gore_duration, 4.0f, 0.0f, 999.9f, CConfig::SAVE)

MACRO_CONFIG_COLOR(ButtonNormalFillColor, button_normal_fill_color, 2112594, CConfig::SAVE)
MACRO_CONFIG_COLOR(ButtonNormalOutlineColor, button_normal_outline_color, 0, CConfig::SAVE)
MACRO_CONFIG_COLOR(ButtonNormalTextColor, button_normal_text_color, 3300095, CConfig::SAVE)
MACRO_CONFIG_COLOR(ButtonFocusFillColor, button_focus_fill_color, 2117688, CConfig::SAVE)
MACRO_CONFIG_COLOR(ButtonFocusOutlineColor, button_focus_outline_color, 0, CConfig::SAVE)
MACRO_CONFIG_COLOR(ButtonFocusTextColor, button_focus_text_color, 845428735, CConfig::SAVE)

#endif
