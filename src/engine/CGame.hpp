/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME
#define H_GAME

#include <game/CGameClient.hpp>
#include <cstdlib>


class CGame final
{
	static CGame *ms_pCoreInstance;
public:
	static CGame* getInstance()
	{
		if (!ms_pCoreInstance)
		{
			ms_pCoreInstance = new CGame();
			atexit(&destroyInstance);
		}

		return ms_pCoreInstance;
	}
	~CGame();

	class CGameClient* Client() const noexcept { return m_pGameClient; }

	void init() noexcept;

protected:
	CGame() noexcept;
	CGame(const CGame&) noexcept;
	CGame& operator= (const CGame&) noexcept;

	class CGameClient *m_pGameClient;

private:
	static void destroyInstance() noexcept
	{
		delete ms_pCoreInstance;
		ms_pCoreInstance = nullptr;
	}
};

#endif
