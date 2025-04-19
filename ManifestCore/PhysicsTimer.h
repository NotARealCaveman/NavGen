#pragma once
#include "Timer.h"

namespace Manifest_Core
{
	class PhysicsTimer
	{
	private:		
		Timer timer;				
	public:
		PhysicsTimer();
		PhysicsTimer(const Timer::Duration& _timeStep);
		void Update();
		Timer::Duration timeStep;//default is ~50fps
	};
}