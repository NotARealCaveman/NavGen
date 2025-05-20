#include "IntelligenceEngine.h"		

using namespace Manifest_Simulation;

MFbool IntelligenceEngine::Init(TerrainExtractionTable* terrainExtractionTable, ActionPlanTable* actionPlanTable)
{
	hivemind.terrainExtractionTable = terrainExtractionTable;
	hivemind.actionPlanTable = actionPlanTable;	
	//sends a COPY of the thread timer to the planner - ai rate must be chosen before initialization currently. may change in the future. 
	aiThread = std::thread(&Hivemind::RunAI, &hivemind, terrainReadIndex, aiThreadTimer);

	return true;
}

void IntelligenceEngine::Terminate()
{	
	hivemind.EndAI();
	aiThread.join();
}

void IntelligenceEngine::SetUpdateRate(const Timer::Duration& updateRate)
{
	aiThreadTimer.SetRunTime(updateRate);
}
 