#pragma once
#include <memory>

#include <ManifestGraphics/Mesh.h>

namespace Manifest_Core
{
	enum class EngineStatus
	{
		RUNNING = 1,
		PAUSED = 0,
		CLOSING = 2
	};	
}