/**
 * @file PmxLoader.cpp
 * @brief PMX読み込み＆描画
 * @author hoshi hirofumi
 * @date 2021/10/11
 */

#include "PmxLoader.h"

#include <array>
#include <d3dcompiler.h>
#include <filesystem>


// Initialize member variables.

PmxLoader::PmxLoader() : m_vertexBuffer(nullptr), m_vertexBufferView{},
m_vsBlob(nullptr), m_psBlob(nullptr), m_rootSignature(nullptr), m_pipelineState(nullptr),
m_indexBuffer(nullptr),m_constantBuffer(nullptr), m_materialBuffer(nullptr),
m_texture(NULL), m_position(), m_scale(), m_rotation(), m_worldTransform(),
m_resourceDescriptors(nullptr), m_materialDescriptors(nullptr), m_ps(), m_data{}
{
}

/**
 * @brief Pmxを読み込む
 * @param fileName ファイル名
 */
void PmxLoader::PmxRead(const char* fileName)
{
	std::filesystem::path ps =fileName;
	ps.remove_filename();

	m_ps = ps;

	// ファイルを開く
	FILE* fp;
	fp = fopen(fileName, "rb");

	Header(fp);
	Vertex(fp);
	Surface(fp);
	Texture(fp);
	Material(fp);
	Born(fp);
	Map();
	SetUp();
}

void PmxLoader::Update()
{
	m_worldTransform = m_scale * m_rotation * m_position;

	VSOUT* map_buffer = nullptr;
	m_constantBuffer->Map(0, nullptr, (void**)&map_buffer);
	map_buffer->world = m_worldTransform;
	map_buffer->view = m_camera->GetViewMatrix();
	map_buffer->proj = m_camera->GetProjectionMatrix();
	map_buffer->eye = m_camera->GetForwardVector();
	m_constantBuffer->Unmap(0, nullptr);
}

/**
	@brief	描画
*/
void PmxLoader::Render()
{

	DXTK->CommandList->SetPipelineState(m_pipelineState.Get());
	DXTK->CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
	DXTK->CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DXTK->CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	DXTK->CommandList->IASetIndexBuffer(&m_indexBufferView);

	unsigned int idx0ffset = 0;
	ID3D12DescriptorHeap* heapes[2] = { m_resourceDescriptors->Heap(), m_materialDescriptors->Heap() };

	DXTK->CommandList->SetDescriptorHeaps(1, &heapes[0]);
	DXTK->CommandList->SetGraphicsRootDescriptorTable(0, m_resourceDescriptors->GetGpuHandle(0));
	DXTK->CommandList->SetDescriptorHeaps(1, &heapes[1]);
	for (int i = 0; i < m_data.material.size(); ++i) {
		DXTK->CommandList->SetGraphicsRootDescriptorTable(1, m_materialDescriptors->GetGpuHandle(i * 4));
		DXTK->CommandList->DrawIndexedInstanced(m_data.materials[i].indicesNum, 1, idx0ffset, 0, 0);
		idx0ffset += m_data.materials[i].indicesNum;
	}

}

/**
	@brief	カメラ設定
	@param	camera	カメラの行列(外部ライブラリのカメラを使用)
*/
void PmxLoader::SetCamera(DX12::CAMERA camera)
{

	m_worldTransform = m_scale * m_rotation * m_position;
	m_camera = camera;

	VSOUT* map_buffer = nullptr;
	m_constantBuffer->Map(0, nullptr, (void**)&map_buffer);
	map_buffer->world = m_worldTransform;
	map_buffer->view = m_camera->GetViewMatrix();
	map_buffer->proj = m_camera->GetProjectionMatrix();
	map_buffer->eye = m_camera->GetForwardVector();
	m_constantBuffer->Unmap(0, nullptr);

}

/**
	@brief	ポジション設定
	@param	position ポジション
*/
void PmxLoader::SetPosition(Vector3 position = Vector3(0,0,0))
{
	m_position = Matrix::CreateTranslation(position);
}

