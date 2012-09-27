
#ifndef MNET_FINDFILES_HPP
#define MNET_FINDFILES_HPP

#include "FindFilesResult.hpp"

namespace MNet{
	//////////////////////////////////////////////////////////////////////////////////
	//bool FindFiles( const char *where, const char *filename, FindFilesResult &r )
	//  Searches for files with the name of *filename*, and
	//  puts the result(s) into a FindFileResult object.
	//
	//Parameter(s)
	//  const char *where     -  directory to search
	//  const char *filename  -  filename/mask to search for
	//  FindFilesResult &r    -  reference to the result object
	//
	//Return Value
	//  Returns true if at least 1 file was found.
	//  Returns false if there were no files found.
	//
	//
	//////////////////////////////////////////////////////////////////////////////////
	bool FindFiles( const char *where, const char *filename, FindFilesResult &r );
}

#endif
