#pragma once
#include <game/rtech/utils/studio/studio_generic.h>

#include <game/rtech/cpakfile.h>
#include <game/rtech/assets/texture.h>
#include <game/rtech/assets/material.h>

//
// File contains data for exporting and storing 3D assets
//

// pre def structs
struct ModelMeshData_t;
class ModelParsedData_t;

struct VertexWeight_t
{
	float weight;
	int16_t bone;
};

struct Vertex_t
{
	Vector position;
	Normal32 normalPacked;
	Color32 color;
	Vector2D texcoord; // texcoord0

	// [rika]: if this vertex buffer is used for rendering our shader doesn't support bones anyway.
	uint32_t weightCount : 8;
	uint32_t weightIndex : 24; // max weight count in a mesh is 1048576 (2^20), 24 bits gives plenty of headroom with a max value of 16777216 (2^24)

	static void ParseVertexFromVG(Vertex_t* const vert, VertexWeight_t* const weights, Vector2D* const texcoords, ModelMeshData_t* const mesh, const char* const rawVertexData, const uint8_t* const boneMap, const vvw::mstudioboneweightextra_t* const weightExtra, int& weightIdx);

	// Generic (basic data shared between them)
	static void ParseVertexFromVTX(Vertex_t* const vert, Vector2D* const texcoords, ModelMeshData_t* const mesh, const vvd::mstudiovertex_t* const pVerts, const Vector4D* const pTangs, const Color32* const pColors, const Vector2D* const pUVs, const int origId);

	// Basic Source
	static void ParseVertexFromVTX(Vertex_t* const vert, VertexWeight_t* const weights, Vector2D* const texcoords, ModelMeshData_t* mesh, const OptimizedModel::Vertex_t* const pVertex, const vvd::mstudiovertex_t* const pVerts, const Vector4D* const pTangs, const Color32* const pColors, const Vector2D* const pUVs,
		int& weightIdx, const bool isHwSkinned, const OptimizedModel::BoneStateChangeHeader_t* const pBoneStates);

	// Apex Legends
	static void ParseVertexFromVTX(Vertex_t* const vert, VertexWeight_t* const weights, Vector2D* const texcoords, ModelMeshData_t* mesh, const OptimizedModel::Vertex_t* const pVertex, const vvd::mstudiovertex_t* const pVerts, const Vector4D* const pTangs, const Color32* const pColors, const Vector2D* const pUVs,
		const vvw::vertexBoneWeightsExtraFileHeader_t* const pVVW, int& weightIdx);

	inline static void InvertTexcoord(Vector2D& texcoord)
	{
		texcoord.y = 1.0f - texcoord.y;
	}
};

//
// PARSEDDATA
//
struct ModelMeshData_t
{
	ModelMeshData_t() : meshVertexDataIndex(invalidNoodleIdx), rawVertexData(nullptr), rawVertexLayoutFlags(0ull), indexCount(0), vertCount(0), vertCacheSize(0), weightsPerVert(0), weightsCount(0), texcoordCount(0), texcoodIndices(0), materialId(0), materialAsset(nullptr), bodyPartIndex(-1) {};
	ModelMeshData_t(const ModelMeshData_t& mesh) : meshVertexDataIndex(mesh.meshVertexDataIndex), rawVertexData(mesh.rawVertexData), rawVertexLayoutFlags(mesh.rawVertexLayoutFlags), indexCount(mesh.indexCount), vertCount(mesh.vertCount), vertCacheSize(mesh.vertCacheSize),
		weightsPerVert(mesh.weightsPerVert), weightsCount(mesh.weightsCount), texcoordCount(mesh.texcoordCount), texcoodIndices(mesh.texcoodIndices), materialId(mesh.materialId), materialAsset(mesh.materialAsset), bodyPartIndex(mesh.bodyPartIndex) {
	};
	ModelMeshData_t(ModelMeshData_t& mesh) : meshVertexDataIndex(mesh.meshVertexDataIndex), rawVertexData(mesh.rawVertexData), rawVertexLayoutFlags(mesh.rawVertexLayoutFlags), indexCount(mesh.indexCount), vertCount(mesh.vertCount), vertCacheSize(mesh.vertCacheSize),
		weightsPerVert(mesh.weightsPerVert), weightsCount(mesh.weightsCount), texcoordCount(mesh.texcoordCount), texcoodIndices(mesh.texcoodIndices), materialId(mesh.materialId), materialAsset(mesh.materialAsset), bodyPartIndex(mesh.bodyPartIndex) {
	};

	~ModelMeshData_t() {}

	size_t meshVertexDataIndex;

	char* rawVertexData; // for model preview only
	uint64_t rawVertexLayoutFlags; // the flags from a 'vg' (baked hwData) mesh, identical to 'Vertex Layout Flags' (DX)

	uint32_t indexCount;

	uint32_t vertCount;
	uint16_t vertCacheSize;

