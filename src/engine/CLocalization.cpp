/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <base/system.hpp>
#include "CLocalization.hpp"
#include <fstream>
#include <cstring>
#include <sstream>

CLocalization g_l10n;
CLocalization::CLocalization() noexcept
{
	m_pStorage = 0x0;
}
CLocalization::~CLocalization() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CLocalization", "Deleted");
	#endif
}

bool CLocalization::load(const char *pLang, Zpg *pStorage) noexcept
{
	ups::msgDebug("CLocalization", "Loading '%s' localization...", pLang);

	m_pStorage = pStorage;

	char aPathFile[128];
	snprintf(aPathFile, sizeof(aPathFile), "data/languages/%s.txt", pLang);
	unsigned long fileSize = 0;
	const unsigned char* pData = m_pStorage->getFileData(aPathFile, &fileSize);
	std::string fileContent = Zpg::toString(pData, fileSize);
	std::stringstream stream(fileContent);

	char aLine[255];
	while (stream.getline(aLine, sizeof(aLine)))
	{
		const char *pTStr = strchr(aLine, '=');
		if (!pTStr)
			continue;

		const unsigned int pos = pTStr-aLine;
		char *pOStr = (char*)malloc((pos + 1) * sizeof(char));
		memcpy(pOStr, aLine, pos);
		pOStr[pos] = 0;

		m_Translations.insert(std::make_pair(pOStr, pTStr+1));
		free(pOStr);
	}

	return true;
}
const char *CLocalization::getText(const std::string &str) const noexcept
{
	std::map<std::string, std::string>::const_iterator itstr = m_Translations.find(str);
	if (itstr == m_Translations.cend())
		return str.c_str();
	return (*itstr).second.c_str();
}
