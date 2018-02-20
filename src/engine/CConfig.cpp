/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/* Original Code from Teeworlds by Magnus Auvinen */

#include "CConfig.hpp"
#include <base/system.hpp>
#include <base/math.hpp>
#include <fstream>
#include <cstring>

CConfiguration g_Config;

CConfig::CConfig(const char *pPathFile) noexcept
{
	strncpy(m_PathFile, pPathFile, sizeof(m_PathFile));
	reset();
	load();

	loadGameInfo();
}


bool CConfig::load() noexcept
{
	std::ifstream configFile;
	configFile.open(m_PathFile);

	if(!configFile.is_open())
		return false;

	std::string paramName, paramValue;
	while (configFile >> paramName >> paramValue)
	{
		#define MACRO_CONFIG_INT(Name,ScriptName,def,min,max,flags) if (paramName.compare(#ScriptName) == 0) g_Config.m_##Name = upm::clamp(::atoi(paramValue.c_str()), min, max);
		#define MACRO_CONFIG_FLOAT(Name,ScriptName,def,min,max,flags) if (paramName.compare(#ScriptName) == 0) g_Config.m_##Name = upm::clamp((float)::atof(paramValue.c_str()), min, max);
		#define MACRO_CONFIG_STR(Name,ScriptName,len,def,flags) if (paramName.compare(#ScriptName) == 0) strncpy(g_Config.m_##Name, paramValue.c_str(), len);
		#define MACRO_CONFIG_BOOL(Name,ScriptName,def,flags) if (paramName.compare(#ScriptName) == 0) g_Config.m_##Name = (paramValue.compare("true") == 0);
		#define MACRO_CONFIG_COLOR(Name,ScriptName,def,flags) if (paramName.compare(#ScriptName) == 0) g_Config.m_##Name = ups::intToColor(::atoi(paramValue.c_str()));

		#include <game/config_vars.h>

		#undef MACRO_CONFIG_INT
		#undef MACRO_CONFIG_FLOAT
		#undef MACRO_CONFIG_STR
		#undef MACRO_CONFIG_BOOL
	}


	configFile.close();

	return true;
}

void CConfig::loadGameInfo() noexcept
{
	g_Config.m_aWeaponsInfo[WEAPON_GRENADE_LAUNCHER] = {g_Config.m_WeaponGrenadeLauncherFireDelay, g_Config.m_WeaponGrenadeLauncherEnergy, g_Config.m_WeaponGrenadeLauncherLifeTime, g_Config.m_WeaponGrenadeLauncherRadius, g_Config.m_WeaponGrenadeLauncherSpeed};
	g_Config.m_aWeaponsInfo[WEAPON_JET_PACK] = {g_Config.m_WeaponJetPackFireDelay, g_Config.m_WeaponJetPackEnergy, 0.0f, 0.0f, 0.0f};

}

void CConfig::reset() noexcept
{
	#define MACRO_CONFIG_INT(Name,ScriptName,def,min,max,flags) g_Config.m_##Name = def;
	#define MACRO_CONFIG_FLOAT(Name,ScriptName,def,min,max,flags) g_Config.m_##Name = def;
	#define MACRO_CONFIG_STR(Name,ScriptName,len,def,flags) strncpy(g_Config.m_##Name, def, len);
	#define MACRO_CONFIG_BOOL(Name,ScriptName,def,flags) g_Config.m_##Name = def;
	#define MACRO_CONFIG_COLOR(Name,ScriptName,def,flags) g_Config.m_##Name = ups::intToColor(def);

	#include <game/config_vars.h>

	#undef MACRO_CONFIG_INT
	#undef MACRO_CONFIG_FLOAT
	#undef MACRO_CONFIG_STR
	#undef MACRO_CONFIG_BOOL
	#undef MACRO_CONFIG_COLOR
}

bool CConfig::save() noexcept
{
	std::ofstream configFile;
	configFile.open(m_PathFile);

	if(!configFile.is_open())
		return false;

	char aLineBuf[512];  // ignore unused warning

	#define MACRO_CONFIG_INT(Name,ScriptName,def,min,max,flags) if(flags&SAVE){ snprintf(aLineBuf, sizeof(aLineBuf), "%s %i\n", #ScriptName, upm::clamp(g_Config.m_##Name, min, max)); configFile << aLineBuf; }
	#define MACRO_CONFIG_FLOAT(Name,ScriptName,def,min,max,flags) if(flags&SAVE){ snprintf(aLineBuf, sizeof(aLineBuf), "%s %.2f\n", #ScriptName, upm::clamp(g_Config.m_##Name, min, max)); configFile << aLineBuf; }
	#define MACRO_CONFIG_STR(Name,ScriptName,len,def,flags) if(flags&SAVE){ snprintf(aLineBuf, sizeof(aLineBuf), "%s %s\n", #ScriptName, g_Config.m_##Name); configFile << aLineBuf; }
	#define MACRO_CONFIG_BOOL(Name,ScriptName,def,flags) if(flags&SAVE){ snprintf(aLineBuf, sizeof(aLineBuf), "%s %s\n", #ScriptName, g_Config.m_##Name?"true":"false"); configFile << aLineBuf; }
	#define MACRO_CONFIG_COLOR(Name,ScriptName,def,flags) if(flags&SAVE){ snprintf(aLineBuf, sizeof(aLineBuf), "%s %u\n", #ScriptName, ups::colorToInt(g_Config.m_##Name)); configFile << aLineBuf; }

	#include <game/config_vars.h>

	#undef MACRO_CONFIG_INT
	#undef MACRO_CONFIG_FLOAT
	#undef MACRO_CONFIG_STR
	#undef MACRO_CONFIG_BOOL
	#undef MACRO_CONFIG_COLOR

	configFile.close();
	return true;
}
