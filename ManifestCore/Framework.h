#pragma once
#include <ManifestSimulation/SimulationEngine.h>
#include <ManifestGraphics/GameContext.h>
#include <ManifestMemory/Manifest_Allocator.h>
#include <ManifestMemory/Allocators/GlobalAllocator.h>
#include <ManifestMemory/Utility/ThreadPool.h>
#include <ManifestUtility/Profiler.h>
#include <ManifestPersistence/Runtime_Database.h>
#include <ManifestTerrain/Terrain.h>

#include "EngineSupport.h"
#include "Config.h"
#include "Gametimer.h"
#include "Camera.h"
#include "DepthStructures.h"
#include "World.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "Systems/FrameworkSystem.h"

#include <ManifestAnimation/Animation.h>

using namespace Manifest_Animation;
using namespace Manifest_Graphics;
using namespace Manifest_Memory;
using namespace Manifest_Utility;


namespace Manifest_Core
{	
	static constexpr MFu32 MAX_OBJECTS = 10024;

	class Framework
	{
	public:
		Framework();
		~Framework();		
		MFbool Init();
		void RegisterKeyboardCallbackFunction(const InputSystemMessageType& keyboardEventType, const InputCode keyCode, KeyboardFunction&& keyboardFunction);
		void RegisterKeyboardPollingFunction(const InputSystemMessageType& keyboardEventType, const InputCode keyCode, KeyboardFunction&& keyboardFunction);
		void RegisterMouseButtonFunction(const InputSystemMessageType& mouseEventType, const InputCode buttonNumber, MouseButtonFunction&& buttonFunction);
		void SetPlayerEntity(const PrimaryKey playerID);
		const MFu32 CurrentPollingGeneration();
		void ProcessMessages();		
		void Update();		
		void Sleep();
		void Shutdown();				
		//Helper Functions
		void SetSimulationLimit(const MFu32& limit);
		const MouseHandler& GetMouseHandler() const;

		inline FileSystemEventSpace& FileSystem() { return fileSystemEventSpace; }
		inline GraphicsSystemEventSpace& GraphicsSystem() { return graphicsSystemEventSpace; }
		inline InputSystemEventSpace& InputSystem() { return inputSystemEventSpace; }
		inline FrameworkSystemEventSpace& FrameworkSystem(){ return frameworkSystemEventSpace; }
		inline HivemindSystemEventSpace& HivemindSystem() { return hivemindSystemEventSpace; }

		//CURRENT
		ManifestRuntimeDatabase runtimeDatabase;//shared memory nexus			
		SimulationEngine simulationEngine;//handles simulation and produces renderable state		

		//TO BE UPDATED
		EngineStatus engineStatus;//current running state of engine(to be overhauled)		
		World world;//game world
		Terrain terrain;//manages and updates terrain chunks	
		GameContext gameContext;//context for rendering		
		//testing terrain updates
		std::thread terrainUpdateThread;				
	private:
		//defined in initializer.cpp
		MFbool Initialize_Core();
		MFbool Initialize_Communication();
		MFbool Initialize_Persistence();
		MFbool Initialize_World();
		MFbool Initialize_Terrain();
		MFbool Initialize_Simulation();
		MFbool Initialize_Graphics();	
		MFbool RegisterFrameworkObservers();
		void ObserveFrameworkMessages();

		//handles input callback and polling events
		KeyboardHandler keyboardHandler;
		MouseHandler mouseHandler;
		//controls main thread sleeping and running time 
		GameTimer gameTimer;
		Grid2D* worldGrid;
		ThreadPool threadPool;
		std::thread renderThread;		
		//used by simulation engine to gauge time from perspective of renderer
		MFu32 currentPollingGeneration{ 0 };
		//system event spaces
		FileSystemEventSpace fileSystemEventSpace;
		GraphicsSystemEventSpace graphicsSystemEventSpace;
		WorldSystemEventSpace worldSystemEventSpace;
		InputSystemEventSpace inputSystemEventSpace;
		FrameworkSystemEventSpace frameworkSystemEventSpace;
		HivemindSystemEventSpace hivemindSystemEventSpace;
		//handles dispatching attachments
		FrameworkSystemObserver frameworkSystemObserver;
		static constexpr FrameworkSystemObservationToken frameworkSystemObservationToken
		{	

		};
		//sim and game will run on the main thread - have game register to read spaces and pass read indices along
		MFu32 terrainExtractionReadIndex;

		//DEBUG STUFF
		Timer printTimer;
		Timer::Duration elapsedSim{ 0 };
		MFu32 simframe;				
	};
	extern inline void UpdateEngineStatus(EngineStatus& currentStatus, const EngineStatus desiredStatus);
	void FrameworkSystemMessageProcessingFunction(std::vector<Message>& messages, Framework& framework);
}

