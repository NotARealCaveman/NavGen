#pragma once
#include "Camera.h"

using namespace Manifest_Math;

namespace Manifest_Core
{
	//represents a structure that holds the split information used for a cascaded depth map
	struct CascadedCameraView
	{
		CascadedCameraView() = default;
		CascadedCameraView(const Camera& cam);

		MFmat4 projSplits[numCascades];//projection per split, per adjust
		MFfloat zSplits[numCascades+1];//cascade split extremities, per adjust
		MFfloat sceneScale[numCascades]; //used in calculating depth bias, per frame
	};	
}