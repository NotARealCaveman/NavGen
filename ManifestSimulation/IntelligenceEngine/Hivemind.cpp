#include "Hivemind.h"

using namespace Manifest_Simulation;

 
Hivemind::Hivemind() : hivemindSystemObserver{ hivemindSystemObservationToken }
{
	TableStateUpdater::UpdaterBase::blackBoard = &blackBoard;
}

Hivemind::~Hivemind()
{

}


//UPDATE TIMER IS TAKEN AS A COPY FOR NOW TO AVOID RACE CONDITION WHEN TRYING TO UPDATE IT
//IF THE UPDATE TIMER NEEDS TO BE DYNAMIC IN THE FUTURE THIS CAN BE READDRESSED
//TODO:move terrain/navmesh to external init
void Hivemind::RunAI(const MFu32 terrainReadIndex, GameTimer updateTimer)
{	
	hivemindStatus = EngineStatus::RUNNING;	
	//just sleep for a few seconds, let the databases build up
	std::this_thread::sleep_for(Manifest_Core::Timer::Duration{ 5.0s });
	while (hivemindStatus == EngineStatus::RUNNING)
	{
		//GRAB MESSAGES
		updateTimer.Update();		
		terrainExtractionTable->Pull(terrainReadIndex, WorldTerrainProcessingFunction, std::ref(blackBoard));		
		if (hivemindSystemObserver.HasPendingMessages())
			hivemindSystemObserver.ProcessEvents(HivemindSystemMessageProcessingFunction, *this);
		blackBoard.Update();


		using DesireQueue = std::priority_queue<Desire, std::vector<Desire>>;
		std::unordered_map<MFu64, DesireQueue> desireQueues;
		//REPLACE REMOVED DESIRES AND BUILD PRIORITY QUEUE WITH REMAINING 
		std::ranges::for_each(assignedDesires, [this, &desireQueues](Desire& desire)->void
		{
			static const MFu64 SENTINEL_HASH{ DesireHash(SENTINEL_DESIRE) };
			const MFu64 desireHash{ DesireHash(desire) };
			if (desireHash == SENTINEL_HASH)
				return;

			const MFbool satisfied{ ForwardFunction(desire.satisfier, blackBoard, desire.objectiveID, desire.assignedID) };
			const MFbool terminated{ ForwardFunction(desire.termiantor, blackBoard, desire.objectiveID, desire.assignedID) };
			const MFbool removed{ satisfied || terminated };
			if (removed)
			{//TECHNICALLY DOESN'T NEED TO BE ERASED BUT HELPS MEMORY REUSE
				assignedDesires.erase(DesireHash(desire));
				desire = SENTINEL_DESIRE;
			}
			else
				desireQueues[desire.assignedID].push(desire);
		});				
		//APPEND ARCHETYPE UPDATER DESIRES
		MFu32 index{ 0 };
		for (Descriptor objectAttributes : blackBoard.objectAttributes)
		{
			auto key{ blackBoard.objectAttributes.keys.begin() };
			std::advance(key, index);			
			const MFu64 objectID{ key->ID };

			Descriptor attributeMask{ 1 };
			while (objectAttributes)
			{
				const Descriptor maskedAttributes{ objectAttributes & attributeMask };
				if (maskedAttributes == attributeMask)
				{
					const ArchetypeUpdater& updater{ archetypeUpdaters.at(maskedAttributes) };
					const Desire desire{ updater(blackBoard,objectID,objectAttributes) };
					if (desire.representation != NO_DESIRE_SET)
						desireQueues[desire.assignedID].push(desire);
					objectAttributes ^= attributeMask;
				}
				attributeMask <<= 1;
			}
		}

		ActionPlan* actionPlan{ new ActionPlan };
		//for each object, pick the most pressing desire to solve
		for (auto& desireQueue : desireQueues)
		{
			auto& [objectID, queue] { desireQueue };			
			MFbool solved{ false };
			while(!solved && !queue.empty())
			{
				Desire desire{ queue.top() };
				solved = SolveDesire(desire, *actionPlan);
				if (!solved)
					queue.pop();
				else//update assigned desire
					assignedDesires.set(DesireHash(desire), std::move(desire));
			}			
		}
		//push action plan for agents to execute on simulation side		
		actionPlanTable->Push([actionPlan]()->ActionPlan*
		{return actionPlan; });

		updateTimer.Sleep();
	}	
}