/**
	@brief　スケール設定
	@param	scale 拡大率
*/
void PmxLoader::SetScale(Vector3 scale = Vector3(1,1,1))
{
	m_scale = Matrix::CreateScale(scale);
}

/**
	@brief	ローテーション設定
	@param	rotation ローテーション
*/
void PmxLoader::SetRotation(Vector3 rotation = Vector3(0, 0, 0))
{
	m_rotation = Matrix::CreateFromYawPitchRoll(
		XMConvertToRadians(rotation.y),
		XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.z)
	);
}

void PmxLoader::Animetion(float deltaTime)
{
	m_vmd.Update(deltaTime);
}

/**
 * @brief ヘッダーを読み込む
 * @param fp ファイルデータ
 */
void PmxLoader::Header(FILE* fp)
{
	// ヘッダーチェック
	BYTE header[4];
	fread(header, 4, 1, fp);
	if (memcmp(header, "PMX ", 4) != 0)
		DX::ThrowIfFailed(E_FAIL);

	float ver;
	fread(&ver, sizeof(ver), 1, fp);
	if (ver < 2.0f)
		DX::ThrowIfFailed(E_FAIL);

	fseek(fp, 1, SEEK_CUR);
	fread(&m_data.encord, 1, 1, fp);
	fread(&m_data.addUv, 1, 1, fp);
	fread(&m_data.verticesIndex, 1, 1, fp);
	fread(&m_data.textureIndex, 1, 1, fp);
	fread(&m_data.materialIndex, 1, 1, fp);
	fread(&m_data.boneIndex, 1, 1, fp);
	fread(&m_data.morphIndex, 1, 1, fp);
	fread(&m_data.rigidbodyIndex, 1, 1, fp);

	//モデル情報
	std::wstring m_name;
	for (int i = 0; i < 4; i++)
	{
		getPMXStringUTF16(fp, m_name);
		m_data.modelName[i] = m_name;
	}
}

/**
 * @brief 頂点データを読み込む
 * @param fp ファイルデータ
 */
void PmxLoader::Vertex(FILE* fp)
{
	// 頂点数を読み込む
	fread(&m_data.numVertex, sizeof(int), 1, fp);

	// 配列を確保する
	m_data.vertices.resize(m_data.numVertex);

	// ファイルから頂点データを取得
	for (int i = 0; i < m_data.numVertex; ++i) {
		fread(&m_data.vertices[i].pos, sizeof(XMFLOAT3), 1, fp);
		fread(&m_data.vertices[i].normal, sizeof(XMFLOAT3), 1, fp);
		fread(&m_data.vertices[i].uv, sizeof(XMFLOAT2), 1, fp);

		byte weighttype;
		fread(&weighttype, 1, 1, fp);

		switch (weighttype) {
		case PmxData::Vertex::Weight::BDEF:
			//_data.vertices[i].weight.type = PmxData::Vertex::Weight::BDEF;
			fread(&m_data.vertices[i].weight.born1, m_data.boneIndex, 1, fp);
			m_data.vertices[i].weight.born2 = -1;
			m_data.vertices[i].weight.born3 = -1;
			m_data.vertices[i].weight.born4 = -1;
			m_data.vertices[i].weight.weight1 = 1.0f;
			break;

		case PmxData::Vertex::Weight::BDEF1:
			//_data.vertices[i].weight.type = PmxData::Vertex::Weight::BDEF1;
			fread(&m_data.vertices[i].weight.born1, m_data.boneIndex, 1, fp);
			fread(&m_data.vertices[i].weight.born2, m_data.boneIndex, 1, fp);
			m_data.vertices[i].weight.born3 = -1;
			m_data.vertices[i].weight.born4 = -1;
			fread(&m_data.vertices[i].weight.weight1, 4, 1, fp);
			m_data.vertices[i].weight.weight2 = 1.0f - m_data.vertices[i].weight.weight1;
			break;

		case PmxData::Vertex::Weight::BDEF4:
			//_data.vertices[i].weight.type = PmxData::Vertex::Weight::BDEF4;
			fread(&m_data.vertices[i].weight.born1, m_data.boneIndex, 1, fp);
			fread(&m_data.vertices[i].weight.born2, m_data.boneIndex, 1, fp);
			fread(&m_data.vertices[i].weight.born3, m_data.boneIndex, 1, fp);
			fread(&m_data.vertices[i].weight.born4, m_data.boneIndex, 1, fp);
			fread(&m_data.vertices[i].weight.weight1, 4, 1, fp);
			fread(&m_data.vertices[i].weight.weight2, 4, 1, fp);
			fread(&m_data.vertices[i].weight.weight3, 4, 1, fp);
			fread(&m_data.vertices[i].weight.weight4, 4, 1, fp);
			break;

		case PmxData::Vertex::Weight::SDEF:
			//_data.vertices[i].weight.type = PmxData::Vertex::Weight::SDEF;
			fread(&m_data.vertices[i].weight.born1, m_data.boneIndex, 1, fp);
			fread(&m_data.vertices[i].weight.born2, m_data.boneIndex, 1, fp);
			fread(&m_data.vertices[i].weight.weight1, 4, 1, fp);
			m_data.vertices[i].weight.weight2 = 1.0f - m_data.vertices[i].weight.weight1;
			fread(&m_data.vertices[i].weight.c, 12, 1, fp);
			fread(&m_data.vertices[i].weight.r0, 12, 1, fp);
			fread(&m_data.vertices[i].weight.r1, 12, 1, fp);
			break;

		}
		fread(&m_data.vertices[i].edge, 4, 1, fp);
	}
}

