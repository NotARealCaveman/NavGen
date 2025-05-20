#pragma once

#include <ManifestMemory/MemoryGuards/ExchangeLock.h>

#include "Action.h"

namespace Manifest_Simulation
{ 

	struct Goal
	{
		const Representation operableRepresenation;//world state the goal will operate on
		const std::vector<ActionDetails> goalActions;//specific actions that can be taken to acomplish this goal		
	};	
}