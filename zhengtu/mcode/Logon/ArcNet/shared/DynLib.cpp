
#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include "DynLib.hpp"

////////////////////////////////////////////////////////////////
/// @namespace MNet
/// @brief MÍøÂç¿â
namespace MNet
{

	DynLib::DynLib(const char* libfilename)
	{
		if(libfilename == NULL)
			throw "Dynlib::Dynlib() encountered NULL library filename";

		filename = libfilename;
		lptr = NULL;
		error = false;
	}

	DynLib::~DynLib()
	{
		Close();
	}

#ifdef WIN32

	bool DynLib::Load()
	{
		lptr = LoadLibrary(filename.c_str());

		if(lptr != NULL)
		{
			return true;
		}
		else
		{
			error = true;
			return false;
		}
	}

	void* DynLib::GetAddressForSymbol(const char* symbol)
	{
		void* address = NULL;

		address = GetProcAddress(reinterpret_cast< HMODULE >(lptr), symbol);

		if(address == NULL)
			error = true;

		return address;
	}

	void DynLib::Close()
	{

		if(lptr != NULL)
		{
			int err = 0;

			err = FreeLibrary(reinterpret_cast< HMODULE >(lptr));

			if(err != 0)
				error = true;
		}
	}


#else

	bool DynLib::Load()
	{
		lptr = dlopen(filename.c_str(), RTLD_NOW);

		if(lptr != NULL)
		{
			return true;
		}
		else
		{
			error = true;
			return false;
		}
	}

	void* DynLib::GetAddressForSymbol(const char* symbol)
	{
		void* address = NULL;

		address = dlsym(lptr, symbol);

		if(address == NULL)
			error = true;

		return address;
	}

	void DynLib::Close()
	{

		if(lptr != NULL)
		{
			int err = 0;

			err = dlclose(lptr);

			if(err != 0)
				error = true;
		}
	}


#endif

}