/**
 * @brief インデックスデータを読み込む
 * @param fp ファイルデータ
 */
void PmxLoader::Surface(FILE* fp)
{
	//面
	fread(&m_data.numSurfaces, 4, 1, fp);
	m_data.surfaces.resize(m_data.numSurfaces);
	for (int i = 0; i < m_data.numSurfaces; i++)
		fread(&m_data.surfaces[i].vertexIndex, m_data.verticesIndex, 1, fp);
}

/**
 * @brief テクスチャ名を読み込む
 * @param fp ファイルデータ
 */
void PmxLoader::Texture(FILE* fp)
{
	//テクスチャ
	fread(&m_data.numTexture, 4, 1, fp);
	m_data.texturePaths.resize(m_data.numTexture);
	std::wstring texturePath{};
	for (int i = 0; i < m_data.numTexture; i++)
	{
		getPMXStringUTF16(fp, texturePath);

		m_data.texturePaths[i] += texturePath;
	}
}

/**
 * @brief マテリアルデータを読み込む
 * @param fp ファイルデータ
 */
void PmxLoader::Material(FILE* fp)
{
	//マテリアル
	fread(&m_data.numMaterial, 4, 1, fp);
	m_data.material.resize(m_data.numMaterial);
	std::wstring materialPath{};
	for (int i = 0; i < m_data.numMaterial; i++)
	{
		for (int k = 0; k < 2; k++) {
			getPMXStringUTF16(fp, materialPath);
			m_data.material[i].materialPaths += materialPath;
		}

		fread(&m_data.material[i].diffuse, 16, 1, fp);
		fread(&m_data.material[i].specular, 16, 1, fp);
		fread(&m_data.material[i].ambient, 12, 1, fp);

		fread(&m_data.material[i].bitFlag, 1, 1, fp);
		fread(&m_data.material[i].edgeColor, 16, 1, fp);
		fread(&m_data.material[i].edgeSize, 4, 1, fp);
		fread(&m_data.material[i].colorMapTextureIndex, m_data.textureIndex, 1, fp);
		fread(&m_data.material[i].mapTextureIndex, m_data.textureIndex, 1, fp);

		fread(&m_data.material[i].sphereMode, 1, 1, fp);

		fread(&m_data.material[i].toonFlag, 1, 1, fp);

		if (m_data.material[i].toonFlag)
			fread(&m_data.material[i].toonTextureIndex, 1, 1, fp);
		else {
			fread(&m_data.material[i].toonTexture, m_data.textureIndex, 1, fp);
		}
		int memo;
		fread(&memo, 4, 1, fp);
		fseek(fp, memo, SEEK_CUR);
		fread(&m_data.material[i].vertexNum, 4, 1, fp);
	}
}

