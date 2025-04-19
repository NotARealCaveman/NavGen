#pragma once

#include <ManifestSimulation/IntelligenceEngine/NavigationMesh.h>

using namespace Manifest_Simulation;
using namespace Manifest_Math;

int main()
{
	MFtransform voxelToWorld;
	MFtriangle triangle;
	NavigationMesh navigationMesh;
	InsertTriangle(triangle, navigationMesh);
	SimplifyMesh(navigationMesh);
	VoxelSpaceToWorldSpace(voxelToWorld, navigationMesh);
	DecomposeMesh(navigationMesh);
}