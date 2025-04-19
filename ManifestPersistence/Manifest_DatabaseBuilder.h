#pragma once

#include "Binary_Database.h"

namespace Manifest_Persistence
{
	struct ManifestResourceDatabaseBuilder
	{
		GeometryObjectBuildTable geometryObjectBuildTable;
		MeshBuildTable meshBuildTable;
		VertexBuildTables vertexBuildTables;
		IndexBuildTable indexBuildTable;
		GeometryNodeBuildTable geometryNodeBuildTable;
		ObjectRefBuildTable objectRefBuildTable;
		MaterialRefBuildTable materialRefBuildTable;
		MaterialBuildTable materialBuildTable;
		TextureBuildTable textureBuildTable;		
		ColliderBuildTable colliderBuildTable;		
		RigidbodyBuildTable rigidBodyBuildTable;				
	};	
	void ExportBinaryResourceDatabase(const ManifestResourceDatabaseBuilder& databaseBuild, std::ofstream& exportFile);

	struct ManifestWorldDatabaseBuilder
	{
		TerrainBuildTable terrainBuildTable;
		VoxelMapBuildTable voxelMapBuildTable;
	};

	void ExportBinaryTerrainDatabase(const ManifestWorldDatabaseBuilder& databaseBuild, std::ofstream& exportFile);
}
