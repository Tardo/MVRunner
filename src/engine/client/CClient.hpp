/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME
#define H_GAME

#include <game/CGameClient.hpp>
#include <cstdlib>


class CClient final
{
	static CClient *ms_pCoreInstance;
public:
	static CClient* getInstance()
	{
		if (!ms_pCoreInstance)
		{
			ms_pCoreInstance = new CClient();
			atexit(&destroyInstance);
		}

		return ms_pCoreInstance;
	}
	~CClient();

	class CGameClient* Client() const noexcept { return m_pGameClient; }

	void init() noexcept;

protected:
	CClient() noexcept;
	CClient(const CClient&) noexcept;
	CClient& operator= (const CClient&) noexcept;

	class CGameClient *m_pGameClient;

private:
	static void destroyInstance() noexcept
	{
		delete ms_pCoreInstance;
		ms_pCoreInstance = nullptr;
	}
};

#endif
