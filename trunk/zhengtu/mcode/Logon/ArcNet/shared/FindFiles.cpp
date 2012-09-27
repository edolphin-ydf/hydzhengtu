
#include "FindFiles.hpp"

#ifdef WIN32
#include "Windows.h"
#else
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#endif

namespace MNet
{

#ifdef WIN32

	bool FindFiles(const char* where, const char* filename, FindFilesResult & r)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;

		std::string fname(where);
		fname += "*";

		if(filename != NULL)
		{
			fname += filename;
			fname += "*";
		}

		hFind = FindFirstFile(fname.c_str(), &FindFileData);

		if(hFind == INVALID_HANDLE_VALUE)
			return false;

		do
		{
			r.Add(FindFileData.cFileName);
		}
		while(FindNextFile(hFind, &FindFileData));

		FindClose(hFind);

		return true;
	}

#else

	bool FindFiles(const char* where, const char* filename, FindFilesResult & r)
	{
		int n = 0;
		dirent** filelist = NULL;

		n = scandir(where, &filelist, NULL, NULL);

		if(n < 0)
			return false;
		else
			while(n-- > 0)
			{
				if((filename == NULL) || (strstr(filename, "*") != NULL) || (strstr(filelist[ n ]->d_name, filename) != NULL))
					r.Add(filelist[ n ]->d_name);

				free(filelist[ n ]);
			}

		free(filelist);

		return true;
	}

#endif
}
