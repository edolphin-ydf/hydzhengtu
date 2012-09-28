
#ifndef FQUEUE_H
#define FQUEUE_H

#include "Mutex.h"

template<class T>
class FQueue
{
	public:
		FQueue() { first = last = NULL; size = 0; }
		volatile unsigned int size;

		uint32 get_size()
		{
			lock.Acquire();
			::uint32 retval = size;
			lock.Release();
			return retval;
		}

		void push(T & item)
		{
			h* p = new h;
			p->value = item;
			p->pNext = NULL;

			lock.Acquire();
			if(last != NULL)//have some items
			{
				last->pNext = (h*)p;
				last = p;
				++size;
			}
			else //first item
			{
				last = first = p;
				size = 1;
			}
			lock.Release();
		}

		T pop_nowait() { return pop(); }

		T pop()
		{
			lock.Acquire();
			if(size == 0)
			{
				lock.Release();
				return NULL;
			}

			h* tmp = first;
			if(tmp == NULL)
			{
				lock.Release();
				return NULL;
			}

			if(--size) //more than 1 item
				first = (h*)first->pNext;
			else //last item
			{
				first = last = NULL;
			}

			lock.Release();

			T returnVal = tmp->value;
			delete tmp;
			return returnVal;
		}

	private:
		struct h
		{
			T value;
			void* pNext;
		};

		h* first;
		h* last;

		Mutex lock;
};

#endif


