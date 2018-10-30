/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include "CAssetManager.hpp"
#include <game/CGameClient.hpp>
#include <base/system.hpp>
#include <SFML/System/Lock.hpp>
#include <cstring>

sf::Mutex CAssetManager::s_Mutex;
CAssetManager::CAssetManager(Zpg *pStorage) noexcept
{
	m_pStorage = pStorage;

	m_Loaded = 0;
	m_LoadErrors = false;
	m_aCurrentLoadAssetPath[0] = 0;
	m_pDataDefFont = nullptr;
}
CAssetManager::~CAssetManager() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CAssetManager", "Deleted");
	#endif
}

bool CAssetManager::isLoaded() const noexcept
{
	if (!g_Config.m_UseShaders)
		return m_Loaded == NUM_TOTAL-MAX_SHADERS;
	return m_Loaded == NUM_TOTAL;
}

void CAssetManager::load() noexcept
{
	if (isLoaded())
		return;

	sf::Lock lock(s_Mutex);

	const unsigned char *pFileData = nullptr, *pFileDataB = nullptr;
	unsigned long fileSize = 0, fileSizeB = 0;

	m_pDataDefFont = m_pStorage->getFileData("data/font.ttf", &fileSize);
	m_DefFont.loadFromMemory(m_pDataDefFont, fileSize);

	LOAD_TEXTURE(TEXTURE_SMOKE_WHITE, "data/fx/smoke_w.png")
	LOAD_TEXTURE(TEXTURE_SMOKE_BLACK, "data/fx/smoke_b.png")
	LOAD_TEXTURE(TEXTURE_BULLET_SPARK, "data/fx/bullet_spark.png")
	LOAD_TEXTURE(TEXTURE_EXPLOSION, "data/fx/explosion.png")
	LOAD_TEXTURE(TEXTURE_EXPLOSION_RING, "data/fx/explosion_ring.png")
	LOAD_TEXTURE(TEXTURE_EXPLOSION_ANIM, "data/fx/explosion_anim.png")
	LOAD_TEXTURE(TEXTURE_POINT_LIGHT, "data/fx/pointlight.jpg")
	LOAD_TEXTURE(TEXTURE_SPOT_LIGHT, "data/fx/spotlight.jpg")
	LOAD_TEXTURE(TEXTURE_EMISSIVE_LIGHT, "data/fx/emissivelight.jpg")
	LOAD_TEXTURE(TEXTURE_PLAYER, "data/characters/player.png")
	LOAD_TEXTURE(TEXTURE_BOX, "data/fx/box.png")
	LOAD_TEXTURE(TEXTURE_SFML_LOGO, "data/fx/sfml_logo.png")
	LOAD_TEXTURE(TEXTURE_WATER_SHADER, "data/fx/water_shader.png")
	LOAD_TEXTURE(TEXTURE_GRENADE, "data/objects/grenade.png")
	LOAD_TEXTURE(TEXTURE_CANON_BALL, "data/objects/canon_ball.png")
	LOAD_TEXTURE(TEXTURE_METAL_BOX, "data/fx/metal_box.png")
	LOAD_TEXTURE(TEXTURE_SKIN_DEFAULT, "data/skins/default.png")
	LOAD_TEXTURE(TEXTURE_HOOK_DEFAULT_HEAD, "data/hooks/default_head.png")
	m_vTextures[TEXTURE_HOOK_DEFAULT_HEAD].setRepeated(true);
	m_vTextures[TEXTURE_HOOK_DEFAULT_HEAD].setSmooth(false);
	LOAD_TEXTURE(TEXTURE_HOOK_DEFAULT_BODY, "data/hooks/default_body.png")
	m_vTextures[TEXTURE_HOOK_DEFAULT_BODY].setRepeated(true);
	m_vTextures[TEXTURE_HOOK_DEFAULT_BODY].setSmooth(false);

	LOAD_SOUND(SOUND_MOUSE_CLICK, "data/sfx/mouse_click.wav")
	LOAD_SOUND(SOUND_GAMEOVER, "data/sfx/gameover.wav")
	LOAD_SOUND(SOUND_METAL_IMPACT, "data/sfx/metal_impact.wav")
	LOAD_SOUND(SOUND_OBJECT_FLY, "data/sfx/object_fly.wav")
	LOAD_SOUND(SOUND_KILL, "data/sfx/kill.wav")
	LOAD_SOUND(SOUND_HEARTBEAT, "data/sfx/heartbeat.wav")
	LOAD_SOUND(SOUND_DAMAGE, "data/sfx/damage.wav")
	LOAD_SOUND(SOUND_ELECTRICITY, "data/sfx/electricity.wav")
	LOAD_SOUND(SOUND_STONE_IMPACT, "data/sfx/stone_impact.wav")
	LOAD_SOUND(SOUND_BOX_DESTROYED, "data/sfx/stone_impact.wav")
	LOAD_SOUND(SOUND_WHEEL_CLICK, "data/sfx/wheel_click.wav")
	LOAD_SOUND(SOUND_BEEP, "data/sfx/beep.wav")

	if (g_Config.m_UseShaders)
	{
		LOAD_SHADERS(SHADER_FLAG, "data/shaders/wave.vert", "data/shaders/flag.frag")
		LOAD_SHADER(SHADER_PIXELATE, "data/shaders/pixelate.frag", sf::Shader::Fragment)
		LOAD_SHADER(SHADER_CHROMATIC_ABERRATION, "data/shaders/chromatic_aberration.frag", sf::Shader::Fragment)
		LOAD_SHADER(SHADER_BLUR, "data/shaders/blur.frag", sf::Shader::Fragment)
		LOAD_SHADER(SHADER_BLOOM, "data/shaders/bloom.frag", sf::Shader::Fragment)
		LOAD_SHADER(SHADER_METABALL, "data/shaders/metaball.frag", sf::Shader::Fragment)
		LOAD_SHADER(SHADER_NORMALMAP, "data/shaders/bump.frag", sf::Shader::Fragment)
	}

	s_Mutex.unlock();
}

sf::Texture* CAssetManager::getTexture(int textId) noexcept
{
	if (textId < 0 || textId >= MAX_TEXTURES)
		return nullptr;

	return &m_vTextures[textId];
}

const sf::SoundBuffer& CAssetManager::getSound(int soundId) noexcept
{
	return m_vSounds[soundId];
}

sf::Shader* CAssetManager::getShader(int shaderId) noexcept
{
	if (shaderId < 0 || shaderId >= MAX_SHADERS || !g_Config.m_UseShaders)
		return 0x0;

	return &m_vShaders[shaderId];
}
