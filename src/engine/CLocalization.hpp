/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_LOCALIZATION
#define H_ENGINE_LOCALIZATION
#include <Zpg/Zpg.hpp>
#include <string>
#include <map>

#define	_(str)	g_l10n.getText(str)

class CLocalization final
{
public:
	CLocalization() noexcept;
	~CLocalization() noexcept;

	bool load(const char *pLang, Zpg *pStorage) noexcept;
	const char *getText(const std::string &str) const noexcept;

private:
	//TODO: Not use stl
	std::map<std::string, std::string> m_Translations;
	Zpg *m_pStorage;
};

extern CLocalization g_l10n;

#endif
