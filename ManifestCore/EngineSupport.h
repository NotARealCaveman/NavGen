#pragma once
#include <memory> 

namespace Manifest_Core
{
	enum class EngineStatus
	{
		RUNNING = 1,
		PAUSED = 0,
		CLOSING = 2
	};	
}