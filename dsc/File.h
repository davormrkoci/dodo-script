#if !defined(DSC_FILE_H_)
#define DSC_FILE_H_

#include <string>
#include <vector>
#include "ClassUtils.h"
#include "BaseTypes.h"

namespace dsc
{
	class File
	{
		DSC_NOCOPY(File)

	public:
		typedef std::vector<std::string> StringVector;

		enum FileOpenMode
		{
			READ_BINARY,
			WRITE_BINARY,
			APPEND_BINARY,
		};

		File();
		~File();

		bool Open(const char* name, const FileOpenMode mode);
		void Close();
		uint32 Read(void* pBuffer, uint32 nBytes);
		uint32 Write(const void* pBuffer, uint32 nBytes);
		bool MovePos(int32 offset);
		int32 GetPos();
		bool SetPos(int32 pos);
		uint32 Size();
		const char* GetName() { return m_name.c_str(); }

		bool IsOpen() { return m_pFile != 0; }

	private:
		std::string m_name;
		FileOpenMode m_mode;
		FILE* m_pFile;
	};

	void FindFiles(const char* path, const char* name, File::StringVector& foundFiles, bool clear = true);
	void FindFilesRecursive(const char* path, const char* name, File::StringVector& foundFiles, bool clear = true);
	void SeparateDirName(const char* fullPath, std::string& dir, std::string& name);
	void ConvertToStandardDir(const char* dirName, std::string& dir);
}

#endif