/**
 * @brief ボーンデータを読み込む
 * @param fp ファイルデータ
 */
void PmxLoader::Born(FILE* fp)
{
	//ボーン
	fread(&m_data.numBone, 4, 1, fp);
	m_data.bones.resize(m_data.numBone);
	std::wstring mate_name;
	for (int i = 0; i < m_data.numBone; i++)
	{
		getPMXStringUTF16(fp, mate_name);
		m_data.bones[i].name += mate_name;

		getPMXStringUTF16(fp, mate_name);
		m_data.bones[i].nameEnglish += mate_name;

		fread(&m_data.bones[i].pos, 12, 1, fp);
		fread(&m_data.bones[i].parentNo, m_data.boneIndex, 1, fp);
		fread(&m_data.bones[i].transformationHierarchy, 4, 1, fp);
		fread(&m_data.bones[i].flag, 2, 1, fp);

		if (m_data.bones[i].flag & PmxData::ACCESS_POINT)
		{
			fread(&m_data.bones[i].boneIndexSize, m_data.boneIndex, 1, fp);
		}
		else
		{
			m_data.bones[i].boneIndexSize = -1;
			fread(&m_data.bones[i].posOffSet, 12, 1, fp);
		}

		if ((m_data.bones[i].flag & PmxData::IMPART_TRANSLATION) || (m_data.bones[i].flag & PmxData::IMPART_ROTATION))
		{
			fread(&m_data.bones[i].parentBoneIndexSize, m_data.boneIndex, 1, fp);
			fread(&m_data.bones[i].grantRate, 4, 1, fp);
		}

		if (m_data.bones[i].flag & PmxData::AXIS_FIXING)
			fread(&m_data.bones[i].axisvVector, 12, 1, fp);

		if (m_data.bones[i].flag & PmxData::LOCAL_AXIS) {
			fread(&m_data.bones[i].xAxisVector, 12, 1, fp);
			fread(&m_data.bones[i].zAxizVector, 12, 1, fp);
		}

		if (m_data.bones[i].flag & PmxData::EXTERNAL_PARENT_TRANS)
			fread(&m_data.bones[i].keyIndex, 4, 1, fp);

		if (m_data.bones[i].flag & PmxData::IK)
		{
			fread(&m_data.bones[i].ikBoneIndexSize, m_data.boneIndex, 1, fp);
			fread(&m_data.bones[i].numLoop, 4, 1, fp);
			fread(&m_data.bones[i].axizLimits, 4, 1, fp);
			fread(&m_data.numLinks, 4, 1, fp);
			m_data.bones[i].ikLinks.resize(m_data.numLinks);

			for (int j = 0; j < m_data.numLinks; j++)
			{
				fread(&m_data.bones[i].ikLinks[j].linkBoneIndexSize, m_data.boneIndex, 1, fp);
				fread(&m_data.bones[i].ikLinks[j].limitFlag, 1, 1, fp);

				if (m_data.bones[i].ikLinks[j].limitFlag) {
					fread(&m_data.bones[i].ikLinks[j].lowerLimit, 12, 1, fp);
					fread(&m_data.bones[i].ikLinks[j].higherLimit, 12, 1, fp);

				}
			}
		}
	}
	fclose(fp);
	m_data.materials.resize(m_data.material.size());
}

/**
 * @brief 使いやすいように変換する
 */
