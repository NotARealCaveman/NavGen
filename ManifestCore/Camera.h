#pragma once

#include <ManifestMath/Vector2.h>
#include <ManifestMath/Projection.h>
#include <ManifestMath/Quaternion.h>
#include <ManifestMath/Polyhedron.h>

#include <ManifestTerrain/TerrainFlags.h>
#include <ManifestUtility/DebugLogger.h>


//#include <glad/glad.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>


using namespace Manifest_Math;
using namespace Manifest_Terrain;

namespace Manifest_Core
{
	//to change	
	constexpr MFu32 CAM_WIDTH = 800;
	constexpr MFu32 CAM_HEIGHT = 600;
	constexpr MFfloat CAM_FOV_Y = 0.872665f;//50* verticle FoV
	constexpr MFfloat CAM_FAR = 500.0f;
	constexpr MFfloat CAM_NEAR = 0.1f;
	constexpr MFu32 numCascades = MAX_TERRAIN_LOD_SUPPORT+1;//number of views camera can be split into
	constexpr MFu32 numCascadeSplits = numCascades + 1;
	//Camera orientation is describe as axis-angle representation
	//theta represents the amount of rotation about the orientation axis
	class Camera
	{
		public:
			Camera();
			Camera(const MFpoint3& _position, const MFquaternion& _orientation);
			void Default();
			void Update(const MFtransform playerTransform);			
			void SplitFrustumPolyhedron();			
			//deprecated to be changed with advanced rendering
			MFmat4 SplitCameraProj(const MFfloat& n, const MFfloat& f) const;
			void SplitCameraClip(MFfloat* splits) const;

			//view&projection
			MFtransform view;//world->view
			MFtransform wsView;//view->world
			MFmat4 projection;//updated only when projection values change, resize, zoom, etc
			MFmat4 viewProjection;
			MFfloat near;//near clip
			MFfloat far;//far clip
			//this needs to be addressed in the future, poor clips can cause a violation to the LOD rule
			MFfloat cascadeClips[numCascadeSplits]{ 0.1,500,1000,1500,2000 };
			std::vector<MFPolyhedron> frustumCascades;
			//rotation
			MFpoint3 position;			
			//projection variables
			MFfloat fovY;//stored in raidans
			MFfloat focalLength;//correlates fovY
			MFu32 width;
			MFu32 height;
			MFbool cameraOverride = false;//debug shit					
	};
	MFtransform LookAt(const MFpoint3& position, const OrthonormalBase& centerAxes);
}