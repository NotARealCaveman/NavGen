#pragma once

#include <ManifestPersistence/MDB/Binary/Binary_Types.h>

namespace Manifest_Persistence
{
	struct ManifestBinaryResourceDatabase
	{
		ManifestBinaryResourceDatabase() = default;
		ManifestBinaryResourceDatabase(const ManifestBinaryResourceDatabase&) = delete;
		ManifestBinaryResourceDatabase(ManifestBinaryResourceDatabase&& other);

		BinaryMeshTable binaryMeshTable;
		BinaryMaterialTable binaryMaterialTable;
		BinaryTextureTable binaryTextureTable;
		BinaryGeometryObjectTable binaryGeometryObjectTable;
		BinaryGeometryNodeTable binaryGeometryNodeTable;
		BinaryRigidBodyTable binaryRigidBodyTable;		
		BinaryColliderTable binaryColliderTable;		
	};
	ManifestBinaryResourceDatabase ImportGameResources(std::ifstream& resourceFile);

	struct ManifestBinaryWorldDatabase
	{
		ManifestBinaryWorldDatabase() = default;
		ManifestBinaryWorldDatabase(const ManifestBinaryWorldDatabase&) = delete;
		ManifestBinaryWorldDatabase(ManifestBinaryWorldDatabase&& other);
		
		BinaryVoxelMapTable binaryVoxelMapTable;
		BinaryTerrainTable binaryTerrainTable;		
	};
	ManifestBinaryWorldDatabase ImportGameTerrain(std::ifstream& terrainFile);
}
