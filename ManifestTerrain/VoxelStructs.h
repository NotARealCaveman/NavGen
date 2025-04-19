#pragma once
#include <vector>
#include <set>
#include <unordered_map>

#include <ManifestMath/Integer3D.h>

using namespace Manifest_Math;

namespace Manifest_Terrain
{    
    //since chunks are already 128x128x128 at any lod no wsY/vsZ transitions will not occur with how maps are to be generated. height shouldn't have a reason to exceed 128
    constexpr MFu8 NUMBER_TRANSITION_FACES = 4;
    
    struct Triangle
    {
        MFu32 vertexIndex[3];
    };

    struct CellStorage
    {
        MFu32 corner[7];
        MFu32 edge[9];
    };
        
    struct VoxelBlock
    {
        //voxel mesh information
        Integer3D minCorner;//ws offset of min block corner     
        Integer3D* meshVertices;//primary mesh vertices in voxel spapce
        Triangle* meshTriangles;//triangle primary vertex information
        MFint32 nVertices{ 0 };//number of primary vertices in voxel mesh
        MFint32 nTriangles{ 0 };//number of primary triangles in voxel mesh
        Integer3D* meshTransitionVertices[NUMBER_TRANSITION_FACES];//transition mesh vertices in voxel space
        Triangle* meshTransitionTriangles[NUMBER_TRANSITION_FACES];//triangle transition vertex information       
        MFint32 nTransitionVertices[NUMBER_TRANSITION_FACES];//number of transition vertices in voxel mesh 
        MFint32 nTransitionTriangles[NUMBER_TRANSITION_FACES];//number of transition triangles in voxel mesh  
        std::vector<MFint32> transitionHalfResolutionIndices[NUMBER_TRANSITION_FACES];//used to map corresponding half resolution vertices with their appropriate offsets                
        std::unordered_map<MFint32, MFu8> faceVertexIndexMap;//non shared vertices 
        std::unordered_map<MFint32, MFu8> sharedFaceVertexIndexMap;
        //if set to false block is unallocated
        bool IS_EXTRACTED_BLOCK{ false };
        ~VoxelBlock();
    };            
    struct VoxelBlockDeleter
    {
       inline void operator()(VoxelBlock* voxelBlock);
    };    

    //TESTING EXTREMAL SHIFTING
    //FOR PRIMARY MESHES LIKE THOSE IN THE VOXEL MESHES EXTREMAL VALUES ARE THE CORNERS,0 AND (WHEN EXTRACTED, 0 BASED) VOXEL_SIZE_OVERRIDE
    //FOR TRANSITION MESHES THEY ARE THE VERTICES WHICH ARE AT THE CORNERS LIKE THE ABOVE BUT ALSO THOSE WHICH ARE ON THE HALF RESOLUTION FACE OF THE TRANSITION MESH
    using ExtremalIndices = std::vector<MFu32>[2];
    
    struct TransitionMeshData
    {
        //set on main thread        
        std::vector<MFvec3> transitionTriangleMesh;//mesh vertices
        std::vector<MFvec3> transitionTriangelNormals;//mesh normals
        std::vector<MFu32> transitionTriangleMeshIndices;//mesh indices
        std::vector<MFu32> halfResolutionVertexIndices;//mesh vertices that are shifted
        ExtremalIndices extremalIndices;
        //set on render thread
        MFu32 VAO;
        MFu32 VBO;
        MFu32 EBO;
        MFu32 indices;
        MFfloat* transitionMeshBuffer;                
    };
    
    struct VoxelMesh
    {
        //set on main thread
        //transition meshs
        TransitionMeshData transitionVertexData[NUMBER_TRANSITION_FACES];
        //primary mesh 
        std::vector<MFu32> secondaryVertexIndices[NUMBER_TRANSITION_FACES];//main face vertices along a block's face
        ExtremalIndices primaryExtremalIndices[NUMBER_TRANSITION_FACES];//corner vertices        
        std::vector<MFvec3> primaryTriangleMesh;//mesh vertices
        std::vector<MFvec3> primaryTriangelNormals;//mesh normals
        std::vector<MFu32>  primaryTriangleMeshIndices;//mesh indices        
        //set on render thread
        MFu32 VAO;
        MFu32 VBO;
        MFu32 EBO;
        MFu32 indices;
        MFfloat* primaryMeshBuffer;                        
    };    
}