	uint16_t weightsPerVert; // max number of weights per vertex
	uint32_t weightsCount; // the total number of weights in this mesh

	int16_t texcoordCount;
	int16_t texcoodIndices; // texcoord indices, e.g. texcoord0, texcoord2, etc

	// [rika]: swapped this to CPakAsset because in many cases the parsed asset would not exist yet
	int materialId; // the index of this material
	CPakAsset* materialAsset; // pointer to the material's asset (if loaded)

	int bodyPartIndex;

	// gets the texcoord and indice based off rawVertexLayoutFlags
	void ParseTexcoords();
	void ParseMaterial(ModelParsedData_t* const parsed, const int material);

	inline MaterialAsset* const GetMaterialAsset() const { return reinterpret_cast<MaterialAsset* const>(materialAsset->extraData()); }
};

struct ModelModelData_t
{
	ModelModelData_t() : meshes(nullptr), meshIndex(0), meshCount(0), vertCount(0) {}

	std::string name;
	ModelMeshData_t* meshes;
	size_t meshIndex;
	uint32_t meshCount;

	uint32_t vertCount; // used to determine if this model is disabled in a LOD, as traditional methods (via VTX) will not work for most apex models
};

struct ModelLODData_t
{
	std::vector<ModelModelData_t> models;
	std::vector<ModelMeshData_t> meshes;
	size_t vertexCount;
	size_t indexCount;
	float switchPoint;

	// for exporting
	uint16_t texcoordsPerVert; // max texcoords used in any mesh from this lod
	uint16_t weightsPerVert; // max weights used in any mesh from this lod

	inline const int GetMeshCount() const { return static_cast<int>(meshes.size()); }
	inline const int GetModelCount() const { return static_cast<int>(models.size()); }

	inline const ModelModelData_t* const pModel(const size_t i) const { return &models.at(i); }
};

struct ModelBone_t
{
	ModelBone_t() = default;

	ModelBone_t(const r1::mstudiobone_t* const bone) : baseptr(reinterpret_cast<const char* const>(bone)), name(bone->pszName()), parent(bone->parent), flags(bone->flags), proctype(bone->proctype), procindex(bone->procindex), physicsbone(bone->physicsbone), surfacepropidx(bone->surfacepropidx), contents(bone->contents),
		poseToBone(&bone->poseToBone), pos(bone->pos), quat(bone->quat), rot(bone->rot), scale(bone->scale) {};

	ModelBone_t(const r2::mstudiobone_t* const bone) : baseptr(reinterpret_cast<const char* const>(bone)), name(bone->pszName()), parent(bone->parent), flags(bone->flags), proctype(bone->proctype), procindex(bone->procindex), physicsbone(bone->physicsbone), surfacepropidx(bone->surfacepropidx), contents(bone->contents),
		poseToBone(&bone->poseToBone), pos(bone->pos), quat(bone->quat), rot(bone->rot), scale(bone->scale) {};

	ModelBone_t(const r5::mstudiobone_v8_t* const bone) : baseptr(reinterpret_cast<const char* const>(bone)), name(bone->pszName()), parent(bone->parent), flags(bone->flags), proctype(bone->proctype), procindex(bone->procindex), physicsbone(bone->physicsbone), surfacepropidx(bone->surfacepropidx), contents(bone->contents),
		poseToBone(&bone->poseToBone), pos(bone->pos), quat(bone->quat), rot(bone->rot), scale(bone->scale) {};

	ModelBone_t(const r5::mstudiobone_v12_1_t* const bone) : baseptr(reinterpret_cast<const char* const>(bone)), name(bone->pszName()), parent(bone->parent), flags(bone->flags), proctype(bone->proctype), procindex(bone->procindex), physicsbone(bone->physicsbone), surfacepropidx(bone->surfacepropidx), contents(bone->contents),
		poseToBone(&bone->poseToBone), pos(bone->pos), quat(bone->quat), rot(bone->rot), scale(bone->scale) {};

	ModelBone_t(const r5::mstudiobonehdr_v16_t* const bonehdr, const r5::mstudiobonedata_v16_t* const bonedata) : baseptr(reinterpret_cast<const char* const>(bonehdr)), name(bonehdr->pszName()), parent(bonedata->parent), flags(bonedata->flags), proctype(bonedata->proctype), procindex(bonedata->procindex),
		physicsbone(bonehdr->physicsbone), surfacepropidx(bonehdr->surfacepropidx), contents(bonehdr->contents),
		poseToBone(&bonedata->poseToBone), pos(bonedata->pos), quat(bonedata->quat), rot(bonedata->rot), scale(bonedata->scale)
	{
		const int64_t tmpOffset = reinterpret_cast<const char* const>(bonedata) - reinterpret_cast<const char* const>(bonehdr);
		procindex += static_cast<int>(tmpOffset); // adjust procindex to be based off the bone header
	};

