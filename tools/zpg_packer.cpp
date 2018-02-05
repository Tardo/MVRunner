/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/*****************************************************
 * Syntaxes:
 * 		zpg_packer <ZPGFile> <options>
 *
 * Options:
 * 		- C							> Create the ZPG Package
 * 		- A <path>					> Adds the indicate file/directory into ZPG package
 * 		- L							> List all files inside ZPG package
 * 		- E <file>					> Extract file
 * 		- R <path>					> Remove
 * 		- O							> Overwrite mode
 * 		- M <path> <new path>		> Move the file to new path
 *
 * Example Usage:
 * 		zpg_packer mypack.zpg -C -A photo.png					> Create a new ZPG package 'mypack.zpg' and adds the "photo.png" file
 * 		zpg_packer mypack.zpg -A notes.txt						> Adds into 'mypack.zpg' the "notes.txt" file
 * 		zpg_packer mypack.zpg -A one/folder/data/				> Adds into 'mypack.zpg' the folder "data/"
 * 		zpg_packer mypack.zpg -L								> List files inside 'mypack.zpg'
 * 		zpg_packer mypack.zpg -E photo.png						> Extracts 'photo.png' from 'mypack.zpg'
 * 		zpg_packer mypack.zpg -E data/							> Extracts 'data/' folder from 'mypack.zpg'
 * 		zpg_packer mypack.zpg -R notes.txt						> Removes 'notes.txt' from 'mypack.zpg'
 * 		zpg_packer mypack.zpg -R data/							> Removes from 'mypack.zpg' the folder "data/"
 * 		zpg_packer mypack.zpg -O -A photo.png					> Adds into 'mypack.zpg' the "photo.png" file, overwriting if already exists into the package.
 * 		zpg_packer mypack.zpg -M notes.txt data/notes.txt		> Move 'notes.txt' to 'data/notes.txt'
 */
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <Zpg/Zpg.hpp>
#if defined(__linux__)
	#include <dirent.h>
	#include <sys/stat.h>
#elif defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <direct.h>
#endif
#include <string>

struct ZpgPackerOptions
{
	ZpgPackerOptions()
	{
		m_aToFile[0] = 0;
		m_aAddContentPath[0] = 0;
		m_aExtractContentPath[0] = 0;
		m_aRemoveContentPath[0] = 0;
		m_aOldContentPath[0] = 0;
		m_aNewContentPath[0] = 0;
		m_CreateMode = false;
		m_ListMode = false;
		m_OverwriteMode = false;
	}

    char m_aToFile[512];
    char m_aAddContentPath[1024];
    char m_aExtractContentPath[1024];
    char m_aRemoveContentPath[1024];
    char m_aOldContentPath[1024];
    char m_aNewContentPath[1024];
    bool m_CreateMode;
    bool m_ListMode;
    bool m_OverwriteMode;
};

