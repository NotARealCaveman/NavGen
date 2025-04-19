#include "Camera.h"

using namespace Manifest_Core;

Camera::Camera() :
	position({ 0.0f,1.0f,25.0f }), width(CAM_WIDTH), height(CAM_HEIGHT), fovY(CAM_FOV_Y), near(CAM_NEAR), far(CAM_FAR)
{		
	projection = CreateReverseInfiniteCameraProjectionPerspective(focalLength,fovY, width, height, near, 2.6e-10);	
	Default();
	Update(Identity());//set default vectors
}

Camera::Camera(const MFpoint3& _position, const MFquaternion& _orientation) :
	position(_position), width(CAM_WIDTH), height(CAM_HEIGHT), fovY(CAM_FOV_Y), near(CAM_NEAR), far(CAM_FAR)
{		
	//projection = CreateReverseInfiniteCameraProjectionPerspective(focalLength,fovY, width, height, near, 2.6e-10);
	projection = CreateCameraProjectionPerspective(fovY, static_cast<MFfloat>(width), static_cast<MFfloat>(height), near, 10);
	Update(Identity());//set default vectors
}
//sets the default/invariant values for the main camera
void Camera::Default()
{ 
	position = 0;
	far = CAM_FAR;
	near = CAM_NEAR;
	fovY = CAM_FOV_Y;
	width = CAM_WIDTH;
	height = CAM_HEIGHT;
	view[0][3] = 0;
	view[1][3] = 0;
	view[2][3] = 0;
	view[3][3] = 1;
	wsView = Identity();
	frustumCascades.resize(numCascades);


	viewProjection =
	{ 0.328429699, -0.466258436, -2.03443318e-10, -0.782474339,
	0.00000000, 1.06593728, -1.16275531e-10, -0.44721359 ,
	-0.593117833, -0.258183300, -1.12653539e-10, -0.43328285,
	-191.574326, 43.9159470, 0.100000069, 246.995010 };
	position = { 370,89,-100 };
}
 
//a view matrix transform H can be expressed as 
//H[Mr t], such that Mr represents the rotation from the cam orientation to {0.0f,0.0f,-1.0f}
// [0  1], and t is the translation from the cam position to the world origin
//t can be expressed as -t making H [Mr -t]
//takes 3 row vectors left,up & front, and creates the rotation axis to orient the cam with -Z
//H[Rx Ry Rz -Tx] -> 00 10 20 30, takes row vector right and negated position x 
// [Ux Uy Uz -Ty] -> 01 11 21 31, takes row vector up and negated position y
// [Fx Fy Fz -Tz] -> 02 12 22 32, takes row vector front and negated position z
// [0  0  0   1 ] -> 03 13 23 33, transforma is implicitly {0}1 in the 4th row
void Camera::Update(const MFtransform playerTransform)
{		
	const MFpoint3& playerPosition = playerTransform.GetTranslation();
	const MFpoint3 cameraOffset{ 0,10,20 };	
	//position = playerPosition;	
	//const MFvec3 f = Normalize((playerTransform * static_cast<MFvec3>(MFvec3{0,0,-1.0f})));
	position = playerPosition + playerTransform * static_cast<MFvec3>(cameraOffset);
	const MFvec3 f = Normalize((playerTransform * static_cast<MFvec3>(-cameraOffset)));	
	const MFvec3 r = Normalize(Cross(f, MFvec3{ 0,1,0 }));
	const MFvec3 u = Cross(f, r);
	//x rotation
	view[0][0] = r.x;
	view[0][1] = u.x;
	view[0][2] = f.x;
	//y rotation
	view[1][0] = r.y;
	view[1][1] = u.y;
	view[1][2] = f.y;
	//z rotation
	view[2][0] = r.z;
	view[2][1] = u.z;
	view[2][2] = f.z;
	//translation		
	view[3][0] = -Dot(position, r);
	view[3][1] = -Dot(position, u);
	view[3][2] = -Dot(position, f);	
	//combine into VP
	viewProjection = projection * view; 
	SplitFrustumPolyhedron();
}

//builds the current view frustum per cascade
void Camera::SplitFrustumPolyhedron()
{	
	//store view to worldspace transform	
	for (auto cascade = 0; cascade < numCascades; ++cascade)
		frustumCascades[cascade] = BuildFrustumPolyhedron(wsView, view,focalLength, static_cast<MFfloat>(width) / height, cascadeClips[cascade], cascadeClips[cascade + 1]);
}

///DEPRECATED USE ABOVE
//returns a split projection of the main camera projection
//used for cascaded maps
MFmat4 Camera::SplitCameraProj(const MFfloat& n, const MFfloat& f) const
{
	return CreateReverseCameraProjectionPerspective(fovY, width, height, n, f);
}
//captures the clip splits of the current camera
void Camera::SplitCameraClip(MFfloat* splits) const
{
	splits[0] = near;//min view should be camera near
	splits[numCascades - 1] = far;//max likewise
	for (auto i = 1; i < numCascades - 1; ++i)//calculate splits
		splits[i] = ((0.0f + i) / numCascades) * far;
}

MFtransform Manifest_Core::LookAt(const MFpoint3& position, const OrthonormalBase& centerAxes)
{	
	auto r = centerAxes[0];
	auto u = -centerAxes[1];
	auto f = -centerAxes[2];

	//DLOG("F: " << f << " U: " << u <<" P: " << position);
	return MFtransform
	{
		r.x,u.x,f.x,
		r.y,u.y,f.y,
		r.z,u.z,f.z,
		-Dot(position,r),-Dot(position,u),-Dot(position,f)
	};
}