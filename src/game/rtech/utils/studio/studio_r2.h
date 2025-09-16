#pragma once
#include <game/rtech/utils/studio/studio_r1.h>

namespace r2
{
#pragma pack(push, 4)
	//
	// Model Bones
	//

	// 'STUDIO_PROC_JIGGLE' the only one observed in respawn games
	//#define STUDIO_PROC_AXISINTERP	1
	//#define STUDIO_PROC_QUATINTERP	2
	//#define STUDIO_PROC_AIMATBONE	3
	//#define STUDIO_PROC_AIMATATTACH 4
	//#define STUDIO_PROC_JIGGLE		5
	//#define STUDIO_PROC_TWIST_MASTER	6
	//#define STUDIO_PROC_TWIST_SLAVE		7 // Multiple twist bones are computed at once for the same parent/child combo so TWIST_NULL do nothing

	//#define JIGGLE_IS_FLEXIBLE				0x01
	//#define JIGGLE_IS_RIGID					0x02
	//#define JIGGLE_HAS_YAW_CONSTRAINT			0x04
	//#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
	//#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
	//#define JIGGLE_HAS_LENGTH_CONSTRAINT		0x20
	//#define JIGGLE_HAS_BASE_SPRING			0x40

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiojigglebone_t
	{
		int flags;

		// general params
		float length; // how far from bone base, along bone, is tip
		float tipMass;

		// flexible params
		float yawStiffness;
		float yawDamping;
		float pitchStiffness;
		float pitchDamping;
		float alongStiffness;
		float alongDamping;

		// angle constraint
		float angleLimit; // maximum deflection of tip in radians

		// yaw constraint
		float minYaw; // in radians
		float maxYaw; // in radians
		float yawFriction;
		float yawBounce;

		// pitch constraint
		float minPitch; // in radians
		float maxPitch; // in radians
		float pitchFriction;
		float pitchBounce;

		// base spring
		float baseMass;
		float baseStiffness;
		float baseDamping;
		float baseMinLeft;
		float baseMaxLeft;
		float baseLeftFriction;
		float baseMinUp;
		float baseMaxUp;
		float baseUpFriction;
		float baseMinForward;
		float baseMaxForward;
		float baseForwardFriction;
	};

	struct mstudiobone_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int parent; // parent bone
		int bonecontroller[6]; // bone controller index, -1 == none

		// default values
		Vector pos; // base bone position
		Quaternion quat;
		RadianEuler rot; // base bone rotation
		Vector scale; // base bone scale

		// compression scale
		Vector posscale; // scale muliplier for bone position in animations. depreciated in v53, as the posscale is stored in anim bone headers
		Vector rotscale; // scale muliplier for bone rotation in animations
		Vector scalescale; // scale muliplier for bone scale in animations

		matrix3x4_t poseToBone;
		Quaternion qAlignment;

		int flags;
		int proctype;
		int procindex; // procedural rule offset
		int physicsbone; // index into physically simulated bone
		// from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
		int surfacepropidx; // index into string tablefor property name
		inline char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

		int contents; // See BSPFlags.h for the contents flags

		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// map collision
		uint16_t collisionIndex; // base index into collision shapes, 0xFFFF is no collision
		uint16_t collisionCount; // number of collision shapes for this bone, see: models\s2s\s2s_malta_gun_animated.mdl

