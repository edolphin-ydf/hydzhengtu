
#ifndef CREFCOUNTER_HPP
#define CREFCOUNTER_HPP

namespace MNet
{
	namespace Shared
	{
		/////////////////////////////////////////////////////////////////////
		//class CRefCounter
		//  Reference Counter class.
		//  Reference counting starts with 1 reference, on instantiation
		//
		//
		/////////////////////////////////////////////////////////////////////
		class SERVER_DECL CRefCounter
		{

			public:
				CRefCounter() { Counter.SetVal(1); }


				virtual ~CRefCounter() {}


				////////////////////////////////////////////////////////////////
				//void AddRef()
				//  Increases the reference count by 1
				//
				//Parameters
				//  None
				//
				//Return Value
				//  None
				//
				//
				////////////////////////////////////////////////////////////////
				void AddRef() { ++Counter; }


				////////////////////////////////////////////////////////////////
				//void DecRef()
				//  Decreases the reference count by 1. When it reaches 0,
				//  the object is deleted
				//
				//Parameters
				//  None
				//
				//Return Value
				//  None
				//
				//
				////////////////////////////////////////////////////////////////
				void DecRef()
				{
					if(--Counter == 0)
						delete this;
				}


			private:
				MNet::Threading::AtomicCounter Counter;

		};
	}
}

#endif