void PmxLoader::Map()
{
	//マテリアル情報コピー
	for (int i = 0; i < m_data.material.size(); i++)
	{
		m_data.materials[i].indicesNum = m_data.material[i].vertexNum;
		m_data.materials[i].material.diffuse = m_data.material[i].diffuse;
		m_data.materials[i].material.specular = m_data.material[i].specular;
		m_data.materials[i].material.ambient = m_data.material[i].ambient;
	}

	m_data.shaderData.resize(m_data.material.size());
	for (int i = 0; i < m_data.material.size(); i++)
	{
		m_data.shaderData[i].diffuse = m_data.material[i].diffuse;
		m_data.shaderData[i].specular = m_data.material[i].specular;
		m_data.shaderData[i].ambient = m_data.material[i].ambient;
	}

	//ボーン情報を使いやすいようにMap化
	std::vector<std::wstring> boneNames(m_data.bones.size());
	for (int i = 0; i < m_data.bones.size(); i++)
	{
		auto& pb = m_data.bones[i];
		boneNames[i] = pb.name;
		auto& node = m_data.boneNodeTable[pb.name];
		node.boneIndex = i;
		node.startPos = pb.pos;
	}

	for (auto& pb : m_data.bones)
	{
		if (pb.parentNo >= m_data.bones.size())
		{
			continue;
		}

		auto parentName = boneNames[pb.parentNo];
		m_data.boneNodeTable[parentName].children.emplace_back(
			&m_data.boneNodeTable[pb.name]
		);

	}
}

/**
	@brief	描画するための初期設定
*/
void PmxLoader::SetUp()
{

	m_resourceDescriptors = make_unique<DescriptorHeap>(DXTK->Device, 1);
	m_materialDescriptors = make_unique<DescriptorHeap>(DXTK->Device, m_data.numMaterial * 4);

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(m_data.vertices[0]) * m_data.numVertex;
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	VertexBuffer(heapprop, resdesc);
	IndexBuffer(heapprop, resdesc);
	ConstantBuffer(heapprop, resdesc);
	ExportTexture();
	InitShader();
	CreatePipeLine();
	//m_vmd.Initialize(m_data, m_constantBuffer,"Motion/IA_Conqueror_light_version.vmd");

}

/**
	@brief	頂点バッファー、ビューの生成
	@param	heapprop ヒーププロパティの構造体
	@param  resdesc  リソースディスクの構造体
*/
void PmxLoader::VertexBuffer(D3D12_HEAP_PROPERTIES heapprop, D3D12_RESOURCE_DESC resdesc)
{

	//VertexBufferの書き出し
	HRESULT result;
	result = DXTK->Device->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vertexBuffer.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(result);

	//
	void* map_addr = nullptr;
	m_vertexBuffer->Map(0, nullptr, &map_addr);
	CopyMemory(map_addr, m_data.vertices.data(), sizeof(m_data.vertices[0]) * m_data.numVertex);
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(m_data.vertices[0]) * m_data.numVertex;
	m_vertexBufferView.StrideInBytes = sizeof(m_data.vertices[0]);
}

/**
	@brief	インデックスバッファー、ビューの生成
	@param	heapprop ヒーププロパティの構造体
	@param  resdesc  リソースディスクの構造体
*/
void PmxLoader::IndexBuffer(D3D12_HEAP_PROPERTIES heapprop, D3D12_RESOURCE_DESC resdesc)
{
	//IndexBufferの生成
	HRESULT result;
	resdesc.Width = sizeof(m_data.surfaces[0]) * m_data.surfaces.size();
	result = DXTK->Device->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_indexBuffer.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(result);

	//IndexBufferの書き出し
	void* map_addr = nullptr;
	m_indexBuffer->Map(0, nullptr, &map_addr);
	CopyMemory(map_addr, m_data.surfaces.data(), sizeof(m_data.surfaces[0]) * m_data.surfaces.size());
	m_indexBuffer->Unmap(0, nullptr);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(m_data.surfaces[0]) * m_data.surfaces.size();
}

