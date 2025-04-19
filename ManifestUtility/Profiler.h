#pragma once
#include <chrono>
#include <iostream>

namespace Manifest_Utility
{
	class Profiler
	{
	protected:
		static std::chrono::steady_clock* mainProfiler;
		static std::chrono::duration<double> profilier_accumulator;
	public:
		Profiler();
		static void BeginProfiler();
		static double EndProfiler();	
		~Profiler();
	};


}