	ModelBone_t(const r5::mstudiobonehdr_v16_t* const bonehdr, const r5::mstudiobonedata_v19_t* const bonedata, const r5::mstudiolinearbone_v19_t* const linearbone, const int bone) : baseptr(reinterpret_cast<const char* const>(bonehdr)), name(bonehdr->pszName()), parent(bonedata->parent), flags(bonedata->flags), proctype(bonedata->proctype), procindex(bonedata->procindex),
		physicsbone(bonehdr->physicsbone), surfacepropidx(bonehdr->surfacepropidx), contents(bonehdr->contents),
		poseToBone(linearbone->pPoseToBone(bone)), pos(*linearbone->pPos(bone)), quat(*linearbone->pQuat(bone)), rot(*linearbone->pRot(bone)), scale(*linearbone->pScale(bone))
	{
		const int64_t tmpOffset = reinterpret_cast<const char* const>(bonedata) - reinterpret_cast<const char* const>(bonehdr);
		procindex += static_cast<int>(tmpOffset); // adjust procindex to be based off the bone header
	};

	const char* baseptr;

	const char* name;
	inline const char* const pszName() const { return name; }

	int parent;

	int flags;
	int proctype;
	int procindex; // procedural rule offset
	int physicsbone; // index into physically simulated bone
	inline const void* const pProcedure() const { return procindex ? reinterpret_cast<const void* const>(baseptr + procindex) : nullptr; };

	int surfacepropidx; // index into string tablefor property name
	inline const char* const pszSurfaceProp() const { return baseptr + surfacepropidx; }

	int contents; // See BSPFlags.h for the contents flags

	const matrix3x4_t* poseToBone; // use a pointer for this type since it's very large

	const Vector pos;
	const Quaternion quat;
	const RadianEuler rot;
	const Vector scale;

	ModelBone_t& operator=(const ModelBone_t& bone)
	{
		memcpy_s(this, sizeof(ModelBone_t), &bone, sizeof(ModelBone_t));

		return *this;
	}
};

struct ModelAttachment_t
{
	ModelAttachment_t() = default;
	ModelAttachment_t(const mstudioattachment_t* const attachment) : name(attachment->pszName()), flags(attachment->flags), localbone(attachment->localbone), localmatrix(&attachment->local) {}
	ModelAttachment_t(const r5::mstudioattachment_v8_t* const attachment) : name(attachment->pszName()), flags(attachment->flags), localbone(attachment->localbone), localmatrix(&attachment->local) {}
	ModelAttachment_t(const r5::mstudioattachment_v16_t* const attachment) : name(attachment->pszName()), flags(attachment->flags), localbone(attachment->localbone), localmatrix(&attachment->local) {}

	const char* name;
	int flags;

	int localbone;
	const matrix3x4_t* localmatrix;
};

struct ModelHitbox_t
{
	ModelHitbox_t() = default;
	ModelHitbox_t(const mstudiobbox_t* const bbox) : bone(bbox->bone), group(bbox->group), bbmin(&bbox->bbmin), bbmax(&bbox->bbmax), name(bbox->pszHitboxName()), forceCritPoint(0) {}
	ModelHitbox_t(const r2::mstudiobbox_t* const bbox) : bone(bbox->bone), group(bbox->group), bbmin(&bbox->bbmin), bbmax(&bbox->bbmax), name(bbox->pszHitboxName()), forceCritPoint(bbox->forceCritPoint) {}
	ModelHitbox_t(const r5::mstudiobbox_v8_t* const bbox) : bone(bbox->bone), group(bbox->group), bbmin(&bbox->bbmin), bbmax(&bbox->bbmax), name(bbox->pszHitboxName()), forceCritPoint(bbox->forceCritPoint) {}
	ModelHitbox_t(const r5::mstudiobbox_v16_t* const bbox) : bone(bbox->bone), group(bbox->group), bbmin(&bbox->bbmin), bbmax(&bbox->bbmax), name(bbox->pszHitboxName()), forceCritPoint(0) {}

	int bone;
	int group;
	const Vector* bbmin;
	const Vector* bbmax;
	const char* name;

	int forceCritPoint;
};

