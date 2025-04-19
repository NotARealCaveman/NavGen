#include "Manifest_DatabaseBuilder.h"

using namespace Manifest_Persistence;


void Manifest_Persistence::ExportBinaryResourceDatabase(const ManifestResourceDatabaseBuilder& databaseBuild, std::ofstream& exportFile)
{
	//TODO: ADD TABLE HEADER FOR QUICK LOOK UP - SEE [] OVERLOAD IN TABLE FOR AN EXAMPLE
	//--		
	//convert mdbs into binary records	
	ManifestBinaryResourceDatabase exportDatabase;
	exportDatabase.binaryMeshTable =  BinaryTableConversion<Binary_Mesh>(databaseBuild.meshBuildTable, databaseBuild.vertexBuildTables, databaseBuild.indexBuildTable);
	exportDatabase.binaryMaterialTable = BinaryTableConversion<Binary_Material>(databaseBuild.materialBuildTable, databaseBuild.textureBuildTable);
	exportDatabase.binaryTextureTable = BinaryTableConversion<Binary_Texture>(databaseBuild.textureBuildTable);
	exportDatabase.binaryGeometryObjectTable = BinaryTableConversion<Binary_GeometryObject>(databaseBuild.geometryObjectBuildTable);
	exportDatabase.binaryGeometryNodeTable = BinaryTableConversion<Binary_GeometryNode>(databaseBuild.geometryNodeBuildTable, databaseBuild.objectRefBuildTable, databaseBuild.materialRefBuildTable);
	//dynamic->static
	exportDatabase.binaryRigidBodyTable = BinaryTableConversion<Binary_RigidBody>(databaseBuild.rigidBodyBuildTable);	
	//dynamic->static
	exportDatabase.binaryColliderTable = BinaryTableConversion<Binary_Collider>(databaseBuild.colliderBuildTable);	
	//export binary tables to file
	//TODO: settle on table ordering as ideally the entire table will be loaded into memory and a file header will be parsed to build the various table entrie payloads
	ExportBinaryTable(exportDatabase.binaryMeshTable, exportFile);
	ExportBinaryTable(exportDatabase.binaryMaterialTable, exportFile);
	ExportBinaryTable(exportDatabase.binaryTextureTable, exportFile);
	ExportBinaryTable(exportDatabase.binaryGeometryObjectTable, exportFile);
	ExportBinaryTable(exportDatabase.binaryGeometryNodeTable, exportFile);
	//dynamic->static
	ExportBinaryTable(exportDatabase.binaryRigidBodyTable, exportFile); 
	//dynamic->static
	ExportBinaryTable(exportDatabase.binaryColliderTable, exportFile); 
}

void Manifest_Persistence::ExportBinaryTerrainDatabase(const ManifestWorldDatabaseBuilder& databaseBuild, std::ofstream& exportFile)
{
	ManifestBinaryWorldDatabase exportDatabase;
	exportDatabase.binaryVoxelMapTable = BinaryTableConversion<Binary_VoxelMap>(databaseBuild.voxelMapBuildTable);
	exportDatabase.binaryTerrainTable = BinaryTableConversion<Binary_Terrain>(databaseBuild.terrainBuildTable);


	ExportBinaryTable(exportDatabase.binaryVoxelMapTable, exportFile);
	ExportBinaryTable(exportDatabase.binaryTerrainTable, exportFile);
}