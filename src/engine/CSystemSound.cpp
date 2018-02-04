/* (c) Alexandre D�az. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at fingership.redneboa.es        */

#include "CAssetManager.hpp"
#include "CGame.hpp"
#include "CConfig.hpp"
#include <base/system.hpp>
#include <engine/CSystemSound.hpp>

CSystemSound::CSystemSound() noexcept
{
	m_LastMusicId = -1;
}
CSystemSound::~CSystemSound() noexcept
{
	//Stop Sounds
	stopAll();
	stopBackgroundMusic();
	cleanSoundBuffer();
	m_vpPlaySounds.clear();

	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CSystemSound", "Deleted");
	#endif
}

bool CSystemSound::init(class CGameClient *pGameClient) noexcept
{
	return CSystem::init(pGameClient);
}

void CSystemSound::update(float deltaTime) noexcept
{
	cleanSoundBuffer();

	CGame *pGame = CGame::getInstance();
	if (pGame->Client()->Controller() && pGame->Client()->Controller()->Context()->getPlayer()->getCharacter())
	{
		const sf::Vector2f &pos = CSystemBox2D::b2ToSf(pGame->Client()->Controller()->Context()->getPlayer()->getCharacter()->getBody()->GetPosition());
		sf::Listener::setPosition(pos.x, 0.0f, pos.y);
	}
}

void CSystemSound::setSfxActive(bool status) noexcept
{
	g_Config.m_Sfx = status;

	if (!g_Config.m_Sfx)
		stopAll();
}

void CSystemSound::setMusicActive(bool status) noexcept
{
	g_Config.m_Music = status;

	if (!g_Config.m_Music)
		stopBackgroundMusic();
}

void CSystemSound::cleanSoundBuffer() noexcept
{
	// Sound Manage
	std::list<sf::Sound*>::iterator itps = m_vpPlaySounds.begin();
	while (itps != m_vpPlaySounds.end())
	{
		if ((*itps)->getStatus() == sf::Sound::Stopped)
		{
			delete (*itps);
			(*itps) = nullptr;
			itps = m_vpPlaySounds.erase(itps);
		} else
			++itps;
	}
}

sf::Sound* CSystemSound::createSound(int soundId, const sf::Vector2f worldPos, float volume, bool loop, float minDist, bool play) noexcept
{
	CGame *pGame = CGame::getInstance();
	sf::Sound *pSound = new sf::Sound(pGame->Client()->Assets().getSound(soundId));
	if (pSound)
	{
		pSound->setVolume(volume);
		pSound->setPosition(worldPos.x, 0.0f, worldPos.y);
		pSound->setMinDistance(minDist);
		pSound->setAttenuation(minDist/8.0f);
		pSound->setLoop(loop);
		if (play)
			pSound->play();
		m_vpPlaySounds.push_back(pSound);
	}
	return pSound;
}

sf::Sound* CSystemSound::play(int soundId, float volume) noexcept
{
	if (!g_Config.m_Sfx || soundId < 0 || soundId >= CAssetManager::MAX_SOUNDS)
		return 0x0;

	sf::Sound *pSound = createSound(soundId, VECTOR_ZERO, volume);
	pSound->setRelativeToListener(true);
	pSound->play();
	return pSound;
}

sf::Sound* CSystemSound::play(int soundId, const sf::Vector2f &worldPos, float minDist, float volume) noexcept
{
	if (!g_Config.m_Sfx || soundId < 0 || soundId >= CAssetManager::MAX_SOUNDS)
		return 0x0;

	sf::Sound *pSound = createSound(soundId, worldPos, volume, false, minDist, true);
	return pSound;
}

void CSystemSound::pauseAll() noexcept
{
	if (m_BackgroundMusic.getStatus() != sf::Sound::Stopped)
		m_BackgroundMusic.pause();

	std::list<sf::Sound*>::iterator itps = m_vpPlaySounds.begin();
	while (itps != m_vpPlaySounds.end())
	{
		if ((*itps)->getStatus() != sf::Sound::Stopped)
			(*itps)->pause();

		++itps;
	}
}
void CSystemSound::resumeAll() noexcept
{
	if (m_BackgroundMusic.getStatus() == sf::Sound::Paused)
		m_BackgroundMusic.play();

	std::list<sf::Sound*>::iterator itps = m_vpPlaySounds.begin();
	while (itps != m_vpPlaySounds.end())
	{
		if ((*itps)->getStatus() == sf::Sound::Paused)
			(*itps)->play();

		++itps;
	}
}
void CSystemSound::stopAll() noexcept
{
	std::list<sf::Sound*>::iterator itps = m_vpPlaySounds.begin();
	while (itps != m_vpPlaySounds.end())
	{
		if ((*itps)->getStatus() != sf::Sound::Stopped)
			(*itps)->stop();

		++itps;
	}
}

void CSystemSound::playBackgroundMusic(int id) noexcept
{
	static const char *musicFiles[] = { "data/sfx/first_floor_theme.ogg", "data/sfx/main_menu_theme.ogg" };
	if (!g_Config.m_Music || id < 0 || id >= CAssetManager::MAX_SONGS)
		return;

	if (id != m_LastMusicId)
	{
		if (m_BackgroundMusic.getStatus() != sf::Sound::Stopped)
			m_BackgroundMusic.stop();

		unsigned long fileSize = 0;
		const unsigned char *pData = Client()->Storage().getFileData(musicFiles[id], &fileSize);
		m_BackgroundMusic.openFromMemory(pData, fileSize);
		//delete[] pData;
		m_BackgroundMusic.setLoop(true);
		m_BackgroundMusic.setVolume(80);
		m_BackgroundMusic.play();

		m_LastMusicId = id;
	}
	else if (m_BackgroundMusic.getStatus() == sf::Sound::Stopped)
		m_BackgroundMusic.play();
}

void CSystemSound::stopBackgroundMusic() noexcept
{
	if (m_BackgroundMusic.getStatus() != sf::Sound::Stopped)
		m_BackgroundMusic.stop();
}