struct ModelHitboxSet_t
{
	ModelHitboxSet_t(const mstudiohitboxset_t* const hitboxset, const mstudiobbox_t* const bboxes) : name(hitboxset->pszName()), hitboxes(nullptr), numHitboxes(hitboxset->numhitboxes)
	{
		if (!numHitboxes)
			return;

		hitboxes = new ModelHitbox_t[numHitboxes]{};

		for (int i = 0; i < numHitboxes; i++)
		{
			hitboxes[i] = ModelHitbox_t(bboxes + i);
		}
	};
	ModelHitboxSet_t(const mstudiohitboxset_t* const hitboxset, const r2::mstudiobbox_t* const bboxes) : name(hitboxset->pszName()), hitboxes(nullptr), numHitboxes(hitboxset->numhitboxes)
	{
		if (!numHitboxes)
			return;

		hitboxes = new ModelHitbox_t[numHitboxes]{};

		for (int i = 0; i < numHitboxes; i++)
		{
			hitboxes[i] = ModelHitbox_t(bboxes + i);
		}
	};
	ModelHitboxSet_t(const mstudiohitboxset_t* const hitboxset, const r5::mstudiobbox_v8_t* const bboxes) : name(hitboxset->pszName()), hitboxes(nullptr), numHitboxes(hitboxset->numhitboxes)
	{
		if (!numHitboxes)
			return;

		hitboxes = new ModelHitbox_t[numHitboxes]{};

		for (int i = 0; i < numHitboxes; i++)
		{
			hitboxes[i] = ModelHitbox_t(bboxes + i);
		}
	};
	ModelHitboxSet_t(const r5::mstudiohitboxset_v16_t* const hitboxset) : name(hitboxset->pszName()), hitboxes(nullptr), numHitboxes(hitboxset->numhitboxes)
	{
		if (!numHitboxes)
			return;

		hitboxes = new ModelHitbox_t[numHitboxes]{};

		for (int i = 0; i < numHitboxes; i++)
		{
			hitboxes[i] = ModelHitbox_t(hitboxset->pHitbox(i));
		}
	};

	~ModelHitboxSet_t()
	{
		FreeAllocArray(hitboxes);
	}

	const char* name;
	ModelHitbox_t* hitboxes;
	int numHitboxes;
};

struct ModelMaterialData_t
{
	~ModelMaterialData_t()
	{
		FreeAllocArray(stored);
	}

	// pointer to the referenced material asset
	CPakAsset* asset;

	// also store guid and name just in case the material is not loaded
	uint64_t guid;
	const char* name;
	const char* stored;

	inline MaterialAsset* const GetMaterialAsset() const { return asset ? reinterpret_cast<MaterialAsset* const>(asset->extraData()) : nullptr; }

	// use 'true' to prefer studio model name, use 'false' for material asset name
	const char* const GetName(const bool biasStudio) const
	{
		// [rika]: if we don't prefer studio name, or studio name is generated, return material name if it exists
		if ((!biasStudio || IsAllocated()) && GetMaterialAsset())
			return GetMaterialAsset()->name;

		// [rika]: prefer studio name, or return generated name if the material did not exist
		return name;
	}

	// [rika]: originally had this as one function but it's wasted performance to check in cases it'd never happen
	inline void SetName(const char* const str)
	{
		name = str;
		stored = nullptr;
	}

	inline void StoreName(const char* const str)
	{
		const size_t length = strnlen_s(str, MAX_PATH) + 1;
		char* buf = new char[length] {};
		strncpy_s(buf, length, str, length - 1);

		stored = buf;
		name = stored;
	}

	inline const bool IsAllocated() const { return stored ? true : false; }
};

struct ModelSkinData_t
{
	ModelSkinData_t(const char* nameIn, const int16_t* indiceIn) : name(nameIn), indices(indiceIn) {};

	const char* name;
	const int16_t* indices;
};

struct ModelBodyPart_t
{
	ModelBodyPart_t() : partName(), modelIndex(-1), numModels(0), previewEnabled(true) {};

	std::string partName; // c string?

	int modelIndex;
	int numModels;

	bool previewEnabled;

	FORCEINLINE void SetName(const std::string& name) { partName = name; };
	FORCEINLINE std::string GetName() const { return partName; };
	FORCEINLINE const char* GetNameCStr() const { return partName.c_str(); };
	FORCEINLINE bool IsPreviewEnabled() const { return previewEnabled; };
};

struct ModelAnimSequence_t
{
	enum class eType : uint8_t
	{
		UNLOADED = 0,
		DIRECT,
		RIG,
	};

	CPakAsset* asset;
	uint64_t guid;
	eType type;

	FORCEINLINE const bool IsLoaded() const { return type != eType::UNLOADED; };
};

struct ModelPoseParam_t
{
	ModelPoseParam_t() : name(nullptr), flags(0), start(0.0f), end(0.0f), loop(0.0f) {}
	ModelPoseParam_t(const mstudioposeparamdesc_t* const poseparam) : name(poseparam->pszName()), flags(poseparam->flags), start(poseparam->start), end(poseparam->end), loop(poseparam->loop) {}
	ModelPoseParam_t(const r5::mstudioposeparamdesc_v16_t* const poseparam) : name(poseparam->pszName()), flags(poseparam->flags), start(poseparam->start), end(poseparam->end), loop(poseparam->loop) {}

	const char* name;

	int flags;
	float start;
	float end;
	float loop;
};

