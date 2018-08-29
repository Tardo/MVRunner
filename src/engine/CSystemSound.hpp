/* (c) Alexandre D�az. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at fingership.redneboa.es        */

#ifndef H_ENGINE_SYSTEM_SOUND
#define H_ENGINE_SYSTEM_SOUND
#include "CAssetManager.hpp"
#include <SFML/Audio.hpp>
#include <list>
#include "ISystem.hpp"

#define SOUND_MIN_DISTANCE	1.0f

class CSystemSound final : public ISystem
{
public:
	CSystemSound() noexcept;
	virtual ~CSystemSound() noexcept final;

	virtual bool init() noexcept final;
	virtual void reset() noexcept final;
	virtual void update(float deltaTime) noexcept final;

	void playBackgroundMusic(int id) noexcept;
	void stopBackgroundMusic() noexcept;
	bool isBackgroundMusicPlaying() const noexcept { return m_BackgroundMusic.getStatus() == sf::Sound::Playing; }

	sf::Sound* play(int soundId, float volume = 100) noexcept;
	sf::Sound* play(int soundId, const sf::Vector2f &worldPos, float minDist, float volume = 100) noexcept;
	void pauseAll() noexcept;
	void resumeAll() noexcept;
	void stopAll() noexcept;

	void cleanSoundBuffer() noexcept;
	void setSfxActive(bool status) noexcept;
	void setMusicActive(bool status) noexcept;

	void setAssetManager(CAssetManager *pAssetManager) noexcept { m_pAssetManager = pAssetManager; }
	void setListenerPosition(const sf::Vector2f &listenerPos) noexcept;

	sf::Sound* createSound(int soundId, const sf::Vector2f worldPos, float volume = 100.0f, bool loop = false, float minDist = SOUND_MIN_DISTANCE, bool play = false) noexcept;

	int getNumPlayingSound() const noexcept { return m_vpPlaySounds.size(); }

protected:
	sf::Music m_BackgroundMusic;
	std::list<sf::Sound*> m_vpPlaySounds;
	CAssetManager *m_pAssetManager;

private:
	int m_LastMusicId;
};

#endif
