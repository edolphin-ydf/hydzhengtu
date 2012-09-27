
#ifndef MNET_SYSINFO_HPP
#define MNET_SYSINFO_HPP

namespace MNet{
	////////////////////////////////////////////////////////
	//class SysInfo
	//  Class with static methods capable of retrieving
	//  some basic system information
	//
	///////////////////////////////////////////////////////
	class SysInfo{
	public:
		////////////////////////////////////////////////////////////////////
		//static int GetCPUCount()
		//  Tells the number of CPUs installed in the system
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the number of CPUs
		//
		//
		////////////////////////////////////////////////////////////////////
		static long GetCPUCount();


		////////////////////////////////////////////////////////////////////
		//static unsigned long long GetCPUUsage()
		//  Tells the CPU time ( kernel and user )spent on executing
		//  the caller process.
		//
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the used CPU time in microseconds.
		//
		//
		////////////////////////////////////////////////////////////////////
		static unsigned long long GetCPUUsage();


		////////////////////////////////////////////////////////////////////
		//static unsigned long GetRAMUsage()
		//  Tells the RAM usage of the caller process.
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the number of bytes used by the caller process
		//
		//
		////////////////////////////////////////////////////////////////////
		static unsigned long long GetRAMUsage();


		////////////////////////////////////////////////////////////////////
		//static unsigned long long GetTickCount()
		//  Returns the time elapsed since some starting point, with
		//  milliseconds precision.
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the milliseconds elapsed since some point in the past
		//
		//
		////////////////////////////////////////////////////////////////////
		static unsigned long long GetTickCount();

	};
}

#endif
