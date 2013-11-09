#include <windows.h>
#include "File.h"
#include "StringUtils.h"

namespace dsc
{
	static bool IsDirectory(const WIN32_FIND_DATA& data)
	{
		return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	static bool IsDot(const WIN32_FIND_DATA& data)
	{
		if (IsDirectory(data))
		{
			if (strcmp(data.cFileName, ".") == 0
				|| strcmp(data.cFileName, "..") == 0)
			{
				return true;
			}
		}

		return false;
	}

	void FindFiles(const char* path, const char* name, File::StringVector& foundFiles, bool clear)
	{
		if (clear)
		{
			foundFiles.clear();
		}

		std::string searchString;
		ConvertToStandardDir(path, searchString);
		searchString += name;

		WIN32_FIND_DATA data;
		HANDLE h = FindFirstFile(searchString.c_str(), &data);
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!IsDirectory(data))
				{
					std::string file = path;
					if (file.empty() || (file[file.size()-1] != '/'
						&& file[file.size()-1] != '\\'))
					{
						file += "/";
					}
					file += data.cFileName;
					foundFiles.push_back(file);
				}
			}
			while (FindNextFile(h, &data));

			FindClose(h);
		}
	}

	void FindFilesRecursive(const char* path, const char* name, File::StringVector& foundFiles, bool clear)
	{
		if (clear)
		{
			foundFiles.clear();
		}

		//get files in dir
		FindFiles(path, name, foundFiles, false);

		//search other dirs
		std::string searchString;
		ConvertToStandardDir(path, searchString);
		searchString += "*.*";

		WIN32_FIND_DATA data;
		HANDLE h = FindFirstFile(searchString.c_str(), &data);
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (IsDirectory(data) && !IsDot(data))
				{
					std::string nextPath;
					ConvertToStandardDir(path, nextPath);
					nextPath += data.cFileName;
					FindFilesRecursive(nextPath.c_str(), name, foundFiles, false);
				}
			}
			while (FindNextFile(h, &data));

			FindClose(h);
		}
	}

	void ConvertToStandardDir(const char* dirName, std::string& dir)
	{
		dir = dirName;
		ReplaceChar(dir, '\\', '/');
		Downcase(dir);
		if (dir[dir.size()-1] != '/')
			dir += '/';
	}
}