		int unused[7]; // remove as appropriate
	};
	static_assert(sizeof(mstudiobone_t) == 0xF4);

	// this struct is the same in r1 and r2
	struct mstudiolinearbone_t
	{
		int numbones;

		int flagsindex;
		inline int* pFlags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int*>((char*)this + flagsindex) + i; }
		inline int flags(int i) const { return *pFlags(i); }

		int	parentindex;
		inline int* pParent(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<int*>((char*)this + parentindex) + i;
		}

		int	posindex;
		inline const Vector* pPos(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Vector*>((char*)this + posindex) + i;
		}

		int quatindex;
		inline const Quaternion* pQuat(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + quatindex) + i;
		}

		int rotindex;
		inline const RadianEuler* pRot(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<RadianEuler*>((char*)this + rotindex) + i;
		}

		int posetoboneindex;
		inline const matrix3x4_t* pPoseToBone(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<matrix3x4_t*>((char*)this + posetoboneindex) + i;
		}

		int	posscaleindex; // unused

		int	rotscaleindex;
		inline const Vector* pRotScale(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<Vector*>((char*)this + rotscaleindex) + i; }

		int	qalignmentindex;
		inline const Quaternion* pQAlignment(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + qalignmentindex) + i;
		}

		int unused[6];
	};
	static_assert(sizeof(mstudiolinearbone_t) == 0x40);

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiosrcbonetransform_t
	{
		int sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		matrix3x4_t	pretransform;
		matrix3x4_t	posttransform;
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	//struct mstudioattachment_t

	// this struct is the same in r1, r2, and early r5, and unchanged from p2
	struct mstudiohitboxset_t
	{
		int sznameindex;
		const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		int numhitboxes;
		int hitboxindex;
	};

	struct mstudiobbox_t
	{
		int bone;
		int group;				// intersection group
		Vector bbmin;			// bounding box
		Vector bbmax;
		int szhitboxnameindex;	// offset to the name of the hitbox.
		inline const char* const pszHitboxName() const
		{
			if (szhitboxnameindex == 0)
				return "";

			return reinterpret_cast<const char* const>(this) + szhitboxnameindex;
		}

		int forceCritPoint;	// value of one causes this to act as if it was group of 'head', may either be crit override or group override. mayhaps aligned boolean, look into this
		int hitdataGroupOffset;	// hit_data group in keyvalues this hitbox uses.

		int unused[6];
	};


	//
	// Model IK Info
	//

	//struct mstudioiklock_t

	//struct mstudioiklink_t

	struct mstudioikchain_t
	{
		int	sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		int	linktype;
		int	numlinks;
		int	linkindex;
		const mstudioiklink_t* const pLink(int i) const { return reinterpret_cast<const mstudioiklink_t* const>((char*)this + linkindex) + i; }

		float unk_10;	// tried, and tried to find what this does, but it doesn't seem to get hit in any normal IK code paths
		// however, in Apex Legends this value is 0.866 which happens to be cos(30) (https://github.com/NicolasDe/AlienSwarm/blob/master/src/public/studio.h#L2173)
		// and in Titanfall 2 it's set to 0.707 or alternatively cos(45).
		// TLDR: likely set in QC $ikchain command from a degrees entry, or set with a default value in studiomdl when not defined.

		int	unused[3];
	};

	// these should not be present in respawn games, as it was deprecated during hl2's development, despite this support for them remains even up to modern r5.
	struct mstudioikerror_t
	{
		Vector pos;
		Quaternion q;
	};

	struct mstudiocompressedikerror_t
	{
		float scale[6];
		short offset[6];
	};

	struct mstudioikrule_t
	{
		int index;
		int type;
		int chain;
		int bone;

		int slot; // iktarget slot. Usually same as chain.
		float height;
		float radius;
		float floor;
		Vector pos;
		Quaternion q;

		int compressedikerrorindex;

		int iStart;
		int ikerrorindex;

		float start; // beginning of influence
		float peak; // start of full influence
		float tail; // end of full influence
		float end; // end of all influence

		float contact; // frame footstep makes ground concact
		float drop; // how far down the foot should drop when reaching for IK
		float top; // top of the foot box

		int szattachmentindex; // name of world attachment

		float endHeight; // new in v52
		// titan_buddy_mp_core.mdl

		int unused[8];
	};


	//
	// Model Animation
	// 

	// union mstudioanimvalue_t

	//struct mstudioanim_valueptr_t

	// These work as toggles, flag enabled = raw data, flag disabled = "pointers", with rotations
	enum RleFlags_t : uint8_t
	{
		STUDIO_ANIM_DELTA = 0x01, // delta animation
		STUDIO_ANIM_RAWPOS = 0x02, // Vector48
		STUDIO_ANIM_RAWROT = 0x04, // Quaternion64
		STUDIO_ANIM_RAWSCALE = 0x08, // Vector48, drone_frag.mdl for scale track usage
		STUDIO_ANIM_NOROT = 0x10  // this flag is used to check if there is 1. no static rotation and 2. no rotation track
		// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/bone_setup.cpp#L393 used for this
		// this flag is used when the animation has no rotation data, it exists because it is not possible to check this as there are not separate flags for static/track rotation data
	};

	struct mstudio_rle_anim_t
	{
		float posscale; // does what posscale is used for

		uint8_t bone;
		uint8_t flags;

		inline char* pData() const { return ((char*)this + sizeof(mstudio_rle_anim_t)); } // gets start of animation data, this should have a '+2' if aligned to 4
		inline mstudioanim_valueptr_t* pRotV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData()); } // returns rot as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pPosV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + 8); } // returns pos as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pScaleV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + 14); } // returns scale as mstudioanim_valueptr_t

		inline Quaternion64* pQuat64() const { return reinterpret_cast<Quaternion64*>(pData()); } // returns rot as static Quaternion64
		inline Vector48* pPos() const { return reinterpret_cast<Vector48*>(pData() + 8); } // returns pos as static Vector48
		inline Vector48* pScale() const { return reinterpret_cast<Vector48*>(pData() + 14); } // returns scale as static Vector48

		// points to next bone in the list
		inline int* pNextOffset() const { return reinterpret_cast<int*>(pData() + 20); }
		inline mstudio_rle_anim_t* pNext() const
		{
			const int nextOffset = *pNextOffset();
			return nextOffset ? reinterpret_cast<mstudio_rle_anim_t*>((char*)this + nextOffset) : nullptr;
		}
	};
	static_assert(sizeof(mstudio_rle_anim_t) == 0x8);

	struct mstudioanimsections_t
	{
		int animindex;
	};
	static_assert(sizeof(mstudioanimsections_t) == 0x4);

	struct mstudioanimdesc_t
	{
		int baseptr;

		int sznameindex;
		inline const char* pszName() const { return ((char*)this + sznameindex); }

		float fps; // frames per second	
		int flags; // looping/non-looping flags

		int numframes;

		// piecewise movement
		int nummovements;
		int movementindex;
		inline const mstudiomovement_t* const pMovement(int i) const { return reinterpret_cast<const mstudiomovement_t* const>((char*)this + movementindex) + i; };

		int framemovementindex; // new in v52
		inline const r1::mstudioframemovement_t* pFrameMovement() const { return reinterpret_cast<const r1::mstudioframemovement_t* const>((char*)this + framemovementindex); }

		int animindex; // non-zero when anim data isn't in sections
		//const mstudio_rle_anim_t* const pAnim(int* piFrame) const; // returns pointer to data and new frame index

		int numikrules;
		int ikruleindex; // non-zero when IK data is stored in the mdl

		int numlocalhierarchy;
		int localhierarchyindex;

		int sectionindex;
		int sectionframes; // number of frames used in each fast lookup section, zero if not used
		inline const mstudioanimsections_t* const pSection(int i) const { return reinterpret_cast<const mstudioanimsections_t* const>((char*)this + sectionindex) + i; }

		int unused[8];
	};
	static_assert(sizeof(mstudioanimdesc_t) == 0x5C);

	//struct mstudioevent_t

	//struct mstudioautolayer_t

	//struct mstudioactivitymodifier_t

	struct mstudioseqdesc_t
	{
		int baseptr;

		int	szlabelindex;
		inline const char* pszLabel() const { return ((char*)this + szlabelindex); }

		int szactivitynameindex;
		inline const char* pszActivityName() const { return ((char*)this + szactivitynameindex); }

		int flags; // looping/non-looping flags

		int activity; // initialized at loadtime to game DLL values
		int actweight;

		int numevents;
		int eventindex;
		//inline const mstudioevent_t* const pEvent(const int i) const { return reinterpret_cast<mstudioevent_t*>((char*)this + eventindex) + i; }

		Vector bbmin; // per sequence bounding box
		Vector bbmax;

		int numblends;

		// Index into array of shorts which is groupsize[0] x groupsize[1] in length
		int animindexindex;
		inline const short* const pAnimIndex(const int i) const { return reinterpret_cast<short*>((char*)this + animindexindex) + i; }
		inline const short GetAnimIndex(const int i) const { return *pAnimIndex(i); }
		const int AnimCount() const { return  groupsize[0] * groupsize[1]; }

		int movementindex; // [blend] float array for blended movement
		int groupsize[2];
		int paramindex[2]; // X, Y, Z, XR, YR, ZR
		float paramstart[2]; // local (0..1) starting value
		float paramend[2]; // local (0..1) ending value
		int paramparent;

		float fadeintime; // ideal cross fate in time (0.2 default)
		float fadeouttime; // ideal cross fade out time (0.2 default)

		int localentrynode; // transition node at entry
		int localexitnode; // transition node at exit
		int nodeflags; // transition rules

		float entryphase; // used to match entry gait
		float exitphase; // used to match exit gait

		float lastframe; // frame that should generation EndOfSequence

		int nextseq; // auto advancing sequences
		int pose; // index of delta animation between end and nextseq

		int numikrules;

		int numautolayers;
		int autolayerindex;
		//inline const r1::mstudioautolayer_t* const pAutoLayer(const int i) const { return reinterpret_cast<r1::mstudioautolayer_t*>((char*)this + autolayerindex) + i; }

		int weightlistindex;
		inline const float* const pWeightList() const { return reinterpret_cast<float*>((char*)this + weightlistindex); }
		inline const float weight(const int weightidx) const { return pWeightList()[weightidx]; }

		int posekeyindex;

		int numiklocks;
		int iklockindex;

		// Key values
		int keyvalueindex;
		int keyvaluesize;
		inline const char* pKeyValues() const { return ((char*)this + keyvalueindex); }

		int cycleposeindex; // index of pose parameter to use as cycle index

		int activitymodifierindex;
		int numactivitymodifiers;
		//inline const r1::mstudioactivitymodifier_t* const pActMod(const int i) const { return reinterpret_cast<r1::mstudioactivitymodifier_t*>((char*)this + activitymodifierindex) + i; }

		int ikResetMask;	// new in v52
		// titan_buddy_mp_core.mdl
		// reset all ikrules with this type???
		int unk_C4;	// previously 'unk1'
		// mayhaps this is the ik type applied to the mask if what's above it true

		int unused[8];
	};
	static_assert(sizeof(mstudioseqdesc_t) == 0xE8);

	//struct mstudioposeparamdesc_t

	//struct mstudiomodelgroup_t


	//
	// Model Bodyparts
	//

	struct mstudiomodel_t;

	struct mstudiomesh_t
	{
		int material;

		int modelindex;
		const mstudiomodel_t* const pModel() const { return reinterpret_cast<const mstudiomodel_t* const>((char*)this + modelindex); }

		int numvertices;	// number of unique vertices/normals/texcoords
		int vertexoffset;	// vertex mstudiovertex_t
		// offset by vertexoffset number of verts into vvd vertexes, relative to the models offset

		// Access thin/fat mesh vertex data (only one will return a non-NULL result)

		int deprecated_numflexes; // vertex animation
		int deprecated_flexindex;

		// special codes for material operations
		int deprecated_materialtype;
		int deprecated_materialparam;

		// a unique ordinal for this mesh
		int meshid;

		Vector center;

		mstudio_meshvertexloddata_t vertexloddata;

		void* unk_54; // unknown memory pointer, probably one of the older vertex pointers but moved

		int unused[6];
	};
	static_assert(sizeof(mstudiomesh_t) == 0x74);

	struct mstudiomodel_t
	{
		char name[64];

		int type;

		float boundingradius;

		int nummeshes;
		int meshindex;
		const mstudiomesh_t* const pMesh(int i) const { return reinterpret_cast<const mstudiomesh_t* const>((char*)this + meshindex) + i; }

		// cache purposes
		int numvertices; // number of unique vertices/normals/texcoords
		int vertexindex; // vertex Vector
		// offset by vertexindex number of chars into vvd verts
		int tangentsindex; // tangents Vector
		// offset by tangentsindex number of chars into vvd tangents

		int numattachments;
		int attachmentindex;

		int deprecated_numeyeballs;
		int deprecated_eyeballindex;

		int pad[4];

		int colorindex; // vertex color
		// offset by colorindex number of chars into vvc vertex colors
		int uv2index;	// vertex second uv map
		// offset by uv2index number of chars into vvc secondary uv map

		int unused[4];
	};
	static_assert(sizeof(mstudiomodel_t) == 0x94);

	//struct mstudiobodyparts_t

	struct mstudiotexture_t
	{
		int sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		int unused_flags;
		int used;

		int unused[8];
	};
	static_assert(sizeof(mstudiotexture_t) == 0x2C);


	//
	// Studio Header
	//

	struct studiohdr_t
	{
		int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
		int version; // Format version number, such as 53 (0x35,0x00,0x00,0x00)
		int checksum; // This has to be the same in the phy and vtx files to load!
		int sznameindex; // This has been moved from studiohdr2_t to the front of the main header.
		inline char* const pszName() const { return ((char*)this + sznameindex); }
		char name[64]; // The internal name of the model, padding with null chars.
		// Typically "my_model.mdl" will have an internal name of "my_model"
		int length; // Data size of MDL file in chars.

		Vector eyeposition;	// ideal eye position

		Vector illumposition;	// illumination center

		Vector hull_min;		// ideal movement hull size
		Vector hull_max;

		Vector view_bbmin;		// clipping bounding box
		Vector view_bbmax;

		int flags;

		// highest observed: 250
		// max is definitely 256 because 8bit uint limit
		int numbones; // bones
		int boneindex;
		inline const mstudiobone_t* const pBone(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<mstudiobone_t*>((char*)this + boneindex) + i; }

		int numbonecontrollers; // bone controllers
		int bonecontrollerindex;

		int numhitboxsets;
		int hitboxsetindex;

		int numlocalanim; // animations/poses
		int localanimindex; // animation descriptions
		inline const mstudioanimdesc_t* const pAnimdesc(const int i) const { assert(i >= 0 && i < numlocalanim); return reinterpret_cast<mstudioanimdesc_t*>((char*)this + localanimindex) + i; }

		int numlocalseq; // sequences
		int	localseqindex;
		inline const mstudioseqdesc_t* const pSeqdesc(const int i) const { assert(i >= 0 && i < numlocalseq); return reinterpret_cast<mstudioseqdesc_t*>((char*)this + localseqindex) + i; }

		int activitylistversion; // initialization flag - have the sequences been indexed? set on load
		int eventsindexed;

		// mstudiotexture_t
		// short rpak path
		// raw textures
		int numtextures; // the material limit exceeds 128, probably 256.
		int textureindex;
		inline const mstudiotexture_t* const pTexture(int i) const { assert(i >= 0 && i < numtextures); return reinterpret_cast<mstudiotexture_t*>((char*)this + textureindex) + i; }

		// this should always only be one, unless using vmts.
		// raw textures search paths
		int numcdtextures;
		int cdtextureindex;

		// replaceable textures tables
		int numskinref;
		int numskinfamilies;
		int skinindex;

		int numbodyparts;
		int bodypartindex;
		inline const mstudiobodyparts_t* const pBodypart(int i) const { assert(i >= 0 && i < numbodyparts); return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i; }

		int numlocalattachments;
		int localattachmentindex;

		int numlocalnodes;
		int localnodeindex;
		int localnodenameindex;
		int localNodeUnk; // something about having nodes while include models also have nodes??? used only three times in r2, never used in apex, removed in rmdl v16. super_spectre_v1, titan_buddy, titan_buddy_skyway

		int deprecated_flexdescindex;

		int deprecated_numflexcontrollers;
		int deprecated_flexcontrollerindex;

		int deprecated_numflexrules;
		int deprecated_flexruleindex;

		int numikchains;
		int ikchainindex;

		int uiPanelCount;
		int uiPanelOffset;

		int numlocalposeparameters;
		int localposeparamindex;

		int surfacepropindex;

		int keyvalueindex;
		int keyvaluesize;

		int numlocalikautoplaylocks;
		int localikautoplaylockindex;

		float mass;
		int contents;

		// external animations, models, etc.
		int numincludemodels;
		int includemodelindex;

		// implementation specific back pointer to virtual data
		int /* mutable void* */ virtualModel;

		int bonetablebynameindex;

		// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
		// this value is used to calculate directional components of lighting 
		// on static props
		uint8_t constdirectionallightdot;

		// set during load of mdl data to track *desired* lod configuration (not actual)
		// the *actual* clamped root lod is found in studiohwdata
		// this is stored here as a global store to ensure the staged loading matches the rendering
		uint8_t rootLOD;

		// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
		// to be set as root LOD:
		//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
		//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
		uint8_t numAllowedRootLODs;

		uint8_t unused;

		float fadeDistance; // set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
		// player/titan models seem to inherit this value from the first model loaded in menus.
		// works oddly on entities, probably only meant for static props

		int deprecated_numflexcontrollerui;
		int deprecated_flexcontrolleruiindex;

		float flVertAnimFixedPointScale;
		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// stored maya files from used dmx files, animation files are not added. for internal tools likely
		// in r1 this is a mixed bag, some are null with no data, some have a four byte section, and some actually have the files stored.
		int sourceFilenameOffset;
		inline char* const pszSourceFiles() const { return ((char*)this + sourceFilenameOffset); }

		int numsrcbonetransform;
		int srcbonetransformindex;

		int	illumpositionattachmentindex;

		int linearboneindex;
		inline const r1::mstudiolinearbone_t* const pLinearBones() const { return linearboneindex ? reinterpret_cast<const r1::mstudiolinearbone_t* const>((char*)this + linearboneindex) : nullptr; }

		int m_nBoneFlexDriverCount;
		int m_nBoneFlexDriverIndex;

		// for static props (and maybe others)
		// Precomputed Per-Triangle AABB data
		int m_nPerTriAABBIndex;
		int m_nPerTriAABBNodeCount;
		int m_nPerTriAABBLeafCount;
		int m_nPerTriAABBVertCount;

		// always "" or "Titan", this is probably for internal tools
		int unkStringOffset;
		inline char* const pszUnkString() const { return ((char*)this + unkStringOffset); }

		// ANIs are no longer used and this is reflected in many structs
		// start of interal file data
		int vtxOffset; // VTX
		int vvdOffset; // VVD / IDSV
		int vvcOffset; // VVC / IDCV 
		int phyOffset; // VPHY / IVPS

		int vtxSize; // VTX
		int vvdSize; // VVD / IDSV
		int vvcSize; // VVC / IDCV 
		int phySize; // VPHY / IVPS

		inline OptimizedModel::FileHeader_t* const pVTX() const { return vtxSize > 0 ? reinterpret_cast<OptimizedModel::FileHeader_t*>((char*)this + vtxOffset) : nullptr; }
		inline vvd::vertexFileHeader_t* const pVVD() const { return vvdSize > 0 ? reinterpret_cast<vvd::vertexFileHeader_t*>((char*)this + vvdOffset) : nullptr; }
		inline vvc::vertexColorFileHeader_t* const pVVC() const { return vvcSize > 0 ? reinterpret_cast<vvc::vertexColorFileHeader_t*>((char*)this + vvcOffset) : nullptr; }
		inline ivps::phyheader_t* const pPHY() const { return phySize > 0 ? reinterpret_cast<ivps::phyheader_t*>((char*)this + phyOffset) : nullptr; }

		// for map collision, phys and this are merged in r5
		int collisionOffset;
		int staticCollisionCount; // number of sections for static props, one bone, or a phys that has one (1) solid. if set to 0, and this value is checked, defaults to 1

		// mostly seen on '_animated' suffixed models
		// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
		int boneFollowerCount;
		int boneFollowerOffset; // index only written when numbones > 1, means whatever func writes this likely checks this (would make sense because bonefollowers need more than one bone to even be useful). maybe only written if phy exists
		inline const int* const pBoneFollowers(int i) const { assert(i >= 0 && i < boneFollowerCount); return reinterpret_cast<int*>((char*)this + boneFollowerOffset) + i; }
		inline const int BoneFollower(int i) const { return *pBoneFollowers(i); }

		int unused1[60];

	};
	static_assert(sizeof(studiohdr_t) == 0x2CC);
#pragma pack(pop)

	void CalcBoneQuaternion(int frame, float s,
		const mstudiobone_t* pBone,
		const r1::mstudiolinearbone_t* pLinearBones,
		const mstudio_rle_anim_t* panim, Quaternion& q);

	void CalcBonePosition(int frame, float s,
		const mstudiobone_t* pBone,
		const r1::mstudiolinearbone_t* pLinearBones,
		const mstudio_rle_anim_t* panim, Vector& pos);

	void CalcBoneScale(int frame, float s,
		const Vector& baseScale, const Vector& baseScaleScale,
		const mstudio_rle_anim_t* panim, Vector& scale);
}