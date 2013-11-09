// (c) 2004 DodoSoft Inc.
#if !defined(DSR_ARRAY_H)
#define DSR_ARRAY_H

#include "DSRPlatform.h"
#include "DSRBaseTypes.h"
#include "DSRMemory.h"

namespace dsr
{
	template <class T> class Array
	{
	public:
		typedef Array<T> Self;
		typedef T value_type;
		typedef T* iterator;
		typedef const T* const_iterator;

		DSR_NEWDELETE(Array)

		Array();
		Array(uint32 size);
		Array(const Array& array);
		Array(const_iterator beginIt, const_iterator endIt);
		~Array();
		Array& operator = (const Array& array);

		uint32 size() const							{ return m_data ? *(reinterpret_cast<uint32*>(m_data)-1) : 0; }
		bool empty() const							{ return m_data == 0; }
		T& front()									{ return m_data[0]; }
		const T& front() const						{ return m_data[0]; }
		T& back()									{ return m_data[size()-1]; }
		const T& back() const						{ return m_data[size()-1]; }
		T& operator[] (uint32 index)					{ return m_data[index]; }
		const T& operator[] (uint32 index) const		{ return m_data[index]; }
		iterator begin()							{ return m_data; }
		iterator end()								{ return m_data + size(); }
		const_iterator begin() const				{ return m_data; }
		const_iterator end() const					{ return m_data + size(); }
		void resize(uint32 size)					{ clear(); Construct(size);	}
		void clear();

	private:
		void Alloc(uint32 size)
		{
			//allocate memory for data + uint32 for size
			uint32 *mem = static_cast<uint32*>(Memory::Alloc(size * sizeof(T) + 4));

			//set size
			*mem = size;

			//set data ptr
			m_data = reinterpret_cast<T*>(mem+1);
		}

		void Construct(uint32 size)
		{
			if (size == 0)
			{
				m_data = 0;
			}
			else
			{
				//allocate memory + set size
				Alloc(size);

				//comstruct data
				for (uint32 i = 0; i < size; ++i)
				{
					new(m_data+i) T();
				}
			}
		}

		void Construct(const_iterator beginIt, const_iterator endIt)
		{
			uint32 count = endIt - beginIt;

			if (count == 0)
			{
				m_data = 0;
			}
			else
			{
				//allocate memory + set size
				Alloc(count);

				//construct data
				for (uint32 i = 0; i < count; ++i)
				{
					new(m_data+i) T(*beginIt++);
				}
			}
		}

		void Destruct()
		{
			if (m_data != 0)
			{
				uint32 count = size();
				
				//call the destructor on the data
				for (uint32 i = 0; i < count; ++i)
				{
					m_data[i].~T();
				}

				//free memory
				Memory::Free(reinterpret_cast<uint32*>(m_data)-1);
			}
		}

		// Data
		T*		m_data;
	};

	template <class T> Array<T>::Array()
	: m_data(0)
	{
	}

	template <class T> Array<T>::Array(uint32 size)
	{ 
		Construct(size); 
	}

	template <class T> Array<T>::Array(const Array& array)	
	{ 
		Construct(array.begin(), array.end()); 
	}

	template <class T> Array<T>::Array(const_iterator beginIt, const_iterator endIt)
	{
		Construct(beginIt,endIt);
	}

	template <class T> Array<T>::~Array()
	{ 
		Destruct(); 
	}

	template <class T> Array<T>& Array<T>::operator=(const Array& array)
	{
		if (array.m_data != m_data)
		{
			Destruct();
			Construct(array.begin(), array.end());
		}

		return *this;
	}

	template <class T> void Array<T>::clear()
	{
		Destruct();
		m_data = 0;
	}
}

#endif
