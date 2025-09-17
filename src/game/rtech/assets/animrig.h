#pragma once
#include <game/rtech/cpakfile.h>
#include <game/rtech/utils/utils.h>
#include <game/rtech/assets/model.h>

struct AnimRigAssetHeader_v4_t
{
	void* data; // ptr to studiohdr & rrig buffer

	char* name;

	int unk_10;

	int numAnimSeqs;
	AssetGuid_t* animSeqs;

	int64_t unk_20; // probably reserved
};
static_assert(sizeof(AnimRigAssetHeader_v4_t) == 0x28);

struct AnimRigAssetHeader_v5_t
{
	void* data; // ptr to studiohdr & rrig buffer

	char* name;

	short unk_10;

	short numAnimSeqs;

	int unk_14; // 

	AssetGuid_t* animSeqs;

	int64_t unk_20; // probably reserved
};
static_assert(sizeof(AnimRigAssetHeader_v5_t) == 0x28);

// [rika]: if we use ModelParsedData_t we could reuse model exporting funcs
class AnimRigAsset
{
public:
	AnimRigAsset() = default;
	AnimRigAsset(AnimRigAssetHeader_v4_t* hdr, const eMDLVersion ver) : data(hdr->data), name(hdr->name), numAnimSeqs(hdr->numAnimSeqs), animSeqs(hdr->animSeqs), studioVersion(ver)
	{
		switch (ver)
		{
		case eMDLVersion::VERSION_8:
		{
			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v8_t*>(data));
			break;
		}
		case eMDLVersion::VERSION_12_1:
		{
			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v12_1_t*>(data));
			break;
		}
		case eMDLVersion::VERSION_12_2:
		case eMDLVersion::VERSION_12_3:
		{
			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v12_2_t*>(data));
			break;
		}
		case eMDLVersion::VERSION_12_4:
		case eMDLVersion::VERSION_12_5:
		{
			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v12_4_t*>(data));
			break;
		}
		case eMDLVersion::VERSION_14:
		{
			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v14_t*>(data));
			break;
		}
		}
	}
	AnimRigAsset(AnimRigAssetHeader_v5_t* hdr, const eMDLVersion ver) : data(hdr->data), name(hdr->name), numAnimSeqs(hdr->numAnimSeqs), animSeqs(hdr->animSeqs), studioVersion(ver)
	{
		switch (ver)
		{
		case eMDLVersion::VERSION_16:
		{
			const r5::studiohdr_v16_t* const tmp = reinterpret_cast<const r5::studiohdr_v16_t* const>(data);
			const int studioDataSize = FIX_OFFSET(tmp->boneDataOffset) + (tmp->boneCount * sizeof(r5::mstudiobonedata_v16_t));

			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v16_t*>(data), 0, studioDataSize);
			break;
		}
		case eMDLVersion::VERSION_17:
		case eMDLVersion::VERSION_18:
		{
			const r5::studiohdr_v17_t* const tmp = reinterpret_cast<const r5::studiohdr_v17_t* const>(data);
			const int studioDataSize = FIX_OFFSET(tmp->boneDataOffset) + (tmp->boneCount * sizeof(r5::mstudiobonedata_v16_t));

			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v17_t*>(data), 0, studioDataSize);
			break;
		}
		case eMDLVersion::VERSION_19:
		case eMDLVersion::VERSION_19_1:
		{
			const r5::studiohdr_v17_t* const tmp = reinterpret_cast<const r5::studiohdr_v17_t* const>(data);
			constexpr int dataPerBone = sizeof(int) + sizeof(short) + (sizeof(Vector) * 3) + (sizeof(Quaternion) * 2) + sizeof(matrix3x4_t);

			const int studioDataSize = IALIGN16(FIX_OFFSET(tmp->linearboneindex) + sizeof(r5::mstudiolinearbone_v19_t) + (dataPerBone * tmp->boneCount));

			parsedData = ModelParsedData_t(reinterpret_cast<r5::studiohdr_v17_t*>(data), 0, studioDataSize);
			break;
		}
		}
	}

	void* data; // ptr to studiohdr & rrig buffer
	char* name;

	int numAnimSeqs;
	AssetGuid_t* animSeqs;

	ModelParsedData_t parsedData;

	eMDLVersion studioVersion;

	inline const studiohdr_generic_t& StudioHdr() const { return parsedData.studiohdr; }
	inline const studiohdr_generic_t* const pStudioHdr() const { return &parsedData.studiohdr; }
	inline ModelParsedData_t* const GetParsedData() { return &parsedData; }
	inline const std::vector<ModelBone_t>* const GetRig() const { return &parsedData.bones; } // slerp them bones
};