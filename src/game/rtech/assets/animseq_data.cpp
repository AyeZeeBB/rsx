#include <pch.h>
#include <game/rtech/utils/studio/studio_generic.h>
#include <game/rtech/assets/animseq_data.h>

void LoadAnimSeqDataAsset(CAssetContainer* const container, CAsset* const asset)
{
	UNUSED(container);

	CPakAsset* pakAsset = static_cast<CPakAsset*>(asset);

	AnimSeqDataAsset* animSeqDataAsset = nullptr;

	switch (pakAsset->version())
	{
	case 1:
	{
		AnimSeqDataAssetHeader_v1_t* hdr = reinterpret_cast<AnimSeqDataAssetHeader_v1_t*>(pakAsset->header());
		animSeqDataAsset = new AnimSeqDataAsset(hdr);
		break;
	}
	default:
	{
		assertm(false, "unaccounted asset version, will cause major issues!");
		return;
	}
	}

	pakAsset->setExtraData(animSeqDataAsset);
}

void PostLoadAnimSeqDataAsset(CAssetContainer* const container, CAsset* const asset)
{
	UNUSED(container);

	CPakAsset* pakAsset = static_cast<CPakAsset*>(asset);

	// [rika]: has no name var
	pakAsset->SetAssetNameFromCache();
}

void InitAnimSeqDataAssetType()
{
	AssetTypeBinding_t type =
	{
		.type = 'dqsa',
		.headerAlignment = 8,
		.loadFunc = LoadAnimSeqDataAsset,
		.postLoadFunc = PostLoadAnimSeqDataAsset,
		.previewFunc = nullptr,
		.e = { nullptr, 0, nullptr, 0ull },
	};

	REGISTER_TYPE(type);
}

void ParseAnimSeqDataForSeqdesc(seqdesc_t* const seqdesc)
{
	for (size_t i = 0; i < seqdesc->AnimCount(); i++)
	{
		animdesc_t* const animdesc = &seqdesc->anims.at(i);

		if (!animdesc->animSeqDataGUID && animdesc->baseptr_anim)
			continue;

		CPakAsset* const dataAsset = g_assetData.FindAssetByGUID<CPakAsset>(animdesc->animSeqDataGUID);
		if (!dataAsset)
		{
			/*assertm(false, "animseq data was not loaded!");*/
			animdesc->baseptr_anim = nullptr;
			continue;
		}

		const AnimSeqDataAsset* const animSeqDataAsset = reinterpret_cast<const AnimSeqDataAsset* const>(dataAsset->extraData());

		animdesc->baseptr_anim = animSeqDataAsset->data;
	}
}