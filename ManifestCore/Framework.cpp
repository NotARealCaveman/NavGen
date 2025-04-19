#include "Framework.h"

using namespace Manifest_Core;

Framework::Framework() : frameworkSystemObserver{ frameworkSystemObservationToken }
{
	DLOG({ CONSOLE_WHITE, CONSOLE_BG_MAGENTA, CONSOLE_BOLD, CONSOLE_BLINK }, "Hello Friend :)");
}

MFbool Framework::RegisterFrameworkObservers()
{
	if (!RegisterObserverToEventSpace(frameworkSystemObserver, frameworkSystemEventSpace))
		return false;	
	if (!RegisterObserverToEventSpace(keyboardHandler.inputSystemObserver, inputSystemEventSpace))
		return false;	
	if (!RegisterObserverToEventSpace(mouseHandler.inputSystemObserver, inputSystemEventSpace))
		return false;

	return true;
}

MFbool Framework::Init()
{	
	//create core to generate data stores and shared objects
	if (!Initialize_Core())
		return false;
	//create comms next to dissimenate messages and events 
	if (!Initialize_Communication())
		return false;
	//create database to interact with shared data
	if (!Initialize_Persistence())
		return false;	
	//init main producer/consumer systems	
	if (!Initialize_World())
		return false;
	if (!Initialize_Terrain())
		return false;
	if (!Initialize_Graphics())
		return false;
	if (!Initialize_Simulation())
		return false;	
	//launch initialized render thread
	renderThread = std::thread(&GameContext::LaunchContext, &gameContext);
	
	return true;
}

void Framework::RegisterKeyboardCallbackFunction(const InputSystemMessageType& keyboardEventType, const InputCode keyCode, KeyboardFunction&& keyboardFunction)
{
	keyboardHandler.RegisterKeyboardFunction(keyboardEventType, keyCode, std::move(keyboardFunction),true);
}

void Framework::RegisterKeyboardPollingFunction(const InputSystemMessageType& keyboardEventType, const InputCode keyCode, KeyboardFunction&& keyboardFunction)
{
	MFu8 actionType{ GetKeyboardAction(keyboardEventType) };
	gameContext.polledKeys.emplace_back(PolledKey{ keyCode,actionType });
	keyboardHandler.RegisterKeyboardFunction(keyboardEventType, keyCode, std::move(keyboardFunction),false);
}

void Framework::RegisterMouseButtonFunction(const InputSystemMessageType& mouseEventType, const InputCode buttonNumber, MouseButtonFunction&& buttonFunction)
{
	mouseHandler.RegisterMouseButtonFunction(mouseEventType, buttonNumber, std::move(buttonFunction));
}

void Framework::SetPlayerEntity(const PrimaryKey playerID)
{ 
	//probably fine - only set once and then read so. you know.
	gameContext.renderer.playerID.store(playerID,std::memory_order_relaxed);
	//to change - race condition - please help us
	GraphicsResources& graphicsResources{ gameContext.graphicsResources };
	GraphicsNode graphicsNode;
	graphicsNode.graphicsNodeID = 0;
	graphicsNode.geoemtryID = 2;
	graphicsNode.materialID = 2;		
	graphicsResources.graphicsNodes.insert(std::make_pair(playerID, graphicsNode));
} 

const MFu32 Framework::CurrentPollingGeneration()
{
	return currentPollingGeneration;
}

void Framework::Shutdown()
{
	renderThread.join();
	simulationEngine.Shutdown();	
	keyboardHandler.Shutdown();
}

void Framework::ProcessMessages()
{		
	keyboardHandler.ObserveInputSystemMessages();
	mouseHandler.ObserveInputSystemMessages();
	ObserveFrameworkMessages();
	world.ObserveWorldMessages();	
	terrain.ObserveTerrainMessages();	
	simulationEngine.ObserveSimulationMessages();	
}

void Framework::ObserveFrameworkMessages()
{
	if (frameworkSystemObserver.HasPendingMessages())
		frameworkSystemObserver.ProcessEvents(FrameworkSystemMessageProcessingFunction,*this);
}

void Manifest_Core::FrameworkSystemMessageProcessingFunction(std::vector<Message>& messages, Framework& framework)
{
	for (Message& message : messages)
	{
		switch (message.messageToken)
		{
			
		}
	}
}

void Framework::Update()
{	
	//MAY MOVE THIS UPPER PORTION TO AN "AWAKE" FUNCTIONS
	gameTimer.Update();
	//get most up to date messages before generating data
	//THIS IS TECHNICALLY A RACE CONDITION - IF SOMETHING GOES HORRIBLY WRONG THE GENERATION CAN BE LOADED AFTER THE POLLING IS DONE BUT BEFORE THE STORE 
	currentPollingGeneration = gameContext.currentPollingGeneration.load(std::memory_order_acquire);	
	ProcessMessages();		

	simulationEngine.accumlatedTime += gameTimer.dt;	
	Timer physicsTimer;
	//pull data for simulation update
	simulationEngine.Pull();
	while (simulationEngine.accumlatedTime >= simulationEngine.GetTimeStep())
	{
		if (engineStatus == EngineStatus::RUNNING)
			simulationEngine.StepSimulation(currentPollingGeneration);
		simulationEngine.accumlatedTime -= simulationEngine.GetTimeStep();
	}
	//push simulation update result
	simulationEngine.Push();

	//begin performance logging
	{	
	StepTimer(physicsTimer);
	StepTimer(printTimer);
	elapsedSim += physicsTimer.elapsed;
	if (printTimer.elapsed > Timer::Duration{ 1 })
	{
		LOG({ CONSOLE_YELLOW }, "Avg Physics Sim: ", elapsedSim / simframe, "frame count:", simframe);
		simframe = 0;
		elapsedSim = Timer::Duration{ 0 };
		printTimer = Timer();
	}
	++simframe;
	//end performance logging
	}	
}
 

void Framework::Sleep()
{
	gameTimer.Sleep();
}

void Framework::SetSimulationLimit(const MFu32& limit)
{	
	Timer::Duration timeStep { 1.0 / limit };
	gameTimer.SetRunTime(timeStep);
	simulationEngine.SetTimeStep(timeStep);	
	//just a test rate for now
	simulationEngine.intelligenceEngine.SetUpdateRate(Timer::Duration{0.25s});
}

const MouseHandler& Framework::GetMouseHandler() const
{
	return mouseHandler;
}

Framework::~Framework()
{	
	DLOG({ CONSOLE_WHITE, CONSOLE_BG_MAGENTA, CONSOLE_BOLD, CONSOLE_BLINK }, "Goodbye Friend!");
} 

void Manifest_Core::UpdateEngineStatus(EngineStatus& currentStatus, const EngineStatus desiredStatus)
{
	DLOG({ CONSOLE_DEFAULT }, &currentStatus);
	currentStatus = desiredStatus;
}