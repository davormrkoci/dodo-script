
#include <cassert>
#include "File.h"
#include "StringUtils.h"

namespace dsc
{
	void SeparateDirName(const char* fullPath, std::string& dir, std::string& name)
	{
		dir = "";
		name = "";
		const std::string fp = fullPath;

		std::string::size_type i = fp.rfind('/');
		if (i == std::string::npos)
		{
			name = fp;
		}
		else
		{
			name = fp.substr(i+1);
			dir = fp.substr(0, i);
		}
	}

	File::File()
	{
		m_name = "";
		m_mode = READ_BINARY;
		m_pFile = 0;
	}

	File::~File()
	{
		Close();
	}

	bool File::Open(const char* name, const FileOpenMode mode)
	{
		//close current file
		Close();

		//set the vars
		m_name = name;
		m_mode = mode;

		//prepare the mode string
		std::string modeStr = "";
		switch(mode)
		{
		case READ_BINARY:
			modeStr = "rb";
			break;

		case WRITE_BINARY:
			modeStr = "wb";
			break;

		case APPEND_BINARY:
			modeStr = "ab";
			break;
		};

		//try to open the file
		m_pFile = fopen(m_name.c_str(), modeStr.c_str());

		return (m_pFile != 0);
	}

	void File::Close()
	{
		if (m_pFile)
		{
			int fileClosed = fclose(m_pFile);
			m_pFile = 0;

			//was the file really closed?
			assert(fileClosed == 0);
		}
	}

	uint32 File::Read(void* pBuffer, uint32 nBytes)
	{
		if (m_pFile)
		{
			if (m_mode == READ_BINARY)
			{
				return (uint32) fread(pBuffer, 1, nBytes, m_pFile);
			}
		}

		assert(false);
		return 0;
	}

	uint32 File::Write(const void* pBuffer, uint32 nBytes)
	{
		if (m_pFile)
		{
			if (m_mode == WRITE_BINARY || m_mode == APPEND_BINARY)
			{
				return (uint32) fwrite(pBuffer, 1, nBytes, m_pFile);
			}
		}

		assert(false);
		return 0;
	}

	bool File::MovePos(int32 offset)
	{
		bool success = false;

		if (m_pFile)
		{
			if (fseek(m_pFile, offset, SEEK_CUR) == 0)
				success = true;
		}

		return success;
	}

	int32 File::GetPos()
	{
		int32 pos = -1;

		if (m_pFile)
		{
			pos = ftell(m_pFile);
		}

		return pos;
	}

	bool File::SetPos(int32 pos)
	{
		bool success = false;

		if (m_pFile)
		{
			if (fseek(m_pFile, pos, SEEK_SET) == 0)
				success = true;
		}

		return success;
	}

	uint32 File::Size()
	{
		uint32 size = 0;

		if (m_pFile)
		{
			//remember cur pos
			int32 oldPos = GetPos();

			//get start pos
			rewind(m_pFile);
			int32 start = ftell(m_pFile);

			//get end pos
			int32 end = -1;
			if (fseek(m_pFile, 0, SEEK_END) == 0)
				end = ftell(m_pFile);

			//get size
			if (start >= 0 && end >= 0 && start <= end)
				size = end - start;

			//reset to old pos
			SetPos(oldPos);
		}

		return size;
	}



}