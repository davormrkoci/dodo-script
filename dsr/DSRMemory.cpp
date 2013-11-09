
#include "DSRMemory.h"
#include <malloc.h>
#include "DSRScriptManager.h"

namespace dsr
{
	//------------------------------------------------------------------------
	//default allocator implementation
	DefaultAllocator::DefaultAllocator(size_t maxNumAllocations)
	{
		DSR_ASSERT(maxNumAllocations > 0);
		m_maxNumAllocations = maxNumAllocations;
		m_pBase = (MemAllocEntry*) malloc(maxNumAllocations * sizeof(MemAllocEntry));
		DSR_ASSERT(m_pBase);

		for (uint32 i=0; i<m_maxNumAllocations; ++i)
		{
			m_pBase[i].pAlloc = 0;
		}
	}

	DefaultAllocator::~DefaultAllocator()
	{
		FreeAll();
		free(m_pBase);
	}

	void* DefaultAllocator::Alloc(size_t size)
	{
		const int32 idx = GetFirstFreeIdx();
		if (idx != -1)
		{
			m_pBase[idx].pAlloc = malloc(size);
			return m_pBase[idx].pAlloc;
		}

		//maximum number of concurent allocations reached
		DSR_ASSERT(false);
		return 0;
	}

	void DefaultAllocator::Free(void* ptr)
	{
		if (!ptr)
			return;

		for (uint32 i=0; i<m_maxNumAllocations; ++i)
		{
			if (m_pBase[i].pAlloc == ptr)
			{
				free(m_pBase[i].pAlloc);
				m_pBase[i].pAlloc = 0;
				return;
			}
		}

		//memory was not allocated with this allocator
		DSR_ASSERT(false);
	}

	void DefaultAllocator::FreeAll()
	{
		for (uint32 i=0; i<m_maxNumAllocations; ++i)
		{
			free(m_pBase[i].pAlloc);
			m_pBase[i].pAlloc = 0;
		}
	}

	int32 DefaultAllocator::GetFirstFreeIdx() const
	{
		for (uint32 i=0; i<m_maxNumAllocations; ++i)
		{
			if (m_pBase[i].pAlloc)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------
	//memory implementation
	Allocator* Memory::m_pAllocator = 0;

	void Memory::SetAllocator(Allocator* pAllocator)
	{
		//can only set allocator before script manager is created
		DSR_ASSERT(!ScriptManagerPtr());

		if (!ScriptManagerPtr())
		{
			m_pAllocator = pAllocator;
		}
	}

	void* Memory::Alloc(size_t size)
	{
		DSR_ASSERT(m_pAllocator);

		if (m_pAllocator)
			return m_pAllocator->Alloc(size);

		return 0;
	}

	void Memory::Free(void* ptr)
	{
		DSR_ASSERT(m_pAllocator);

		if (m_pAllocator)
			m_pAllocator->Free(ptr);
	}

	void Memory::FreeAll()
	{
		DSR_ASSERT(m_pAllocator);

		if (m_pAllocator)
			m_pAllocator->FreeAll();
	}
}