struct ModelIKLock_t
{
	ModelIKLock_t() = default;
	ModelIKLock_t(const mstudioiklock_t* const iklock) : chain(iklock->chain), flPosWeight(iklock->flPosWeight), flLocalQWeight(iklock->flLocalQWeight), flags(iklock->flags) {}
	ModelIKLock_t(const r5::mstudioiklock_v8_t* const iklock) : chain(iklock->chain), flPosWeight(iklock->flPosWeight), flLocalQWeight(iklock->flLocalQWeight), flags(iklock->flags) {}
	ModelIKLock_t(const r5::mstudioiklock_v16_t* const iklock) : chain(iklock->chain), flPosWeight(iklock->flPosWeight), flLocalQWeight(iklock->flLocalQWeight), flags(iklock->flags) {}

	int chain;
	float flPosWeight;
	float flLocalQWeight;
	int flags;
};

struct ModelIKLink_t
{
	ModelIKLink_t() = default;
	ModelIKLink_t(const mstudioiklink_t* const iklink) : bone(iklink->bone), kneeDir(iklink->kneeDir) {}
	ModelIKLink_t(const r5::mstudioiklink_v8_t* const iklink) : bone(iklink->bone), kneeDir(iklink->kneeDir) {}
	ModelIKLink_t(const r5::mstudioiklink_v16_t* const iklink) : bone(iklink->bone), kneeDir(iklink->kneeDir) {}

	int bone;
	Vector kneeDir;
};

struct ModelIKChain_t
{
	ModelIKChain_t() = default;
	ModelIKChain_t(const mstudioikchain_t* const ikchain) : name(ikchain->pszName()), unk_10(0.0f)
	{
		assertm(ikchain->numlinks == 3, "ikchain was abnormal");
		assertm(ikchain->linktype == 0, "ikchain was abnormal");

		links[0] = ModelIKLink_t(ikchain->pLink(0));
		links[1] = ModelIKLink_t(ikchain->pLink(1));
		links[2] = ModelIKLink_t(ikchain->pLink(2));
	}
	ModelIKChain_t(const r2::mstudioikchain_t* const ikchain) : name(ikchain->pszName()), unk_10(ikchain->unk_10)
	{
		assertm(ikchain->numlinks == 3, "ikchain was abnormal");
		assertm(ikchain->linktype == 0, "ikchain was abnormal");

		links[0] = ModelIKLink_t(ikchain->pLink(0));
		links[1] = ModelIKLink_t(ikchain->pLink(1));
		links[2] = ModelIKLink_t(ikchain->pLink(2));
	}
	ModelIKChain_t(const r5::mstudioikchain_v8_t* const ikchain) : name(ikchain->pszName()), unk_10(ikchain->unk_10)
	{
		assertm(ikchain->numlinks == 3, "ikchain was abnormal");
		assertm(ikchain->linktype == 0, "ikchain was abnormal");

		links[0] = ModelIKLink_t(ikchain->pLink(0));
		links[1] = ModelIKLink_t(ikchain->pLink(1));
		links[2] = ModelIKLink_t(ikchain->pLink(2));
	}
	ModelIKChain_t(const r5::mstudioikchain_v16_t* const ikchain) : name(ikchain->pszName()), unk_10(ikchain->unk_10)
	{
		assertm(ikchain->numlinks == 3, "ikchain was abnormal");
		assertm(ikchain->linktype == 0, "ikchain was abnormal");

		links[0] = ModelIKLink_t(ikchain->pLink(0));
		links[1] = ModelIKLink_t(ikchain->pLink(1));
		links[2] = ModelIKLink_t(ikchain->pLink(2));
	}

	enum LinkType_t
	{
		IKLINK_THIGH,
		IKLINK_KNEE,
		IKLINK_FOOT,

		IKLINK_COUNT
	};

	const char* name;
	float unk_10;

	uint32_t pad;

	// while this could be dynamic, it's hardcoded throughout all of source and reSource to assume it's 3
	// I'd imagine the original intent was to add more types with varied links, but that never happened
	ModelIKLink_t links[IKLINK_COUNT];
};


