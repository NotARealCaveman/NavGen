#pragma once
#include <vector>


namespace Manifest_Terrain
{
	struct TerrainExtractionData
	{
		/*unrelated*/
	};

	struct TerrainExtraction
	{	
		/*unrelated*/
	};

	//reclaims removed terrain chunks via their deleter
	struct TerrainExtractionDeleter
	{
		void operator()(TerrainExtraction* terrainExtraction)
		{			
			/*removed terrain chunks - will look at terrain allocations later once custom allocator is up and running*/
		}
	};	
}