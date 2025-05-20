#pragma once
#include <chrono>

#include <ManifestUtility/Typenames.h>

using namespace Manifest_Utility;
using namespace std::chrono_literals;

namespace Manifest_Core
{
	struct Timer
	{
		using Clock = std::chrono::steady_clock;		
		using Duration = std::chrono::duration<MFdouble>;
		using TimePoint = std::chrono::time_point<Clock, Duration>;
		Timer();
		//takes the time limit in ms
		Timer(const Duration& timeLimit);

		TimePoint start;//current time of update		
		TimePoint stop;//when timed event ends
		Duration elapsed;//integral time from initial start
	};

	//update the current time
	void StepTimer(Timer& timer);
	//if the timer was given a time limit, checks if limit is up
	MFbool TimerStopped(const Timer& timer);
}