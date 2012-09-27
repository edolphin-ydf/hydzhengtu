

#ifndef RWLOCK_H
#define RWLOCK_H

#include "Mutex.h"

class RWLock
{
	public:
		MNET_INLINE void AcquireReadLock()
		{
			_lock.Acquire();
		}

		MNET_INLINE void ReleaseReadLock()
		{
			_lock.Release();
		}

		MNET_INLINE void AcquireWriteLock()
		{
			_lock.Acquire();
		}

		MNET_INLINE void ReleaseWriteLock()
		{
			_lock.Release();
		}

	private:
		Mutex _lock;
};

#endif
