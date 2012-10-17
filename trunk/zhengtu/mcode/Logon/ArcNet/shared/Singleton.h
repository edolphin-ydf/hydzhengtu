
#ifndef WOWSERVER_SINGLETON_H
#define WOWSERVER_SINGLETON_H

#include "Errors.h"
#include "MemoryPools/Node.h"
/// 应该放在.cpp文件中,其实就相当于class::mSingleton = 0
#define initialiseSingleton( type ) \
  template <> type * Singleton < type > :: mSingleton = 0

#define initialiseTemplateSingleton( temp, type ) \
  template <> temp< type > * Singleton < temp< type > > :: mSingleton = 0

/// To be used as a replacement for initialiseSingleton( )
///  Creates a file-scoped Singleton object, to be retrieved with getSingleton
#define createFileSingleton( type ) \
  initialiseSingleton( type ); \
  type the##type

template < class type > class SERVER_DECL Singleton
{
	public:
		/// Constructor
		Singleton()
		{
			/// If you hit this assert, this singleton already exists -- you can't create another one!
			ASSERT(this->mSingleton == 0);
			this->mSingleton = static_cast<type*>(this);
		}
		/// Destructor
		virtual ~Singleton()
		{
			this->mSingleton = 0;
		}

		MNET_INLINE static type & getSingleton() { ASSERT(mSingleton); return *mSingleton; }
		MNET_INLINE static type* getSingletonPtr() { return mSingleton; }

	protected:

		/// Singleton pointer, must be set to 0 prior to creating the object
		static type* mSingleton;
};
#endif
