#pragma once
#include <ManifestCore/Systems/FileSystem.h>
#include <ManifestCore/Systems/WorldSystem.h>
#include <ManifestPersistence/Runtime_Database.h>
#include <ManifestTerrain/Voxel.h>
#include <ManifestTerrain/TerrainFlags.h>

#include "WorldConfiguration.h"

using namespace Manifest_Math;
using namespace Manifest_Terrain;
using namespace Manifest_Persistence;

namespace Manifest_Core
{

	class World
	{
		private:			
			ManifestRuntimeDatabase* runtimeDatabase;
			WorldSystemEventSpace* worldSystemEventSpace;
			MFu32 worldChunks{ 0 };

			const FileSystemObservationToken fileSystemObservationToken{ UnderlyingType(FileSystemMessageType::MBD_VOXELMAP) };			
			FileSystemObserver fileSystemObserver;
		public:		
			World();
			void ObserveWorldMessages();
			WorldConfiguration worldConfigration;

			MFbool Init(ManifestRuntimeDatabase* _runtimeDatabase,WorldSystemEventSpace* _worldSystemEventSpace);
			MFbool RegisterWorldObservers(FileSystemEventSpace& fileSystemEventSpace);
	};		

	void WorldConfigurationFileSystemMessage(std::vector<Message>& messages,ManifestRuntimeDatabase& runtimeDatabase, WorldConfiguration& worldConfiguration, MFu32& worldChunks);
	WorldConfiguration* CreateNewWorldConfiguration(Binary_VoxelMap& binaryVoxelMap, WorldConfiguration& worldConfiguration);
}