MFbool Hivemind::SolveDesire(const Desire& desire, ActionPlan& actionPlan)
{
	const std::vector<ActionDetails>& actions{ droneActions.at(desire.representation) };
	const WorldState desiredState{ ForwardFunction(desire.comparativeValue,blackBoard,desire.objectiveID,desire.assignedID) };

	const FlatMap<Descriptor>& capabilities{ blackBoard.GetObjectCapabilities() };
	const Descriptor droneCapabilities{ capabilities.at(desire.assignedID) };
	for (const ActionDetails& actionDetails : actions)
	{
		//ensure capabilities
		if (std::ranges::find_if(actionDetails.capabilityCodes, [&](const Descriptor capabilityCode)->MFbool
		{
			const Descriptor capabilityMask{ droneCapabilities & capabilityCode };
			return capabilityMask == capabilityCode;
		}) == actionDetails.capabilityCodes.end())
			continue;
		//ensure end state
		if (desiredState != ForwardFunction(actionDetails.endState, blackBoard, desire.comparativeValue, desire.objectiveID, desire.assignedID))
			continue;
		//attempt to create action - may fail requisite actions
		if (CreateAction(actionDetails, desiredState, droneCapabilities, desire.representation, desire.objectiveID, desire.assignedID, actionPlan))
			return true;
	}
	//invalid planning - clear and try again 
	actionPlan[desire.assignedID].clear();
	return false;
}

MFbool Hivemind::CreateAction(const ActionDetails& actionDetails, const WorldState desiredState, const Descriptor droneCapabilities, const Representation representation, const MFu64 objectiveID, const MFu64 assignedDroneID, ActionPlan& actionPlan)
{
	//solve all requisite states first
	for (const RequisiteState& requisiteState : actionDetails.requisiteStates)
	{
		const WorldState requiredState{ ForwardFunction(requisiteState.comparativeValue,blackBoard,objectiveID,assignedDroneID) };
		const WorldState objectState{ blackBoard.GetState(requisiteState.representation,assignedDroneID) };
		//ensure state is not currently satisfied
		if (ForwardFunction(requisiteState.satisfier, blackBoard, objectiveID, assignedDroneID))
			continue;		
		//ensure requisite state can be satisfied
		const std::vector<ActionDetails>& potentialActions{ droneActions.at(requisiteState.representation) };
		const Desire requisiteDesire
		{
			.comparativeValue { requisiteState.comparativeValue},
			.satisfier { requisiteState.satisfier},
			.objectiveID { objectiveID},
			.assignedID { assignedDroneID },
			.representation { requisiteState.representation },
			.priority { 0 }//shouldn't matter for requisite desires
		};
		if (!SolveDesire(requisiteDesire, actionPlan))		
			return false;//not solveable - attempt new plan
	}

	//requisite's are solveable - create action
	DataSystem* goalDataSystem{ goalDataSystems.at(representation) };
	Action droneAction{ ForwardFunction(actionDetails.actionGenerator,blackBoard,desiredState,objectiveID,assignedDroneID,goalDataSystem) };
	actionPlan[assignedDroneID].emplace_back(std::move(droneAction));

	return true;
}

//technically a race condition but fuck it
void Hivemind::EndAI()
{	
	hivemindStatus = EngineStatus::CLOSING;
}


void Hivemind::AddObject(const Descriptor _objectArchetypes, const Descriptor _objectCapabilities, const MFu64 objectID)
{ 
	blackBoard.InsertObject(_objectArchetypes, _objectCapabilities, objectID);
} 

void Hivemind::RemoveObject()
{ 

}

MFbool Hivemind::FindObject()
{
	return false;
}  

MFsize Hivemind::DroneSystemCount()
{
	return hivemindDroneSystems.size();
}

const HivemindDroneSystem& Hivemind::GetDroneSystem(const Descriptor system)
{
	assert(system < hivemindDroneSystems.size());
	return *hivemindDroneSystems.at(system);
} 

void Manifest_Simulation::HivemindSystemMessageProcessingFunction(std::vector<Message>& messages, Hivemind& hivemind)
{
	std::ranges::for_each(messages, [&](Message& message)
	{
		switch (message.messageToken)
		{
			case UnderlyingType(HivemindSystemMessageType::OBJECT_REGISTRATION): 
			{
				ObjectRegistrationMessage objectRegistrationMessage{ message.GetMessageContent<ObjectRegistrationMessage>() };
				if (objectRegistrationMessage.addObject)
					hivemind.AddObject(objectRegistrationMessage.objectAttributes, objectRegistrationMessage.objectCapaibilities, objectRegistrationMessage.objectID);
				else
					hivemind.RemoveObject();
				return;
			} 
			case UnderlyingType(HivemindSystemMessageType::DESIRE_ASSIGNMENT):
			{
				Desire assignedDesire{ message.GetMessageContent<Desire>() };
				const DesireHash desireHash{ assignedDesire };
				//always update an external desire assignment
				hivemind.assignedDesires.set(desireHash, std::move(assignedDesire));
			}
		}
	});
}
 

