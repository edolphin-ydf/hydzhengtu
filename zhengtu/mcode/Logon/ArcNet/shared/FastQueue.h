
#ifndef _FASTQUEUE_H
#define _FASTQUEUE_H

#include "MemoryPools/Node.h"

/** dummy lock to use a non-locked queue.
 */
class DummyLock
{
	public:
		MNET_INLINE void Acquire() { }
		MNET_INLINE void Release() { }
};

/** linked-list style queue
 */
template<class T, class LOCK>
class FastQueue
{
		struct node
		{
			T element;
			node* next;
		};

		node* last;
		node* first;
		LOCK m_lock;

	public:

		FastQueue()
		{
			last = NULL;
			first = NULL;
		}

		~FastQueue()
		{
			Clear();
		}

		void Clear()
		{
			// clear any elements
			while(last != NULL)
				Pop();
		}

		void Push(T elem)
		{
			m_lock.Acquire();
			node* n = new node;
			if(last)
				last->next = n;
			else
				first = n;

			last = n;
			n->next = NULL;
			n->element = elem;
			m_lock.Release();
		}

		T Pop()
		{
			m_lock.Acquire();
			if(first == NULL)
			{
				m_lock.Release();
				return reinterpret_cast<T>(NULL);
			}

			T ret = first->element;
			node* td = first;
			first = td->next;
			if(!first)
				last = NULL;

			delete td;
			m_lock.Release();
			return ret;
		}

		T front()
		{
			m_lock.Acquire();
			if(first == NULL)
			{
				m_lock.Release();
				return reinterpret_cast<T>(NULL);
			}

			T ret = first->element;
			m_lock.Release();
			return ret;
		}

		void pop_front()
		{
			m_lock.Acquire();
			if(first == NULL)
			{
				m_lock.Release();
				return;
			}

			node* td = first;
			first = td->next;
			if(!first)
				last = NULL;

			delete td;
			m_lock.Release();
		}

		bool HasItems()
		{
			bool ret;
			m_lock.Acquire();
			ret = (first != NULL);
			m_lock.Release();
			return ret;
		}
};

#endif
