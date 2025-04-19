#pragma once
#include <ManifestUtility/GLDefinitions.h>

#include "Binary_Table.h"

namespace Manifest_Persistence
{
	using BinaryMeshTable = BinaryTable<Binary_Mesh>;
	using BinaryTextureTable= BinaryTable<Binary_Texture>;
	using BinaryMaterialTable = BinaryTable<Binary_Material>;
	using BinaryGeometryObjectTable = BinaryTable<Binary_GeometryObject>;
	using BinaryGeometryNodeTable = BinaryTable<Binary_GeometryNode>;
	using BinaryRigidBodyTable = BinaryTable<Binary_RigidBody>;
	using BinaryColliderTable = BinaryTable<Binary_Collider>;
	using BinaryTerrainTable =
		BinaryTable<Binary_Terrain>;
	using BinaryVoxelMapTable = BinaryTable<Binary_VoxelMap>;
}