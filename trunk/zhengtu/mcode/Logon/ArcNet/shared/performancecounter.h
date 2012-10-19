
#ifndef MNET_PERFORMANCECOUNTER_HPP
#define MNET_PERFORMANCECOUNTER_HPP
#include "Common.h"
////////////////////////////////////////////////////////////////
/// @namespace MNet
/// @brief MÍøÂç¿â
namespace MNet{

	/////////////////////////////////////////////////////////
	//class PerformanceCounter
	//  A simple performance counter class, that samples
	//  resource usages and tells the usage in percentages
	//
	//
	////////////////////////////////////////////////////////
	class SERVER_DECL PerformanceCounter{

	public:
		PerformanceCounter();

		
		
		~PerformanceCounter(){}

		
		/////////////////////////////////////////////////////////////////////
		//float GetCurrentCPUUsage()
		//  Returns the CPU usage in percentage since the last sample
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the CPU usage in percentage
		//
		//
		////////////////////////////////////////////////////////////////////
		float GetCurrentCPUUsage();



		////////////////////////////////////////////////////////////////////
		//float GetCurrentRAMUsage()
		//  Tells the current RAM usage in bytes
		//
		//Parameter(s)
		//  None
		//
		//Return Value
		//  Returns the current RAM usage in bytes
		//
		//
		////////////////////////////////////////////////////////////////////
		float GetCurrentRAMUsage();
	
	private:
		unsigned long cpu_count;             // The number of CPUs in this system  ( CPU cores count as separate CPUs )
		unsigned long long last_update;           // The time the last sample was created
		unsigned long long last_cpu_usage;   // The last sample of CPU usage
	};

}

#endif
