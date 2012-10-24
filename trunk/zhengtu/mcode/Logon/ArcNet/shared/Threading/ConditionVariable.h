
#ifndef CONDITIONVARIABLE_H
#define CONDITIONVARIABLE_H
////////////////////////////////////////////////////////////////
/// @namespace MCodeNet
/// @brief M�����
namespace MCodeNet
{

	namespace Threading
	{

		/////////////////////////////////////////////////////////////////
		//class ConditionVariable
		//  ��ʵ��һ��ƽ̨��������������
		//
		//
		/////////////////////////////////////////////////////////////////
		class SERVER_DECL ConditionVariable
		{

			public:
				ConditionVariable();

				~ConditionVariable();

				//////////////////////////////////////////////////////////////////////////
				//void Signal()
				//  �ź���������,�����������̼߳�����ȥ��
				//
				//Parameter(s)
				//  None
				//
				//Return Value
				//  None
				//
				//
				//////////////////////////////////////////////////////////////////////////
				void Signal();


				/////////////////////////////////////////////////////////////////////////
				//void Wait( unsigned long timems )
				//  ����ִ�е����߳�,ֱ�����źŻ�ʱ��
				//  
				//
				//Parameter(s)
				//  unsigned long timems  -  Maximum time to block in milliseconds
				//
				//Return Value(s)
				//  None
				//
				//
				/////////////////////////////////////////////////////////////////////////
				void Wait(unsigned long timems);

			private:

#ifdef WIN32
				HANDLE hEvent;
#else
				pthread_cond_t cond;
				pthread_mutex_t mutex;
#endif
		};
	}
}

#endif
