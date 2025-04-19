#include "Binary_Database.h"

using namespace Manifest_Persistence;

ManifestBinaryResourceDatabase Manifest_Persistence::ImportGameResources(std::ifstream& resourceFile)
{
	ManifestBinaryResourceDatabase result;

	result.binaryMeshTable = ImportBinaryTable<Binary_Mesh>(resourceFile);
	result.binaryMaterialTable = ImportBinaryTable<Binary_Material>(resourceFile);
	result.binaryTextureTable = ImportBinaryTable<Binary_Texture>(resourceFile);
	result.binaryGeometryObjectTable = ImportBinaryTable<Binary_GeometryObject>(resourceFile);
	result.binaryGeometryNodeTable = ImportBinaryTable<Binary_GeometryNode>(resourceFile);
	//dynamic->static
	result.binaryRigidBodyTable = ImportBinaryTable<Binary_RigidBody>(resourceFile);
	//dynamic->static
	result.binaryColliderTable = ImportBinaryTable<Binary_Collider>(resourceFile);

	return result;
}

ManifestBinaryWorldDatabase Manifest_Persistence::ImportGameTerrain(std::ifstream& terrainFile)
{
	ManifestBinaryWorldDatabase result;

	result.binaryVoxelMapTable = ImportBinaryTable<Binary_VoxelMap>(terrainFile);
	result.binaryTerrainTable = ImportBinaryTable<Binary_Terrain>(terrainFile);

	return result;
}

//move constructors for importing
ManifestBinaryResourceDatabase::ManifestBinaryResourceDatabase(ManifestBinaryResourceDatabase&& other)
	: binaryMeshTable{ std::move(other.binaryMeshTable) },
	binaryMaterialTable{ std::move(other.binaryMaterialTable) },
	binaryTextureTable{ std::move(other.binaryTextureTable) },
	binaryGeometryNodeTable{ std::move(other.binaryGeometryNodeTable) },
	binaryGeometryObjectTable{ std::move(other.binaryGeometryObjectTable) }
{}

ManifestBinaryWorldDatabase::ManifestBinaryWorldDatabase(ManifestBinaryWorldDatabase&& other)
	: binaryTerrainTable{ std::move(other.binaryTerrainTable) },
	binaryVoxelMapTable{ std::move(other.binaryVoxelMapTable) }
{}