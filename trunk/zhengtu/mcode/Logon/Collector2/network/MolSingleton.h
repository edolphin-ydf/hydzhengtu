#ifndef _MOL_SINGLETON_H_INCLUDE
#define _MOL_SINGLETON_H_INCLUDE

/** 
* MolNet��������
*
* ����:������
* ����:akinggw
* ����:2010.2.11
*/

#include "MolCommon.h"

#define initialiseSingleton( type ) \
	template <> type * Singleton < type > :: mSingleton = 0

#define initialiseTemplateSingleton( temp, type ) \
	template <> temp< type > * Singleton < temp< type > > :: mSingleton = 0

template < class type > class Singleton
{
public:
	/// ���캯��
	Singleton()
	{
		assert(this->mSingleton == 0);
		this->mSingleton = static_cast<type *>(this);
	}
	/// ��������
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
