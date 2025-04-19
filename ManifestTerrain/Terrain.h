#pragma once
#include <shared_mutex>
#include <memory>

#include <ManifestCore/Timer.h>
#include <ManifestCore/Systems/FileSystem.h>	
#include <ManifestCore/Systems/WorldSystem.h>
#include <ManifestPersistence/Runtime_Database.h>
#include <ManifestMemory/Utility/ThreadPool.h>
#include <ManifestMath/Grid2D.h>
#include <ManifestMath/RNG.h>

#include "TerrainExtraction.h"

using namespace Manifest_Core;
using namespace Manifest_Math;
using namespace Manifest_Persistence;
using namespace Manifest_Communication;

namespace Manifest_Terrain
{
	class Terrain
	{
		private:
			ThreadPool* threadPool;
			ManifestRuntimeDatabase* runtimeDatabase;
			//this will be the shared resource which the RTDB will be using. a custom deleter for TerrainExtraction structures is used so that it only cleans up internal memory and these two TerrainExtraction structures will persist the entire runtime
			TerrainExtraction terrainExtraction[2];
			MFu32 terrainGeneration{ DEFAULT_GENERATION };
						
			const WorldSystemObservationToken worldSystemObservationToken{ UnderlyingType(WorldSystemMessageType::NEW_VOXEL_SDF) | UnderlyingType(WorldSystemMessageType::NEW_WORLD_MAP) };
			//unused at the moment
			const FileSystemObservationToken fileSystemObservationToken{ UnderlyingType(FileSystemMessageType::MBD_TERRAIN) };

			FileSystemObserver fileSystemObserver;
			WorldSystemObserver worldSystemObserver;
		public:
			Terrain();
			void ObserveTerrainMessages();

			//initializer functions
			MFbool Init(ThreadPool* _threadPool, ManifestRuntimeDatabase* _runtimeDatabase);
			MFbool RegisterTerrainObservers(FileSystemEventSpace& fileSystemEventSpace, WorldSystemEventSpace& worldSystemEventSpace);
			//world configuration can probably just be moved into the terrain itself. the renderer and simulation can get the information based on the extraction 
			MFu32 worldConfigurationReadIndex;
	};
	//takes in a message describing the update to the world configuration and its affected chunks - returns a list determinig wether a chunk is copied or not
	void WorldUpdate(std::vector<Message>& messages, std::vector<MFu32>& chunkIndexCopyStatus);
	//when signaled, will read the new world config and update voxel blocks that have been altered from the previous SDF
	void ExtractUpdatedSDFNodes(const typename RCU< WorldConfiguration, WorldConfigurationDeleter>::Handle& handle, std::vector<MFu32>& chunkIndexCopyStatus,  MFu32& terrainGeneration, TerrainExtraction(&terrainExtraction)[2]);
	using ExtractionFunction = typename decltype(ExtractUpdatedSDFNodes);
	void ProcessWorldConfiguration(ExtractionFunction& extractionFunction, ManifestRuntimeDatabase& runtimeDatabase, MFu32& worldConfigurationReadIndex, std::vector<MFu32>& chunkIndexCopyStatus, MFu32& terrainGeneration, TerrainExtraction(&terrainExtraction)[2]);
	using ConfigurationFunction = typename decltype(ProcessWorldConfiguration);
	TerrainExtraction* TerrainUpdate(ExtractionFunction& extractionFunction, ConfigurationFunction& configurationFunction, ManifestRuntimeDatabase& runtimeDatabase, MFu32& worldConfigurationReadIndex, std::vector<MFu32>& chunkIndexCopyStatus, MFu32& terrainGeneration, TerrainExtraction(&terrainExtraction)[2]);
	void DONOTHINGANDRETURN(std::vector<Message>& messages);
}