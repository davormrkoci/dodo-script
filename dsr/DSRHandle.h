// (c) 2004 DodoSoft Inc.
#if !defined(DSR_HANDLE_H_)
#define DSR_HANDLE_H_

#include "DSRPlatform.h"
#include "DSRCountedPtr.h"

namespace dsr
{
	template <class T> class HandleOwner;

	template <class T> class Handle : public CountedResource
	{
		friend class HandleOwner<T>;

		public:
			DSR_NEWDELETE(Handle)

			explicit Handle(T* ptr = 0)
			: m_ptr(ptr)
			{
			}

			const T* get() const
			{
				return m_ptr;
			}

			T* get()
			{
				return m_ptr;
			}

		private:
			//stubbed out
			Handle(const Handle& rhs);
			const Handle& operator==(const Handle& rhs);

		private:
			T* m_ptr;
	};

	template <class T> class HandleOwner
	{
			DSR_NOCOPY(HandleOwner)
		public:
			DSR_NEWDELETE(HandleOwner)

			explicit HandleOwner(T* pObject = 0)
			: m_cpHandle(new Handle<T>(pObject))
			{
				DSR_ASSERT(m_cpHandle.get());
			}

			~HandleOwner()
			{
				DSR_ASSERT(m_cpHandle.get());
				SetHandle(0);
			}

			Handle<T>* GetHandlePtr()
			{
				return m_cpHandle.get();
			}

			void SetHandle(T* pObject)
			{
				DSR_ASSERT(m_cpHandle.get());
				m_cpHandle.get()->m_ptr = pObject;
			}

		private:
			CountedPtr<Handle<T> > m_cpHandle;
	};
}

#endif