void Manifest_Simulation::WorldTerrainProcessingFunction(const typename RCU<TerrainExtraction, TerrainExtractionDeleter>::Handle& handle, BlackBoard& blackBoard)
{
	const TerrainExtraction& terrainExtraction{ *handle.handle };

	if (!terrainExtraction.totalExtractedChunks)
		return;
	
	MFsize chunksPerLOD{ terrainExtraction.totalExtractedChunks / (terrainExtraction.maxExtractionLOD + 1) };

	if (blackBoard.navigationMesh.halt)
		return;

	//TODO: DONT REBUILD THIS EVERY ITERATION PLEASE - FFS
	NavigationMesh& navMesh{ blackBoard.navigationMesh };
	navMesh.polygons.clear();
	navMesh.edges.clear();	
	navMesh.freelist.openAddressesBySizeMap.clear();
		
	//return;
	//std::this_thread::sleep_for(Manifest_Core::Timer::Duration{ 1s });
	LOG({ CONSOLE_MAGENTA }, "GENERATING NEW NAV MESH");
	
	const MFu8 TARGET_LOD{ 3 };	
	

	MFtransform voxelToWorld
	{
		WORLD_SCALE,0.0f,0.0f,
		0.0f,0.0f,WORLD_SCALE,
		0.0f,WORLD_SCALE,0.0f,
		0.0f,0.0f,(MFfloat)(terrainExtraction.voxelSpaceZOffset * WORLD_SCALE)
	};
	//reserve space for nav mesh buffers
	ReserveBuffers(navMesh, 16000);
	TRIANGLE_SET::navVisual.lock.Lock();
	ReserveBuffers(TRIANGLE_SET::navVisual.navMesh,16000);	
	TRIANGLE_SET::navVisual.lock.Unlock();
	//triangles are generated in X->Z->Y strips/sqaures/blocks
	//build nav mesh polygons
	NavigationIndex index{ 0 }; 
	//for (auto i{ 0 }; i < chunksPerLOD; ++i)	  
	//for (auto i{ 4 }; i <= 5; ++i)
	for (auto lod = 3; lod >= 3; --lod)
	{
		TRIANGLE_SET::navVisual.navMesh.edges.clear();
		TRIANGLE_SET::navVisual.navMesh.polygons.clear();
		TRIANGLE_SET::navVisual.navMesh.activePolygons.clear();
	TRIANGLE_SET::navVisual.navMesh.freelist.openAddressesBySizeMap.clear();

		for (auto i{ 0 }; i < chunksPerLOD; ++i)
		{
			//const TerrainExtractionData& data{ terrainExtraction.data[(chunksPerLOD * TARGET_LOD) + i] };
			const TerrainExtractionData& data{ terrainExtraction.data[(chunksPerLOD * lod) + i] };

			const PrimaryTerrain& primaryTerrain{ data.primaryTerrain };
			const ExtractionData& extractionData{ primaryTerrain.extractionData };
			for (const Triangle& triangle : extractionData.triangles)
			{

				auto idnex{ std::distance(extractionData.triangles.data(), &triangle) };
				//DLOG({ CONSOLE_BG_BLUE }, "triangle", idnex);


				const Integer3D a{ extractionData.vertices[triangle.vertexIndex[0]] };
				const Integer3D b{ extractionData.vertices[triangle.vertexIndex[1]] };
				const Integer3D c{ extractionData.vertices[triangle.vertexIndex[2]] };

				MFvec3 surfaceNormal{ static_cast<MFvec3>(ValidationNormal(a,b,c, data.extractionCorner)) };
				//calulate 4*Area of triangle
				const MFfloat quadrupleTriangleArea{ MagnitudeSquared(surfaceNormal) };
				if (!quadrupleTriangleArea)
					continue;

				MFtriangle extraction
				{
					static_cast<MFpoint3>(static_cast<const MFvec3>(c)),
					static_cast<MFpoint3>(static_cast<const MFvec3>(b)),
					static_cast<MFpoint3>(static_cast<const MFvec3>(a))
				};


				//assure triangle is walkable 
				const MFplane extractionPlane{ CalculateNormalizedSurfacePlane(extraction) };
				const MFvec3 extractionNormal{ extractionPlane.Normal() };
				if (Dot(extractionNormal, MFvec3{ 0,0,-1 }) < TRAVERSABLE_TOLERANCE)
					continue;
				 
				InsertTriangle(extraction, navMesh);
				TRIANGLE_SET::navVisual.lock.Lock();
				InsertTriangle(extraction, TRIANGLE_SET::navVisual.navMesh);
				TRIANGLE_SET::navVisual.lock.Unlock();
			}
		}
		std::this_thread::sleep_for(Manifest_Core::Timer::Duration({ 5.0s }));
		VoxelSpaceToWorldSpace(voxelToWorld, navMesh);
		SimplifyMesh(navMesh);		
		DecomposeMesh(navMesh);
		GeneratePortals(navMesh);
		LOG({ CONSOLE_MAGENTA }, "NAV MESH GENERATED");
		TRIANGLE_SET::navVisual.lock.Lock();	
		VoxelSpaceToWorldSpace(voxelToWorld,TRIANGLE_SET::navVisual.navMesh);
		SimplifyMesh(TRIANGLE_SET::navVisual.navMesh);		 
		DecomposeMesh(TRIANGLE_SET::navVisual.navMesh);			
		GeneratePortals(TRIANGLE_SET::navVisual.navMesh);		
		TRIANGLE_SET::navVisual.lock.Unlock();

		//std::string extract;
		//std::getline(std::cin, extract);		
	}
	blackBoard.navigationMesh.halt = true;
} 