void makeDir(const char *pPath)
{
	std::string path(pPath);
	std::size_t delPos = 0;
	while ((delPos = path.find_first_of("/\\", delPos+1)) != std::string::npos)
	{
	#if defined(__linux__)
		mkdir(path.substr(0, delPos+1).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    #elif defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
        CreateDirectory(path.substr(0, delPos).c_str(), NULL);
	#else
		#error Not Implemented!
	#endif
	}
}

bool extractFile(Zpg &zpg, const char *pPathFile)
{
	if (pPathFile[0] == '/' || (strlen(pPathFile) >= 3 && pPathFile[1] == ':' && pPathFile[2] == '\\') || pPathFile[0] == '.' || pPathFile[0] == '$')
	{
		std::cerr << "Invalid Destination Folder. For security reasons '" << pPathFile << "' can't be extracted!" << std::endl;
		return false;
	}

	unsigned long fileSize = 0;
	const unsigned char* pData = zpg.getFileData(pPathFile, &fileSize);
	if (!pData)
		return false;
	else
	{
		std::string dir(pPathFile);
		dir = dir.substr(0, dir.find_last_of("/\\")+1);
		makeDir(dir.c_str());
		std::ofstream file(pPathFile, std::ios::binary);
		if (file.is_open())
		{
			file.write(reinterpret_cast<const char*>(pData), fileSize);
			file.close();
		}
		else
			return false;
	}
	return true;
}

void extractDirectory(Zpg &zpg, const char *pPath)
{
	const std::map<std::string, ZpgFile*> &mFiles = zpg.getFiles();
	std::map<std::string, ZpgFile*>::const_iterator cit = mFiles.begin();
	while (cit != mFiles.end())
	{
		std::size_t fpos = (*cit).first.find(pPath, 0);
		if (fpos == 0)
		{
			std::cout << "Extracting '" << (*cit).first << "'... " << std::flush;
			const bool res = extractFile(zpg, (*cit).first.c_str());
			std::cout << (res?"OK":"FAILURE!") << std::endl;
		}
		++cit;
	}
}

void removeDirectory(Zpg &zpg, const char *pPath)
{
	const std::map<std::string, ZpgFile*> &mFiles = zpg.getFiles();
	std::map<std::string, ZpgFile*>::const_iterator cit = mFiles.begin();
	while (cit != mFiles.end())
	{
		std::size_t fpos = (*cit).first.find(pPath, 0);
		if (fpos == 0)
		{
			std::cout << "Removing '" << (*cit).first << "'... " << std::flush;
			const bool res = zpg.removeFile((*cit).first.c_str());
			std::cout << (res?"OK":"FAILURE!") << std::endl;
		}
		++cit;
	}
}

bool addDirectory(Zpg &zpg, const char *pFromFullPath, const char *pToFullPath, bool Overwrite)
{
	bool hasErrors = false;
#if defined(__linux__)
	struct dirent *pDirent;
	DIR *pDir = opendir(pFromFullPath);
	if (!pDir)
	{
		std::cerr << "Failed to open input directory" << std::endl;
		return false;
	}

	while ((pDirent = readdir(pDir)))
	{
		if (pDirent->d_name[0] == '.' || pDirent->d_name[0] == '$')
			continue;

		if (pDirent->d_type == DT_DIR)
		{
			char aNewFromPath[1024], aNewToPath[1024];
			snprintf(aNewFromPath, sizeof(aNewFromPath), "%s%s/", pFromFullPath, pDirent->d_name);
			snprintf(aNewToPath, sizeof(aNewToPath), "%s%s/", pToFullPath, pDirent->d_name);
			addDirectory(zpg, aNewFromPath, aNewToPath, Overwrite);
		}
		else
		{
			char aFileFromPath[1024], aFileToPath[1024];
			snprintf(aFileFromPath, sizeof(aFileFromPath), "%s%s", pFromFullPath, pDirent->d_name);
			snprintf(aFileToPath, sizeof(aFileToPath), "%s%s", pToFullPath, pDirent->d_name);
			std::cout << "Adding '" << aFileFromPath << "'... " << std::flush;
			const bool res = zpg.addFromFile(aFileFromPath, aFileToPath, Overwrite);
			if (!res)
				hasErrors = true;
			std::cout << (res?"OK":"FAILURE!") << std::endl;
		}
	}

	closedir(pDir);
#elif defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
    char wildcard[FILENAME_MAX];
    snprintf(wildcard, FILENAME_MAX, "%s*", pFromFullPath);

    HANDLE hFind;
    WIN32_FIND_DATA data;

    hFind = FindFirstFile(wildcard, &data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open input directory" << std::endl;
		return false;
    }
    else
    {
        do {
        	if (data.cFileName[0] == '.' || data.cFileName[0] == '$')
                continue;

            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                char aNewFromPath[1024], aNewToPath[1024];
                snprintf(aNewFromPath, sizeof(aNewFromPath), "%s%s/", pFromFullPath, data.cFileName);
                snprintf(aNewToPath, sizeof(aNewToPath), "%s%s/", pToFullPath, data.cFileName);
                addDirectory(zpg, aNewFromPath, aNewToPath, Overwrite);
            }
            else
            {
                char aFileFromPath[1024], aFileToPath[1024];
                snprintf(aFileFromPath, sizeof(aFileFromPath), "%s%s", pFromFullPath, data.cFileName);
                snprintf(aFileToPath, sizeof(aFileToPath), "%s%s", pToFullPath, data.cFileName);
                std::cout << "Adding '" << aFileFromPath << "'... " << std::flush;
                const bool res = zpg.addFromFile(aFileFromPath, aFileToPath, Overwrite);
                if (!res)
                    hasErrors = true;
                std::cout << (res?"OK":"FAILURE!") << std::endl;
            }
        } while (FindNextFile(hFind, &data));

        FindClose(hFind);
    }
#else
	#error Not Implemented!
#endif

	return !hasErrors;
}

bool parseInputOptions(ZpgPackerOptions *pOptions, int argc, char *argv[])
{
	if (argc <= 1)
		return false;

	strncpy(pOptions->m_aToFile, argv[1], sizeof(pOptions->m_aToFile));
    for (int i=2; i<argc; ++i)
    {
    	if (strlen(argv[i]) < 2 || argv[i][0] != '-')
    		continue;

    	switch (argv[i][1])
    	{
			case 'A':
				if (i < argc-1)
					strncpy(pOptions->m_aAddContentPath, argv[++i], sizeof(pOptions->m_aAddContentPath));
				break;
			case 'C':
				pOptions->m_CreateMode = true;
				break;
			case 'L':
				pOptions->m_ListMode = true;
				break;
			case 'E':
				if (i < argc-1)
					strncpy(pOptions->m_aExtractContentPath, argv[++i], sizeof(pOptions->m_aAddContentPath));
				break;
			case 'M':
				if (i < argc-2)
				{
					strncpy(pOptions->m_aOldContentPath, argv[++i], sizeof(pOptions->m_aOldContentPath));
					strncpy(pOptions->m_aNewContentPath, argv[++i], sizeof(pOptions->m_aNewContentPath));
				}
				break;
			case 'O':
				pOptions->m_OverwriteMode = true;
				break;
			case 'R':
				if (i < argc-1)
					strncpy(pOptions->m_aRemoveContentPath, argv[++i], sizeof(pOptions->m_aRemoveContentPath));
				break;
    	}
    }

    return true;
}


