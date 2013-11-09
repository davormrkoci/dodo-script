// Reference counted ptr.
// "The C++ Standard Library" by Josuttis, pg. 222

#if !defined(DSC_COUNTED_PTR_H_)
#define DSC_COUNTED_PTR_H_

#include <cassert>
#include "BaseTypes.h"
#include "ClassUtils.h"

namespace dsc
{
	template <class T>
	class CountedPtr
	{
	public:
		//interface
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
			CountedResource()
			: m_nRefs(0)
			{
			}

			virtual ~CountedResource()
			{
				assert(m_nRefs == 0);	//make sure that there are no more dangling counted ptrs
			}


			CountedResource(const CountedResource& rhs)
			: m_nRefs(0)
			{
			}

			CountedResource& operator=(const CountedResource& rhs)
			{
				//don't change the m_nRefs!  Pointers are still expecting it to be valid
				return *this;
			}

			void AddReference()
			{
				++m_nRefs;
			}

			void RemoveReference()
			{
				assert(m_nRefs > 0);
				--m_nRefs;
				if (m_nRefs == 0)
					delete this;
			}

			uint32 NumReferences()
			{
				return m_nRefs;
			}

		private:
			uint32 m_nRefs;
	};
}
#endif