class ModelParsedData_t
{
public:
	ModelParsedData_t() = default;
	ModelParsedData_t(r1::studiohdr_t* const hdr, StudioLooseData_t* const data) : sequences(nullptr), ikchains(nullptr), iklocks(nullptr), poseparams(nullptr), studiohdr(hdr, data) {};
	ModelParsedData_t(r2::studiohdr_t* const hdr) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr) {};
	ModelParsedData_t(r5::studiohdr_v8_t* const hdr) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr) {};
	ModelParsedData_t(r5::studiohdr_v12_1_t* const hdr) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr) {};
	ModelParsedData_t(r5::studiohdr_v12_2_t* const hdr) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr) {};
	ModelParsedData_t(r5::studiohdr_v12_4_t* const hdr) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr) {};
	ModelParsedData_t(r5::studiohdr_v14_t* const hdr) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr) {};
	ModelParsedData_t(r5::studiohdr_v16_t* const hdr, const int dataSizePhys, const int dataSizeModel) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr, dataSizePhys, dataSizeModel) {};
	ModelParsedData_t(r5::studiohdr_v17_t* const hdr, const int dataSizePhys, const int dataSizeModel) : sequences(nullptr), poseparams(nullptr), ikchains(nullptr), iklocks(nullptr), studiohdr(hdr, dataSizePhys, dataSizeModel) {};

	~ModelParsedData_t()
	{
		FreeAllocArray(sequences);
		FreeAllocArray(poseparams);
	}

	ModelParsedData_t& operator=(ModelParsedData_t&& parsed)
	{
		if (this != &parsed)
		{
			this->meshVertexData.move(parsed.meshVertexData);
			this->bones.swap(parsed.bones);
			this->attachments.swap(parsed.attachments);
			this->hitboxsets.swap(parsed.hitboxsets);

			this->lods.swap(parsed.lods);
			this->materials.swap(parsed.materials);
			this->skins.swap(parsed.skins);

			this->bodyParts.swap(parsed.bodyParts);

			this->sequences = parsed.sequences;
			this->poseparams = parsed.poseparams;
			this->ikchains = parsed.ikchains;
			this->iklocks = parsed.iklocks;

			parsed.sequences = nullptr;
			parsed.poseparams = nullptr;
			parsed.ikchains = nullptr;
			parsed.iklocks = nullptr;

			this->studiohdr = parsed.studiohdr;
		}

		return *this;
	}

	CRamen meshVertexData;

	std::vector<ModelBone_t> bones;
	std::vector<ModelAttachment_t> attachments;
	std::vector<ModelHitboxSet_t> hitboxsets;

	std::vector<ModelLODData_t> lods;
	std::vector<ModelMaterialData_t> materials;
	std::vector<ModelSkinData_t> skins;

	std::vector<ModelBodyPart_t> bodyParts;

	// Resolved vector of animation sequences for use in model preview
	// [rika]: unused, not quite sure why this was added
	//std::vector<ModelAnimSequence_t> animSequences;
	seqdesc_t* sequences;
	ModelPoseParam_t* poseparams;
	ModelIKChain_t* ikchains;
	ModelIKLock_t* iklocks;

	studiohdr_generic_t studiohdr;

	inline const studiohdr_generic_t* const pStudioHdr() const { return &studiohdr; }
	inline const ModelBone_t* const pBone(const int i) const { return &bones.at(i); }
	inline const ModelAttachment_t* const pAttachment(const size_t i) const { return &attachments.at(i); }
	inline const ModelHitboxSet_t* const pHitboxSet(const size_t i) const { return &hitboxsets.at(i); }
	inline const ModelMaterialData_t* const pMaterial(const int i) const { return &materials.at(i); }
	inline const ModelBodyPart_t* const pBodypart(const size_t i) const { return &bodyParts.at(i); }
	inline const ModelLODData_t* const pLOD(const size_t i) const { return &lods.at(i); }

	inline const int BoneCount() const { return studiohdr.boneCount; }

	inline const int NumLocalSeq() const { return studiohdr.localSequenceCount; }
	inline const seqdesc_t* const LocalSeq(const int seq) const { return &sequences[seq]; }

	FORCEINLINE void SetupBodyPart(int i, const char* partName, const int modelIndex, const int numModels)
	{
		ModelBodyPart_t& part = bodyParts.at(i);
		if (part.partName.empty())
		{
			part.SetName(partName);
			part.modelIndex = modelIndex;
			part.numModels = numModels;
		};
	};
};

void ParseModelBoneData_v8(ModelParsedData_t* const parsedData);
void ParseModelBoneData_v12_1(ModelParsedData_t* const parsedData);
void ParseModelBoneData_v16(ModelParsedData_t* const parsedData);
void ParseModelBoneData_v19(ModelParsedData_t* const parsedData);

void ParseModelAttachmentData_v8(ModelParsedData_t* const parsedData);
void ParseModelAttachmentData_v16(ModelParsedData_t* const parsedData);

void ParseModelHitboxData_v8(ModelParsedData_t* const parsedData);
void ParseModelHitboxData_v16(ModelParsedData_t* const parsedData);

void ParseModelDrawData(ModelParsedData_t* const parsedData, CDXDrawData* const drawData, const uint64_t lod);

void ParseSeqDesc_R2(seqdesc_t* const seqdesc, const std::vector<ModelBone_t>* const bones, const r2::studiohdr_t* const pStudioHdr);
void ParseSeqDesc_R5(seqdesc_t* const seqdesc, const std::vector<ModelBone_t>* const bones, const bool useStall);

