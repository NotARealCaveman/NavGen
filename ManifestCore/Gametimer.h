#pragma once
#include <thread>

#include <ManifestUtility/Typenames.h>
#include <ManifestUtility/DebugLogger.h>

#include "Timer.h"


namespace Manifest_Core
{
	class GameTimer 
	{
	private:		
		Timer timer;
		Timer::Duration update;
	public:
		GameTimer();
		GameTimer(const Timer::Duration& _update);
		void Update();
		void Sleep() const;
		void SetRunTime(const Timer::Duration& _update);
		Timer::Duration dt;
	};
}