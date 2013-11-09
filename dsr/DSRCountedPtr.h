
#if !defined(DSR_COUNTEDPTR_H_)
#define DSR_COUNTEDPTR_H_

#include "DSRPlatform.h"
#include "DSRMemory.h"
#include "DSRClassUtils.h"

namespace dsr
{
	template <class T> class CountedPtr
	{
	public:
		DSR_NEWDELETE(CountedPtr)

		CountedPtr(T* p=0)
		: m_ptr(p)
		{
			if (m_ptr)
				m_ptr->AddReference();
		}
		
		CountedPtr(const CountedPtr<T>& cp)
		: m_ptr(cp.m_ptr)
		{
			if (m_ptr)
				m_ptr->AddReference();
		}
		
		virtual ~CountedPtr()
		{
			if (m_ptr)
				m_ptr->RemoveReference();
		}
		
		CountedPtr<T>& operator=(const CountedPtr<T>& cp)
		{
			if (m_ptr)
				m_ptr->RemoveReference();

			m_ptr = cp.m_ptr;

			if (m_ptr)
				m_ptr->AddReference();

			return *this;
		}
		
		const T* get() const
		{
			return m_ptr;
		}
		
		T* get()
		{
			return m_ptr;
		}

		void set(T* ptr)
		{
			if (m_ptr)
				m_ptr->RemoveReference();

			m_ptr = ptr;

			if (m_ptr)
				m_ptr->AddReference();
		}

		operator T* () const
		{
			return m_ptr;
		}

		T* operator-> () const
		{
			return m_ptr;
		}

		T& operator* () const
		{
			return *m_ptr;
		}

		bool operator==(const CountedPtr<T>& rhs) const
		{
			return (m_ptr == rhs.m_ptr);
		}

		bool operator<(const CountedPtr<T>& rhs) const
		{
			return (m_ptr < rhs.m_ptr);
		}

	private:
		//data
		T* m_ptr;
	};

	class CountedResource
	{
		public:
			DSR_NEWDELETE(CountedResource)

			CountedResource()
			: m_nRefs(0)
			{
			}

			virtual ~CountedResource()
			{
				DSR_ASSERT(m_nRefs == 0);
			}

			CountedResource(const CountedResource& rhs)
			: m_nRefs(0)
			{
			}

			CountedResource& operator=(const CountedResource& rhs)
			{
			}

			void AddReference()
			{
				++m_nRefs;
			}

			void RemoveReference()
			{
				DSR_ASSERT(m_nRefs > 0);
				--m_nRefs;
				if (m_nRefs == 0)
					delete this;
			}

			size_t NumReferences()
			{
				return m_nRefs;
			}

		private:
			size_t m_nRefs;
	};
}

#endif