// [rika]: this is for model internal sequence data (r5)
void ParseModelSequenceData_NoStall(ModelParsedData_t* const parsedData, char* const baseptr);
template<typename mstudioseqdesc_t> void ParseModelSequenceData_Stall(ModelParsedData_t* const parsedData, char* const baseptr)
{
	assertm(parsedData->bones.size() > 0, "should have bones");

	const studiohdr_generic_t* const pStudioHdr = parsedData->pStudioHdr();

	if (pStudioHdr->localSequenceCount == 0)
		return;

	parsedData->sequences = new seqdesc_t[pStudioHdr->localSequenceCount];

	for (int i = 0; i < pStudioHdr->localSequenceCount; i++)
	{
		parsedData->sequences[i] = seqdesc_t(reinterpret_cast<mstudioseqdesc_t* const>(baseptr + pStudioHdr->localSequenceOffset) + i, nullptr);

		ParseSeqDesc_R5(&parsedData->sequences[i], &parsedData->bones, true);
	}
}

void ParseModelAnimTypes_V8(ModelParsedData_t* const parsedData);
void ParseModelAnimTypes_V16(ModelParsedData_t* const parsedData);

// 
// COMPDATA
//
class CMeshData
{
public:
	CMeshData() : indiceOffset(0ll), vertexOffset(0ll), weightOffset(0ll), texcoordOffset(0ll), size(0ll), writer(nullptr) {};
	~CMeshData() {};

	inline void InitWriter() { writer = (char*)this + IALIGN16(sizeof(CMeshData)); memset(&indiceOffset, 0, sizeof(CMeshData) - sizeof(writer)); };
	inline void DestroyWriter() { size = writer - reinterpret_cast<char*>(this); writer = nullptr; };

	void AddIndices(const uint16_t* const indices, const size_t indiceCount);
	void AddVertices(const Vertex_t* const vertices, const size_t vertexCount);
	void AddWeights(const VertexWeight_t* const weights, const size_t weightCount);
	void AddTexcoords(const Vector2D* const texcoords, const size_t texcoordCount);

	inline uint16_t* const GetIndices() const { return indiceOffset > 0 ? reinterpret_cast<uint16_t*>((char*)this + indiceOffset) : nullptr; };
	inline Vertex_t* const GetVertices() const { return vertexOffset > 0 ? reinterpret_cast<Vertex_t*>((char*)this + vertexOffset) : nullptr; };
	inline VertexWeight_t* const GetWeights() const { return weightOffset > 0 ? reinterpret_cast<VertexWeight_t*>((char*)this + weightOffset) : nullptr; };
	inline Vector2D* const GetTexcoords() const { return texcoordOffset > 0 ? reinterpret_cast<Vector2D*>((char*)this + texcoordOffset) : nullptr; };

	inline const int64_t GetSize() const { return size; };

private:
	int64_t indiceOffset;
	int64_t vertexOffset;
	int64_t weightOffset;
	int64_t texcoordOffset;

	int64_t size; // size of this data

	char* writer; // for writing only
};

// for parsing the animation data
class CAnimDataBone
{
public:
	CAnimDataBone(const size_t frameCount) : flags(0)
	{
		positions.resize(frameCount);
		rotations.resize(frameCount);
		scales.resize(frameCount);
	};

	inline void SetFlags(const uint8_t& flagsIn) { flags |= flagsIn; };
	inline void SetFrame(const int frameIdx, const Vector& pos, const Quaternion& quat, const Vector& scale)
	{
		positions.at(frameIdx) = pos;
		rotations.at(frameIdx) = quat;
		scales.at(frameIdx) = scale;
	}

	enum BoneFlags
	{
		ANIMDATA_POS = 0x1, // bone has pos values
		ANIMDATA_ROT = 0x2, // bone has rot values
		ANIMDATA_SCL = 0x4, // bone has scale values
		ANIMDATA_DATA = (ANIMDATA_POS | ANIMDATA_ROT | ANIMDATA_SCL), // bone has animation data
	};

	inline const uint8_t GetFlags() const { return flags; };
	inline const Vector* GetPosPtr() const { return positions.data(); };
	inline const Quaternion* GetRotPtr() const { return rotations.data(); };
	inline const Vector* GetSclPtr() const { return scales.data(); };

private:
	uint8_t flags;

	std::vector<Vector> positions;
	std::vector<Quaternion> rotations;
	std::vector<Vector> scales;
};

static const int s_AnimDataBoneSizeLUT[8] =
{
	0,
	sizeof(Vector),								// ANIMDATA_POS
	sizeof(Quaternion),							// ANIMDATA_ROT
	sizeof(Vector) + sizeof(Quaternion),		// ANIMDATA_POS | ANIMDATA_ROT
	sizeof(Vector),								// ANIMDATA_SCL
	sizeof(Vector) * 2,							// ANIMDATA_POS | ANIMDATA_SCL
	sizeof(Vector) + sizeof(Quaternion),		// ANIMDATA_ROT | ANIMDATA_SCL
	(sizeof(Vector) * 2) + sizeof(Quaternion),	// ANIMDATA_POS | ANIMDATA_ROT | ANIMDATA_SCL
};

