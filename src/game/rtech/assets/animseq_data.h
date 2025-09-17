#pragma once
#include <game/rtech/cpakfile.h>
#include <game/rtech/utils/utils.h>

struct AnimSeqDataAssetHeader_v1_t
{
	void* data;
};
static_assert(sizeof(AnimSeqDataAssetHeader_v1_t) == 0x8);

class AnimSeqDataAsset
{
public:
	AnimSeqDataAsset(AnimSeqDataAssetHeader_v1_t* hdr) : data(hdr->data)
	{
	
	}

	void* data;
};

void ParseAnimSeqDataForSeqdesc(seqdesc_t* const seqdesc);