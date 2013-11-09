
#if !defined(DSR_LIST_H)
#define DSR_LIST_H

#include "DSRPlatform.h"
#include "DSRClassUtils.h"
#include "DSRMemory.h"

namespace dsr
{
	template <class T> class List
	{
	public:
		typedef List<T> Self;
		typedef T value_type;

		class Node
		{
		public:
			DSR_NEWDELETE(List)

			Node(const T& data) : m_data(data) {}

			Node*	m_next;
			T		m_data;
		};

		class const_iterator
		{
		public:
			DSR_NEWDELETE(const_iterator)

			const_iterator(Node* node = 0) : m_node(node)		{}
			const_iterator& operator ++ ()						{ m_node = m_node->m_next; return *this; }
			const_iterator operator ++ (int)					{ const_iterator it(*this); m_node = m_node->m_next; return it; }
			const T& operator* () const							{ return m_node->m_data; }
			const T* operator-> () const						{ return &m_node->m_data; }
			bool operator == (const const_iterator& it) const	{ return m_node == it.m_node; }
			bool operator != (const const_iterator& it) const	{ return m_node != it.m_node; }

		protected:
			Node*	m_node;
		};

		class iterator : public const_iterator
		{
		public:
			DSR_NEWDELETE(iterator)

			iterator(Node* node = 0) : const_iterator(node)		{}
			iterator& operator ++ ()							{ m_node = m_node->m_next; return *this; }
			iterator operator ++ (int)							{ iterator it(*this); m_node = m_node->m_next; return it; }
			T& operator* () const								{ return m_node->m_data; }
			T* operator-> () const								{ return &m_node->m_data; }
		};

		List();
		List(const List& list);
		~List();
		List& operator=(const List& list);

		size_t size() const;
		bool empty() const							{ return m_head == 0; }
		T& front()									{ return m_head->m_data; }
		const T& front() const						{ return m_head->m_data; }
		iterator begin()							{ return iterator(m_head); }
		iterator end()								{ return iterator(0); }
		const_iterator begin() const				{ return const_iterator(m_head); }
		const_iterator end() const					{ return const_iterator(0); }
		void clear();
		void push_front(const T& value);
		void push_front_unique(const T& value);
		void pop_front();
		void remove(const T& value);
		void swap(Self& list);

	private:
		static void* AllocNode()
		{
			return Memory::Alloc(sizeof(Node));
		}

		static void FreeNode(Node* node)
		{
			Memory::Free(node);
		}

		void Construct(const_iterator begin, const_iterator end)
		{
			Node** curr = &m_head;
			while (begin != end)
			{
				*curr = new(AllocNode()) Node(*begin);
				curr = &(*curr)->m_next;
				++begin;
			}
			*curr = 0;
		}

		void Destruct()
		{
			Node* curr = m_head;
			while (curr)
			{
				Node* next = curr->m_next;
				curr->~Node();
				FreeNode(curr);
				curr = next;
			}
		}

	private:
		Node* m_head;
	};

	template <class T> List<T>::List()
	: m_head(0)
	{
	}

	template <class T> List<T>::List(const List& list)
	{
		Construct(list.begin(), list.end());
	}

	template <class T> List<T>::~List()
	{
		Destruct();
	}

	template <class T> List<T>& List<T>::operator=(const List& list)
	{
		if (m_head != list.m_head)
		{
			Destruct();
			Construct(list.begin(), list.end());
		}

		return *this;
	}

	template <class T> size_t List<T>::size() const
	{
		size_t size = 0;

		for (Node* curr = m_head; curr; curr = curr->m_next)
		{
			++size;
		}

		return size;
	}

	template <class T> void List<T>::clear()
	{
		Destruct();
		m_head = 0;
	}

	template <class T> void List<T>::push_front(const T& value)
	{
		Node* node = new(AllocNode()) Node(value);
		node->m_next = m_head;
		m_head = node;
	}

	template <class T> void List<T>::push_front_unique(const T& value)
	{
		Node* node = m_head;

		while (node && !(node->m_data == value))
		{
			node = node->m_next;
		}

		if (node == 0)
		{
			push_front(value);
		}
	}

	template <class T> void List<T>::pop_front()
	{
		DE_ASSERT(m_head);

		Node* head = m_head->m_next;
		m_head->~Node();
		FreeNode(m_head);
		m_head = head;
	}

	template <class T> void List<T>::remove(const T& value)
	{
		Node** node = &m_head;

		while (*node)
		{
			if ((*node)->m_data == value)
			{
				Node* oldNode = *node;
				*node = (*node)->m_next;
				oldNode->~Node();
				FreeNode(oldNode);
				return;
			}
			node = &(*node)->m_next;
		}
	}

	template <class T> void List<T>::swap(Self& list)
	{
		Node* temp = m_head;
		m_head = list.m_head;
		list.m_head = temp;
	}
}

#endif
