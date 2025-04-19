#pragma once
#include <thread>

#include <ManifestCore/Gametimer.h>

#include "Hivemind.h"

namespace Manifest_Simulation
{
	class IntelligenceEngine
	{
	public:
		MFbool Init(TerrainExtractionTable* terrainExtractionTable, ActionPlanTable* actionPlanTable);
		void Terminate();
		void SetUpdateRate(const Timer::Duration& updateRate);
		MFbool RegisterHivemindToEventSpace(HivemindSystemEventSpace& hivemindEventSpace);				
		std::thread aiThread;		
		MFu32 terrainReadIndex;	
		Hivemind hivemind;
		GameTimer aiThreadTimer;		
	};
}