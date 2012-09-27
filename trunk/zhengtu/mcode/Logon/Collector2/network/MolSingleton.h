#ifndef _MOL_SINGLETON_H_INCLUDE
#define _MOL_SINGLETON_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:单件类
* 作者:akinggw
* 日期:2010.2.11
*/

#include "MolCommon.h"

#define initialiseSingleton( type ) \
	template <> type * Singleton < type > :: mSingleton = 0

#define initialiseTemplateSingleton( temp, type ) \
	template <> temp< type > * Singleton < temp< type > > :: mSingleton = 0

template < class type > class Singleton
{
public:
	/// 构造函数
	Singleton()
	{
		assert(this->mSingleton == 0);
		this->mSingleton = static_cast<type *>(this);
	}
	/// 析构函数
	virtual ~Singleton()
	{
		this->mSingleton = 0;
	}

	inline static type & getSingleton() { assert(mSingleton); return *mSingleton; }
	inline static type * getSingletonPtr() { return mSingleton; }

protected:
	static type * mSingleton;
};

#endif