/**
	@brief	コンスタントバッファー、ビューの生成
	@param	heapprop ヒーププロパティの構造体
	@param  resdesc  リソースディスクの構造体
*/
void PmxLoader::ConstantBuffer(D3D12_HEAP_PROPERTIES heapprop, D3D12_RESOURCE_DESC resdesc)
{
	//ConstantBufferの生成
	HRESULT result;
	resdesc.Width = (sizeof(VSOUT) + 0xff) & ~0xff;
	result = DXTK->Device->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_constantBuffer.ReleaseAndGetAddressOf())
	);
	DX::ThrowIfFailed(result);


	auto materialSize = sizeof(PmxData::MaterialForHlsl);
	materialSize = (materialSize + 0xff) & ~0xff;
	//
	resdesc.Width = materialSize * m_data.numMaterial;
	result = DXTK->Device->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_materialBuffer.ReleaseAndGetAddressOf())
	);
	DX::ThrowIfFailed(result);

	//ConstantBufferViewの生成
	auto desc_addr = m_resourceDescriptors->GetCpuHandle(0);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{};
	cbv_desc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	cbv_desc.SizeInBytes = (UINT)m_constantBuffer->GetDesc().Width;
	DXTK->Device->CreateConstantBufferView(&cbv_desc, desc_addr);

	cbv_desc.BufferLocation = m_materialBuffer->GetGPUVirtualAddress();
	cbv_desc.SizeInBytes = (UINT)m_materialBuffer->GetDesc().Width;


	char* mapMaterial = nullptr;
	m_materialBuffer->Map(0, nullptr, (void**)&mapMaterial);
	for (int i = 0; i < m_data.numMaterial; ++i) {
		*((PmxData::MaterialForHlsl*)mapMaterial) = m_data.shaderData[i];
		mapMaterial += materialSize;
	}
	m_materialBuffer->Unmap(0, nullptr);

	for (int i = 0; i < m_data.numMaterial; i++) {
		desc_addr = m_materialDescriptors->GetCpuHandle(i * 4);

		DXTK->Device->CreateConstantBufferView(&cbv_desc, desc_addr);
	}

}

/**
	@brief	テクスチャデータを書き出す
*/
void PmxLoader::ExportTexture()
{
	ResourceUploadBatch resourceUpload(DXTK->Device);
	resourceUpload.Begin();

	//テクスチャ書き出し
	m_texture.resize(m_data.numMaterial);
	m_sphTexture.resize(m_data.numMaterial);
	m_toonTexture.resize(m_data.numMaterial);
	std::wstring textureData[256] = {};
	for (int i = 0; i < m_data.numTexture + 1; i++) {
		if (i == m_data.numTexture)
		{
			std::wstring textureName = L"Model/white.bmp";
			textureData[255] = textureName.c_str();
			break;
		}
		auto textureName = m_ps + m_data.texturePaths[i];
		textureData[i] = textureName.c_str();
	}

	std::wstring toonTextureData[10];
	for (int i = 1; i < 11; i++)
	{
		if (i == 10)
		{
			std::wstring n = L"Model/toon/toon10.bmp";
			toonTextureData[i-1] = n;
			break;
		}

		std::wstring textureName = L"Model/toon/toon0";

		textureName.append(std::to_wstring(i));
		textureName.append(L".bmp");

		toonTextureData[i-1] = textureName;
	}

	//テクスチャをデスクリプターヒープに書き出す
	for (int i = 0; i < m_data.numMaterial; i++) {

		auto textureName = textureData[m_data.material[i].colorMapTextureIndex];
		DX12::CreateTextureSRV(DXTK->Device, textureName.c_str(), resourceUpload, m_materialDescriptors.get(), i * 4 + 1, m_texture[i].ReleaseAndGetAddressOf());

		auto sphName = textureData[m_data.material[i].mapTextureIndex];
		DX12::CreateTextureSRV(DXTK->Device, sphName.c_str(), resourceUpload, m_materialDescriptors.get(), i * 4 + 2, m_sphTexture[i].ReleaseAndGetAddressOf());

		auto toonName = toonTextureData[m_data.material[i].toonTextureIndex];
		if (!m_data.material[i].toonFlag)
			toonName = textureData[m_data.material[i].toonTexture];

		DX12::CreateTextureSRV(DXTK->Device, toonName.c_str(), resourceUpload, m_materialDescriptors.get(), i * 4 + 3, m_toonTexture[i].ReleaseAndGetAddressOf());
	}


	auto uploadResourcesFinished = resourceUpload.End(DXTK->CommandQueue);
	uploadResourcesFinished.wait();
}

