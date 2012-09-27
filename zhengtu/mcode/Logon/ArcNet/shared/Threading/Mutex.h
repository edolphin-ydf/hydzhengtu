/*
 * ArcEmu MMORPG Server
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _THREADING_MUTEX_H
#define _THREADING_MUTEX_H

class SERVER_DECL Mutex
{
	public:
		//friend class Condition;

		/** Initializes a mutex class, with InitializeCriticalSection / pthread_mutex_init
		 */
		Mutex();

		/** Deletes the associated critical section / mutex
		 */
		virtual ~Mutex();

		/** Attempts to acquire this mutex. If it cannot be acquired (held by another thread)
		 * it will return false.
		 * @return false if cannot be acquired, true if it was acquired.
		 尝试获取互斥锁。
		 如果它不能被获取(被另一个线程锁住),它将返回false。如果不能获得返回false,获得则返回true。
		 */
		bool AttemptAcquire();

		/** Acquires this mutex. If it cannot be acquired immediately, it will block.
		//获得互斥锁。如果它不能立即获得,它将会一直阻塞。
		 */
		void Acquire();

		/** Releases this mutex. No error checking performed
		//释放互斥锁。没有错误检查执行
		 */
		void Release();

	protected:
#ifdef WIN32
		/** Critical section used for system calls
		 */
		CRITICAL_SECTION cs;

#else
		/** Static mutex attribute
		 */
		static bool attr_initalized;
		static pthread_mutexattr_t attr;

		/** pthread struct used in system calls
		 */
		pthread_mutex_t mutex;
#endif
};

#ifdef WIN32

class SERVER_DECL FastMutex
{
#pragma pack(push,8)
		volatile long m_lock;
#pragma pack(pop)
		DWORD m_recursiveCount;

	public:
		FastMutex() : m_lock(0), m_recursiveCount(0) {}

		~FastMutex() {}

		bool AttemptAcquire();

		void Acquire();

		void Release();
};

#else

#define FastMutex Mutex

#endif

#endif