class CAnimData
{
public:
	CAnimData(const int boneCount, const int frameCount) : numBones(boneCount), numFrames(frameCount), pBuffer(nullptr), pOffsets(nullptr), pFlags(nullptr) {};
	CAnimData(char* const buf);

	inline void ReserveVector() { bones.resize(numBones, numFrames); };
	CAnimDataBone& GetBone(const size_t idx) { return bones.at(idx); };

	// mem
	inline const uint8_t GetFlag(const size_t idx) const { return pFlags[idx]; };
	inline const uint8_t GetFlag(const int idx) const { return pFlags[idx]; };
	inline const char* const GetData(const size_t idx) const { return GetFlag(idx) & CAnimDataBone::ANIMDATA_DATA ? reinterpret_cast<const char* const>(pBuffer + pOffsets[idx]) : nullptr; }

	const Vector* const GetBonePosForFrame(const int bone, const int frame) const;
	const Quaternion* const GetBoneQuatForFrame(const int bone, const int frame) const;
	const Vector* const GetBoneScaleForFrame(const int bone, const int frame) const;

	// returns allocated buffer
	const size_t ToMemory(char* const buf);

private:
	int numBones;
	int numFrames;

	bool memory; // memory format

	// mem
	char* const pBuffer;
	const size_t* pOffsets;
	const uint8_t* pFlags;

	std::vector<CAnimDataBone> bones;
};

//
// EXPORT FORMATS
//
enum eModelExportSetting : int
{
	MODEL_CAST,
	MODEL_RMAX,
	MODEL_RMDL,
	MODEL_SMD,

	// rmdl only for now, but can support sourcemodelasset in the future
	MODEL_STL_VALVE_PHYSICS,
	MODEL_STL_RESPAWN_PHYSICS,

	MODEL_COUNT,
};

static const char* s_ModelExportSettingNames[] =
{
	"CAST",
	"RMAX",
	"RMDL",
	"SMD",
	"STL (Valve Physics)", 
	"STL (Respawn Physics)"
};

static const char* s_ModelExportExtensions[] =
{
	".cast",
	".rmax",
	".rmdl",
	".smd",
};

enum eAnimRigExportSetting : int
{
	ANIMRIG_CAST,
	ANIMRIG_RMAX,
	ANIMRIG_RRIG,
	ANIMRIG_SMD,

	ANIMRIG_COUNT,
};

static const char* s_AnimRigExportSettingNames[] =
{
	"CAST",
	"RMAX",
	"RRIG",
	"SMD",
};

enum eAnimSeqExportSetting : int
{
	ANIMSEQ_CAST,
	ANIMSEQ_RMAX,
	ANIMSEQ_RSEQ,
	ANIMSEQ_SMD,

	ANIMSEQ_COUNT,
};

static const char* s_AnimSeqExportSettingNames[] =
{
	"CAST",
	"RMAX",
	"RSEQ",
	"SMD",
};

bool ExportModelRMAX(const ModelParsedData_t* const parsedData, std::filesystem::path& exportPath);
bool ExportModelCast(const ModelParsedData_t* const parsedData, std::filesystem::path& exportPath, const uint64_t guid);
bool ExportModelSMD(const ModelParsedData_t* const parsedData, std::filesystem::path& exportPath);
bool ExportModelQC(const ModelParsedData_t* const parsedData, std::filesystem::path& exportPath, const int setting, const int version);

bool ExportSeqDesc(const int setting, const seqdesc_t* const seqdesc, std::filesystem::path& exportPath, const char* const skelName, const std::vector<ModelBone_t>* const bones, const uint64_t guid);

void UpdateModelBoneMatrix(CDXDrawData* const drawData, const ModelParsedData_t* const parsedData);
void InitModelBoneMatrix(CDXDrawData* const drawData, const ModelParsedData_t* const parsedData);

struct ModelPreviewInfo_t
{
	ModelPreviewInfo_t() : lastSelectedBodypartIndex(0u), selectedBodypartIndex(0u), lastSelectedSkinIndex(0u), selectedSkinIndex(0u), selectedLODLevel(0u), minLODIndex(0u), maxLODIndex(0u)
	{

	}

	std::vector<size_t> bodygroupModelSelected;
	size_t lastSelectedBodypartIndex = 0;
	size_t selectedBodypartIndex = 0;

	size_t lastSelectedSkinIndex = 0;
	size_t selectedSkinIndex = 0;

	// for controlling and handling lod changes
	uint8_t selectedLODLevel = 0u;
	uint8_t minLODIndex = 0u;
	uint8_t maxLODIndex = 0u;
};

void* PreviewParsedData(ModelPreviewInfo_t* const info, ModelParsedData_t* const parsedData, char* const assetName, const uint64_t assetGUID, const bool firstFrameForAsset);
void PreviewSeqDesc(const seqdesc_t* const seqdesc);