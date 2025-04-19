#pragma once

#include <ManifestParser/OpenGEX/GEX_Color.h>
#include <ManifestParser/OpenGEX/GEX_Texture.h>

#include "MDB_Table.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{
	constexpr MFu64 TEXTURE_INFO_BOW_BITOFFSET{ 61 };
	constexpr MFu64 TEXTURE_DIMENSION_BOW_BITOFFSET{ 32 };

	struct TextureTypes
	{
		static constexpr MFu8 DIFFUSE_TEXTURE{ 0 };
		static constexpr MFu8 NORMAL_TEXTURE{ 1 };
		static constexpr MFu8 PARALLAX_TEXTURE{ 2 };

		static const std::map<std::string, MFu8> textureTypeMap;
	};

	struct ChannelTypes
	{
		static constexpr MFu8 R{ 1 };
		static constexpr MFu8 RGB{ 3 };
		static constexpr MFu8 RGBA{ 4 };
	};
	
	//a MDB_Texture may be created from either a GEX_Color or GEX_Texture
	//the specific channel data will be stored in the channelData member which converts to the payload for the binary type conversion	
	struct MDB_Texture
	{
		PrimaryKey textureID = KEY_NOT_PRESENT;
		//ForeignKey materialID = KEY_NOT_PRESENT;
		CompositeKey textureInfo = KEY_NOT_PRESENT;//represents size(b:0-60) and nChannels(b:61-63)
		CompositeKey textureDimensions;//represents width(b:0-31) and height(b:32-63)
		MFu8 textureType;
		float* channelData = BUFFER_NOT_PRESENT;		
	};

	typedef DatabaseBuildTable<MDB_Texture> TextureBuildTable;
	void TableEntry(const DDL_Structure& structure, ForeignKey(&textureIDs)[3], TextureBuildTable& textureBuildTable);
}