/**
	@brief	シェーダーの設定
*/
void PmxLoader::InitShader()
{

	HRESULT result;
	ComPtr<ID3DBlob> error_blob;
	result = D3DCompileFromFile(
		vsfileName,
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0,
		m_vsBlob.ReleaseAndGetAddressOf(),
		error_blob.ReleaseAndGetAddressOf()
	);
	DX::ThrowIfFailed(result);

	result = D3DCompileFromFile(
		psfileName,
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0,
		m_psBlob.ReleaseAndGetAddressOf(),
		error_blob.ReleaseAndGetAddressOf()
	);
	DX::ThrowIfFailed(result);
}

/**
	@brief	パイプライン設定
*/
void PmxLoader::CreatePipeLine()
{
	// ルートシグネチャ
	HRESULT result;
	D3D12_DESCRIPTOR_RANGE descRange[3] = {};
	descRange[0].NumDescriptors = 1;
	descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descRange[0].BaseShaderRegister = 0;
	descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descRange[1].NumDescriptors = 1;
	descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descRange[1].BaseShaderRegister = 1;
	descRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descRange[2].NumDescriptors = 3;
	descRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRange[2].BaseShaderRegister = 0;
	descRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	//ルートパラム
	D3D12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descRange[0];
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descRange[1];
	rootparam[1].DescriptorTable.NumDescriptorRanges = 2;
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//サンプラー
	D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[0].MinLOD = 0.0f;
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc[0].ShaderRegister = 0;

	samplerDesc[1] = samplerDesc[0];
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootparam;
	rootSignatureDesc.NumParameters = 2;
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

	result = DirectX::CreateRootSignature(
		DXTK->Device,
		&rootSignatureDesc,
		m_rootSignature.ReleaseAndGetAddressOf()
	);
	DX::ThrowIfFailed(result);

	// パイプラインステート
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr;
	gpipeline.VS.pShaderBytecode = m_vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = m_vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = m_psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = m_psBlob->GetBufferSize();
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.LogicOpEnable = false;
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DepthStencilState.StencilEnable = false;

	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL",0, DXGI_FORMAT_R32G32B32_FLOAT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BONENO",0, DXGI_FORMAT_R32_UINT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		/*{"BONENO1",0, DXGI_FORMAT_R32_UINT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BONENO2",0, DXGI_FORMAT_R32_UINT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BONENO3",0, DXGI_FORMAT_R32_UINT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"WEIGHT",0, DXGI_FORMAT_R32_FLOAT,0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},*/
	};
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;

	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	gpipeline.pRootSignature = m_rootSignature.Get();
	result = DXTK->Device->CreateGraphicsPipelineState(
		&gpipeline,
		IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf())
	);
	DX::ThrowIfFailed(result);
}

/**
	@brief	ファイル名のユニコード変換、余計な文字削除
	@param	_file   ファイルデータ
	@param  output  変換したファイル名
	@return 成功　失敗
*/
bool PmxLoader::getPMXStringUTF16(FILE* file, std::wstring& output)
{
	if (file == NULL)
		return false;

	std::array<wchar_t, 512> wBuffer{};
	int a;

	fread(&a, sizeof(a), 1, file);

	fread(&wBuffer, a, 1, file);

	output = std::wstring(wBuffer.data());
	wchar_t chars[] = L"\\";

	int i = output.find(chars);
	if (i != -1)
		output.replace(i, 1, L"/");

	return true;
}

void PmxLoader::ToonTexture()
{

}

std::string PmxLoader::GetExtension(const std::string& path)
{
	return std::string();
}

