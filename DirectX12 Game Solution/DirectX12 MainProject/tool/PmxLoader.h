/**
 * @file PmxLoader.h
 * @brief PMX読み込み＆描画
 * @author hoshi hirofumi
 * @date 2021/10/11
 */

#pragma once

/*
-------------------------------------------------------------------------------------
	インクルード
-------------------------------------------------------------------------------------
*/
#include "PmxStructList.h"
#include "VMDLoader.h"

/*
-------------------------------------------------------------------------------------
	using
-------------------------------------------------------------------------------------
*/
using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::make_unique;
using namespace DirectX;
using namespace DirectX::SimpleMath;



/*
-------------------------------------------------------------------------------------
	PmxLoaderクラス　宣言
-------------------------------------------------------------------------------------
*/
class PmxLoader {
public:
	PmxLoader();
	virtual ~PmxLoader() {}

	PmxLoader(PmxLoader&&) = default;
	PmxLoader& operator= (PmxLoader&&) = default;

	PmxLoader(PmxLoader const&) = delete;
	PmxLoader& operator= (PmxLoader const&) = delete;

	void PmxRead(const char*);
	void Update();
	void Render();
	void SetShader(LPCWSTR vs, LPCWSTR ps)
	{
		vsfileName = vs;
		psfileName = ps;
	};
	void SetCamera(DX12::CAMERA camera);
	void SetPosition(Vector3 position);
	void SetScale(Vector3 scale);
	void SetRotation(Vector3 rotation);

	//アニメーション
	void Animetion(float deltaTime);


private:
	//モデル情報ロード
	void Header(FILE* fp);
	void Vertex(FILE* fp);
	void Surface(FILE* fp);
	void Texture(FILE* fp);
	void Material(FILE* fp);
	void Born(FILE* fp);
	void Map();

	//描画準備
	void SetUp();
	void VertexBuffer(D3D12_HEAP_PROPERTIES, D3D12_RESOURCE_DESC);
	void IndexBuffer(D3D12_HEAP_PROPERTIES, D3D12_RESOURCE_DESC);
	void ConstantBuffer(D3D12_HEAP_PROPERTIES, D3D12_RESOURCE_DESC);
	void InitShader();
	void ExportTexture();
	void CreatePipeLine();

	bool getPMXStringUTF16(FILE* _file, std::wstring& output);

	//! 頂点バッファー　ビュー
	ComPtr<ID3D12Resource>		m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView;

	//! シェーダー
	ComPtr<ID3DBlob>			m_vsBlob;
	ComPtr<ID3DBlob>			m_psBlob;

	//!　ルートシグネチャ　パイプライン
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	//!　インデックスバッファー　ビュー
	ComPtr<ID3D12Resource>		m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW		m_indexBufferView;

	//! コンスタントバッファー
	ComPtr<ID3D12Resource> m_constantBuffer;
	//! マテリアル用
	ComPtr<ID3D12Resource> m_materialBuffer;

	//! テクスチャリソース
	std::vector<ComPtr<ID3D12Resource>>  m_texture;
	std::vector<ComPtr<ID3D12Resource>>  m_sphTexture;
	std::vector<ComPtr<ID3D12Resource>>  m_toonTexture;

	SimpleMath::Matrix m_position;
	SimpleMath::Matrix m_scale;
	SimpleMath::Matrix m_rotation;


	SimpleMath::Matrix m_worldTransform;

	unique_ptr<DescriptorHeap>  m_resourceDescriptors;
	unique_ptr<DescriptorHeap>  m_materialDescriptors;

	//! ファイルパス
	std::wstring m_ps;

	PmxData m_data;
	VMDLoader m_vmd;

	LPCWSTR vsfileName = L"Shaders/VS.hlsl";
	LPCWSTR psfileName = L"Shaders/PS.hlsl";

	DX12::CAMERA m_camera;

};