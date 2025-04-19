#include "Framework.h"

using namespace Manifest_Core;

MFbool Framework::Initialize_Core()
{
	//currently just scratch pad - to change with a proper memory management system for run time.
	constexpr MFsize RESERVATION_SIZE{ 512 };
	INIT_MEMORY_RESERVES(512);
	RegisterProgramExecutiveThread();	
	engineStatus = EngineStatus::RUNNING;	 
	//will need to address world grid issue 
	worldGrid = new Grid2D(4, 0, 100, 100, 0);
	return true;
}
MFbool Framework::Initialize_World()
{
	if (!world.Init(&runtimeDatabase,&worldSystemEventSpace))
		return false;

	return true;
}
MFbool Framework::Initialize_Terrain()
{
	if (!terrain.Init(&threadPool, &runtimeDatabase))
		return false;

	return true;
}
MFbool Framework::Initialize_Simulation()
{
	simulationEngine.pool = &threadPool;	
	simulationEngine.graphicsSystemEventSpace = &graphicsSystemEventSpace;
	simulationEngine.hivemindSystemEventSpace = &hivemindSystemEventSpace;
	simulationEngine.runtimeDatabase = &runtimeDatabase;	
	SetSimulationLimit(75);
	
	if (!simulationEngine.Init(HivemindSystem()))
		return false;	


	return true;
}
MFbool Framework::Initialize_Graphics()
{
	return gameContext.Init(144, runtimeDatabase, inputSystemEventSpace, threadPool, engineStatus, const_cast<MouseState&>(mouseHandler.GetMouseState()));
}
MFbool Framework::Initialize_Communication()
{
	if (!RegisterFrameworkObservers())
		return false;
	if (!simulationEngine.RegisterSimulationObservers(fileSystemEventSpace,frameworkSystemEventSpace,hivemindSystemEventSpace))
		return false;
	if (!gameContext.RegisterGraphicsObserver(fileSystemEventSpace, graphicsSystemEventSpace,frameworkSystemEventSpace))
		return false;
	if (!terrain.RegisterTerrainObservers(fileSystemEventSpace, worldSystemEventSpace))
		return false;
	if (!world.RegisterWorldObservers(fileSystemEventSpace))
		return false;	

	return true;
}
MFbool Framework::Initialize_Persistence()
{
	//number of subsytem table readers to be calculated at runtime to integrate with thread pool - requires more matured subsytems/thread pool interface
	constexpr MFu32 maxThreads{ 4 };//don't melt test PCs
	const MFu32 nThreads{ std::thread::hardware_concurrency() > maxThreads ? maxThreads : std::thread::hardware_concurrency() };
	runtimeDatabase.simulationStateTable = new SimulationStateTable{ nThreads };
	runtimeDatabase.terrainExtractionTable = new TerrainExtractionTable{ nThreads };
	runtimeDatabase.worldConfigurationTable = new WorldConfigurationTable{ nThreads }; 
	runtimeDatabase.actionPlanTable= new ActionPlanTable{ nThreads };

	//register thread readers to tables
	//give renderer and hivemind access to state table
	gameContext.renderer.stateReadIndex = runtimeDatabase.simulationStateTable->ReserveTableReadFlag();	
	//give simulation/hivemind & context access to terrain table	
	gameContext.terrainExtractionReadIndex = runtimeDatabase.terrainExtractionTable->ReserveTableReadFlag();
	terrainExtractionReadIndex = runtimeDatabase.terrainExtractionTable->ReserveTableReadFlag();
	simulationEngine.intelligenceEngine.terrainReadIndex = runtimeDatabase.terrainExtractionTable->ReserveTableReadFlag();
	//give terrain access to world confiuration
	terrain.worldConfigurationReadIndex = runtimeDatabase.worldConfigurationTable->ReserveTableReadFlag();
	//give simulation access to action plan table
	simulationEngine.actionPlanReadIndex = runtimeDatabase.actionPlanTable->ReserveTableReadFlag();

	return true;
}
