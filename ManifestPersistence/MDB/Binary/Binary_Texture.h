#pragma once 

#include <ManifestPersistence/MDB/MDB_Texture.h>

namespace Manifest_Persistence
{	
	struct Binary_Texture
	{
		struct Entry_Header
		{
			MFsize payloadSize{ 0 };//stores size of texture data in bytes			
			PrimaryKey textureID;//stored database id of the entry
			MFu32 width;
			MFu32 height;
			MFu32 internalFormat;
			MFu32 dataFormat;
			MFu8 nChannels;
			MFu8 textureType;//type of texture the map represents			
		}header;
		Byte* payload;
	};
	//see Buffer_Types.cpp for details
	size_t Convert_MDB(const MDB_Texture& texture, Binary_Texture& binaryTexture);
}