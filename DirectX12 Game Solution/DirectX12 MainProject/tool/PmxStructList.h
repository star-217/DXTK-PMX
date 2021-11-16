#pragma once

#include "Base/pch.h"
#include "Base/dxtk.h"

using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::make_unique;
using namespace DirectX;

/*
-------------------------------------------------------------------------------------
	構造体
-------------------------------------------------------------------------------------
*/
struct PmxData {
	//それぞれのタイプ保存
	byte encord;
	byte addUv;
	byte verticesIndex;
	byte textureIndex;
	byte materialIndex;
	byte boneIndex;
	byte morphIndex;
	byte rigidbodyIndex;
	byte weightType;

	//各データの数
	int numVertex;
	int numSurfaces;
	int numTexture;
	int numMaterial;
	int numBone;
	int numLinks;

	//頂点データ構造体
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		//std::vector<XMFLOAT4> addUv;

		struct Weight {
			enum Type
			{
				BDEF,
				BDEF1,
				BDEF4,
				SDEF
			};

			//Type type;
			int born1;
			int born2;
			int born3;
			int born4;
			float weight1;
			float weight2;
			float weight3;
			float weight4;
			XMFLOAT3 c;
			XMFLOAT3 r0;
			XMFLOAT3 r1;


		}weight;
		float edge;
	};

	//面データ
	struct Surface
	{
		int vertexIndex;
	};

	//マテリアルデータ
	struct Material
	{
		std::wstring materialPaths;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
		XMFLOAT3 ambient;

		byte bitFlag;

		XMFLOAT4 edgeColor;
		float edgeSize;

		byte sphereMode;
		byte toonFlag;

		int colorMapTextureIndex;
		int mapTextureIndex;
		int toonTextureIndex;
		int toonTexture;

		int vertexNum;
	};

	//シェーダ側に投げられるマテリアルデータ
	struct MaterialForHlsl {
		XMFLOAT4 diffuse; //ディフューズ色
		XMFLOAT4 specular; //スペキュラ色
		XMFLOAT3 ambient; //アンビエント色
	};
	//それ以外のマテリアルデータ
	struct AdditionalMaterial {
		std::string texPath;//テクスチャファイルパス
		int toonIdx; //トゥーン番号
		bool edgeFlg;//マテリアル毎の輪郭線フラグ
	};
	//まとめたもの
	struct Materials {
		unsigned int indicesNum;//インデックス数
		MaterialForHlsl material;
		AdditionalMaterial additional;
	};

	//ボーン
	struct Bone
	{
		//ボーン名
		std::wstring name;
		std::wstring nameEnglish;

		XMFLOAT3 pos;
		int parentNo;
		int transformationHierarchy;
		unsigned short flag;

		XMFLOAT3 posOffSet;
		int boneIndexSize;

		int parentBoneIndexSize;
		float grantRate;

		XMFLOAT3 axisvVector;

		XMFLOAT3 xAxisVector;
		XMFLOAT3 zAxizVector;

		int keyIndex;

		//IK
		int ikBoneIndexSize;
		int numLoop;
		float axizLimits;


		struct IKLink
		{
			int linkBoneIndexSize;;
			byte limitFlag;
			XMFLOAT3 lowerLimit;
			XMFLOAT3 higherLimit;
		};
		std::vector<IKLink> ikLinks;

	};

	// ボーンの状態
	enum boneFlag
	{
		ACCESS_POINT = 0x0001,
		IK = 0x0020,
		IMPART_TRANSLATION = 0x0100,
		IMPART_ROTATION = 0x0200,
		AXIS_FIXING = 0x0400,
		LOCAL_AXIS = 0x0800,
		EXTERNAL_PARENT_TRANS = 0x2000,
	};

	//各ボーンデータ
	struct BoneNode
	{
		int boneIndex;
		XMFLOAT3 startPos;
		XMFLOAT3 endPos;
		std::vector<BoneNode*> children;
	};

	std::vector<PmxData::Materials> materials;								//マテリアルデータ
	std::vector<PmxData::MaterialForHlsl> shaderData;						//シェーダーデータ
	std::map<std::wstring, BoneNode> boneNodeTable;							//ボーンの接続データ
	std::vector<Vertex> vertices;											//頂点データ
	std::vector<Surface> surfaces;											//面データ
	std::wstring modelName[4];												//モデル名
	std::vector<std::wstring> texturePaths;									//テクスチャ名
	std::vector<Material> material;											//マテリアルデータ
	std::vector<Bone> bones;												//ボーンデータ
};

// シェーダーデータ
struct VSOUT
{
	XMMATRIX world;				//モデル
	XMMATRIX view;
	XMMATRIX proj;
	XMFLOAT3 eye;
	XMMATRIX bone[512];			//ボーン
};

struct VMDData {
	wchar_t name[16];
	unsigned int frame_no;
	XMFLOAT3     location;
	XMFLOAT4     quaternion;
	BYTE         bezier[64];
};


struct VMDKeyFrame {
	unsigned int frame_no;
	XMVECTOR     quaternion;
	XMFLOAT2     p1;
	XMFLOAT2     p2;

	VMDKeyFrame(const unsigned int frame, const XMVECTOR& qt, const XMFLOAT2& pt1, const XMFLOAT2& pt2)
		: frame_no(frame), quaternion(qt), p1(pt1), p2(pt2)
	{}
};