int main(int argc, char *argv[])
{
	Zpg myZpg;

    ZpgPackerOptions Options;
    if (!parseInputOptions(&Options, argc, argv))
    {
    	std::cerr << "Invalid Parameters!" << std::endl;
    	return -1;
    }


    bool NeedWrite = false;
	if (!Options.m_CreateMode)
	{
		if (!myZpg.open(Options.m_aToFile))
			return -1;
	}

	if (Options.m_aAddContentPath[0] != 0)
	{
		std::string path(Options.m_aAddContentPath);
		std::size_t delPos = path.find_last_of("/\\");

		if (delPos == path.size()-1) // Directory
		{
			std::size_t delPosPrev = path.find_last_of("/\\", delPos-1);
			std::string folderName = (delPosPrev == std::string::npos)?path.substr(0, delPos+1):path.substr(delPosPrev+1, delPos+1);
			addDirectory(myZpg, Options.m_aAddContentPath, folderName.c_str(), Options.m_OverwriteMode);
		}
		else
		{
			std::cout << "Adding '" << Options.m_aAddContentPath << "'... ";
			const bool res = myZpg.addFromFile(Options.m_aAddContentPath, (delPos == std::string::npos)?path.c_str():path.substr(delPos+1).c_str(), Options.m_OverwriteMode);
			std::cout << (res?"OK":"FAILURE!") << std::endl;
		}

		NeedWrite = true;
	}

	if (Options.m_aExtractContentPath[0] != 0)
	{
		std::string path(Options.m_aAddContentPath);
		std::size_t delPos = path.find_last_of("/\\");

		if (delPos == path.size()-1) // Directory
		{
			extractDirectory(myZpg, Options.m_aExtractContentPath);
		}
		else
		{
			std::cout << "Extracting '" << Options.m_aExtractContentPath << "'... " << std::flush;
			const bool res = extractFile(myZpg, Options.m_aExtractContentPath);
			std::cout << (res?"OK":"FAILURE!") << std::endl;
		}
	}

	if (Options.m_aRemoveContentPath[0] != 0)
	{
		std::string path(Options.m_aRemoveContentPath);
		std::size_t delPos = path.find_last_of("/\\");

		if (delPos == path.size()-1) // Directory
		{
			removeDirectory(myZpg, Options.m_aRemoveContentPath);
		}
		else
		{
			std::cout << "Removing '" << Options.m_aRemoveContentPath << "'... " << std::flush;
			const bool res = myZpg.removeFile(Options.m_aRemoveContentPath);
			std::cout << (res?"OK":"FAILURE!") << std::endl;
		}

		NeedWrite = true;
	}

	if (Options.m_aOldContentPath[0] != 0 && Options.m_aNewContentPath[0] != 0)
	{
		std::cout << "Moving '" << Options.m_aOldContentPath << "' to '" << Options.m_aNewContentPath << "'... " << std::flush;
		const bool res = myZpg.moveFile(Options.m_aOldContentPath, Options.m_aNewContentPath);
		std::cout << (res?"OK":"FAILURE!") << std::endl;

		NeedWrite = true;
	}

	if (NeedWrite)
	{
		std::cout << "Saving '" << Options.m_aToFile << "', using ZLib... " << std::flush;
		const bool res = myZpg.saveToFile(Options.m_aToFile);
		std::cout << (res?"SUCCESS":"FAILURE!") << std::endl;
	}

	if (Options.m_ListMode)
	{
		const std::map<std::string, ZpgFile*> &mFiles = myZpg.getFiles();
		std::cout << "Num. Files: " << mFiles.size() << std::endl;
		std::map<std::string, ZpgFile*>::const_iterator It = mFiles.begin();
		while (It != mFiles.end())
		{
			const float pc = ((*It).second->m_Header.m_FileSize - (*It).second->m_Header.m_FileSizeComp) * 100.0f / (*It).second->m_Header.m_FileSize;
			std::cout << std::dec << (*It).first << " [CSize: " << (*It).second->m_Header.m_FileSizeComp << "][Size: " << (*It).second->m_Header.m_FileSize << "]" << "[" << std::fixed << std::setprecision(2) << pc << "%]" << std::endl;
			++It;
		}
	}

    return 0;
}
