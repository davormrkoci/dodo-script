
#if !defined(DSR_MEMORY_H_)
#define DSR_MEMORY_H_

#include "DSRPlatform.h"
#include "DSRBaseTypes.h"
#include "DSRClassUtils.h"

//macro for operators new/delete
#define DSR_NEWDELETE(classname)							\
		static void* operator new(size_t size)				\
		{													\
			return Memory::Alloc(size);						\
		}													\
		static void* operator new(size_t size, void* ptr)	\
		{													\
			return ptr;										\
		}													\
		static void operator delete(void* ptr)				\
		{													\
			Memory::Free(ptr);								\
		}													\
		static void operator delete(void* ptr, void*)		\
		{													\
		}													\
		static void* operator new[](size_t size)			\
		{													\
			return Memory::Alloc(size);						\
		}													\
		static void* operator new[](size_t size, void* ptr)	\
		{													\
			return ptr;										\
		}													\
		static void operator delete[](void* ptr)			\
		{													\
			Memory::Free(ptr);								\
		}													\
		static void operator delete[](void* ptr, void*)		\
		{													\
		}													\

namespace dsr
{
	/// Interface class for memory allocators
	class Allocator
	{
	public:
		virtual ~Allocator() {}
		/// Request memory block of [size] number of bytes
		virtual void* Alloc(size_t size) = 0;
		/// Free memory block [ptr]
		virtual void Free(void* ptr) = 0;
		/// Free all memory blocks allocated with this allocator
		virtual void FreeAll() = 0;
	};

	/// Default allocator implementation.
	/// Uses malloc for allocating memory blocks.
	class DefaultAllocator : public Allocator
	{
		DSR_NOCOPY(DefaultAllocator)
	public:
		explicit DefaultAllocator(size_t maxNumAllocations);
		virtual ~DefaultAllocator();
		virtual void* Alloc(size_t size);
		virtual void Free(void* ptr);
		virtual void FreeAll();

	private:
		DefaultAllocator();		//not implemented


	private:
		int32 GetFirstFreeIdx() const;

	private:
		class MemAllocEntry
		{
		public:
			void* pAlloc;
		};

		MemAllocEntry* m_pBase;
		size_t m_maxNumAllocations;
	};

	/// Memory allocation/deallocation interface for dodoScript.
	class Memory
	{
	public:
		/// Register the allocator that will be used by dodoScript system
		/// for all memory allocations.  Only one allocator can be registered.
		/// This function needs to be called before the call to
		/// ScriptManager::Create()
		static void SetAllocator(Allocator* pAllocator);
		/// Request memory block of [size] number of bytes
		/// Called by dodoScript system.
		static void* Alloc(size_t size);
		/// Free memory block [ptr]
		/// Called by dodoScript system.
		static void Free(void* ptr);
		/// Free all memory blocks allocated with this allocator
		/// Called by dodoScript system.
		static void FreeAll();

	private:
		static Allocator* m_pAllocator;
	};
}

#endif