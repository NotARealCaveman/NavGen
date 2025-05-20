#pragma once

#include <ManifestSimulation/IntelligenceEngine/Pathing.h>
#include <ManifestSimulation/IntelligenceEngine/Hivemind.h>

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
	MFpoint3 currentLocation; MFpoint3 targetLocation;
	NavigablePolygon const* const currentPolygon{ ClosestPolygonToPoint(navigationMesh, currentLocation) };
	NavigablePolygon const* const targetPolygon{ ClosestPolygonToPoint(navigationMesh, targetLocation) };
	const std::vector<MFpoint3> wayPoints{ GenerateWayPoints(navigationMesh,currentLocation,targetLocation) };
	
	Hivemind hivemind;
}