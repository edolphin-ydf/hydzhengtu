
#ifndef MNET_DYNLIB_HPP
#define MNET_DYNLIB_HPP

#include <string>
////////////////////////////////////////////////////////////////
/// @namespace MNet
/// @brief MÍøÂç¿â
namespace MNet{
	//////////////////////////////////////////////////////
	//class DynLib
	//  Dynamic Library ( dll / so / dylib ) handler class
	//
	//
	//////////////////////////////////////////////////////
	class DynLib{

	public:
		//////////////////////////////////////////////////////
		//DynLib( const char *libfilename = "" )
		//  Constructor of the class
		//
		//Parameter(s)
		//  const char *libfilename  - filename with path
		//
		//Return Value
		//  None
		//
		//
		//////////////////////////////////////////////////////
		DynLib( const char *libfilename = "" );

		~DynLib();


		//////////////////////////////////////////////////////
		//bool Load()
		//  Loads the library if possible.
		//  Sets the error state of the class to true on error
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns true on success.
		//  Returns false on failure.
		//
		//
		//////////////////////////////////////////////////////
		bool Load();


		//////////////////////////////////////////////////////
		//void Close()
		//  Closes the library if possible.
		//  Sets the error state of the class on error.
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  None
		//
		//
		//////////////////////////////////////////////////////
		void Close();


		//////////////////////////////////////////////////////
		//void* GetAddressForSymbol( const char *symbol )
		//  Returns the address of the symbol.
		//  Sets the error state of the class on error
		//
		//Parameter(s)
		//  const char *symbol  -  identifier of the symbol
		//                         to be looked up.
		//
		//Return Value
		//  Returns the address on success.
		//  Returns NULL on failure.
		//
		//
		//////////////////////////////////////////////////////
		void* GetAddressForSymbol( const char *symbol );


		//////////////////////////////////////////////////////
		//bool Error()
		//  Returns the last error state of the class, and
		//  resets it to false.
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the last error state
		//
		//
		//////////////////////////////////////////////////////
		bool Error(){
			bool lasterror = error;
			
			error = false;
			
			return lasterror;
		}


		//////////////////////////////////////////////////////
		//std::string GetName()
		//  Returns the name of the library loaded
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the name of the library loaded
		//
		//
		//////////////////////////////////////////////////////
		std::string GetName(){
			return filename;
		}

	private:
		std::string filename;  // filename of the library file
		void *lptr;            // pointer to the opened library
		bool error;            // last error state
	};
}

#endif

