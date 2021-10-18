/**
 * @file VMDLoader.cpp
 * @brief VMD読み込み
 * @author hoshi hirofumi
 * @date 2021/10/11
 */

/*
---------------------------------------------------------------------
	インクルード
---------------------------------------------------------------------
*/
#include "VMDLoader.h"
#include <array>
#include <codecvt>

// Initialize member variables.
VMDLoader::VMDLoader(): elapsedTime(),motionFPS(30)
{

}

/**
 * @brief VMDファイルを読み込む
 * @param name ファイル名
 * @return アニメーションデータ
*/
std::unordered_map<std::wstring, std::vector<VMDKeyFrame>> VMDLoader::LoadVMD(const char* name)
{
	FILE* fp;

	fopen_s(&fp, name, "rb");
	if (fp == nullptr) {
		OutputDebugString(TEXT("vmd file not find.\n"));
		DX::ThrowIfFailed(0x80070002);	// FileNotFoundException
	}
	fseek(fp, 50, SEEK_SET);

	unsigned int num_of_motion;
	fread(&num_of_motion,4,1,fp);
	data.resize(num_of_motion);

	for (auto& motion : data) {
		char c[15];
		fread(&c, 15, 1, fp);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, c, -1, motion.name, sizeof(motion.name));
		fread(&motion.frame_no, 96, 1, fp);

		motionData[motion.name].emplace_back(
			VMDKeyFrame(
				motion.frame_no, XMLoadFloat4(&motion.quaternion),
				XMFLOAT2(motion.bezier[3] / 127.0f, motion.bezier[7] / 127.0f),
				XMFLOAT2(motion.bezier[11] / 127.0f, motion.bezier[15] / 127.0f)
			)
		);

		maxFrame = motion.frame_no;
	}

	fclose(fp);

	// モーションデータ内の配列をフレーム番号順にソート
	for (auto& motion : motionData) {
		std::sort(motion.second.begin(), motion.second.end(),
			[](const VMDKeyFrame& lval, const VMDKeyFrame& rval) {
				return lval.frame_no <= rval.frame_no;
			});
	}

	return motionData;
}

/**
 * @brief VMDファイルを読み込む
 * @param name ファイル名
 * @return アニメーションデータ
*/
void VMDLoader::Initialize(PmxData data, ComPtr<ID3D12Resource> constantBuffer,const char* name)
{
	m_pmxData = data;
	m_constantBuffer = constantBuffer;
	boneMatrices.resize(data.bones.size());
	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());
	motionData = LoadVMD("Motion/IA_Conqueror_light_version.vmd");

	 //トランスフォーム行列
	m_constantBuffer->Map(0, nullptr, (void**)&constantBufferPrt);

	for (auto& bonemotion : motionData) {
		const auto& node = m_pmxData.boneNodeTable[bonemotion.first];
		const auto& pos = node.startPos;
		const auto  transform =
			XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* XMMatrixRotationQuaternion(bonemotion.second[0].quaternion)
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIndex] = transform;
	}
	MatrixMultiplyChildren(&m_pmxData.boneNodeTable[L"センター"], SimpleMath::Matrix::Identity);

	CopyMemory(constantBufferPrt + 1, boneMatrices.data(), sizeof(XMMATRIX) * boneMatrices.size());
}

void VMDLoader::Update(float deltaTime)
{
	UpdateBoneMatrices(deltaTime);
	auto& left_node = m_pmxData.boneNodeTable[L"右足"];
	const auto& left_pos = left_node.startPos;
	const auto  left_transform =
		XMMatrixTranslation(-left_pos.x, -left_pos.y, -left_pos.z)
		* XMMatrixRotationX(XMConvertToRadians(0))
		* XMMatrixTranslation(left_pos.x, left_pos.y, left_pos.z);
	boneMatrices[left_node.boneIndex] *= left_transform;

	m_constantBuffer->Map(0, nullptr, (void**)&constantBufferPrt);

	MatrixMultiplyChildren(&m_pmxData.boneNodeTable[L"センター"], SimpleMath::Matrix::Identity);
	CopyMemory(constantBufferPrt + 1, boneMatrices.data(), sizeof(XMMATRIX) * boneMatrices.size());
}

void VMDLoader::MatrixMultiplyChildren(PmxData::BoneNode* node, const XMMATRIX& matrix)
{
	boneMatrices[node->boneIndex] *= matrix;
	for (auto& children : node->children) {
		MatrixMultiplyChildren(children, boneMatrices[node->boneIndex]);
	}
}

void VMDLoader::UpdateBoneMatrices(const float deltaTime)
{
	elapsedTime += deltaTime;


	const float MAX_TIME = maxFrame / motionFPS;
	if (elapsedTime > MAX_TIME) {
		elapsedTime -= MAX_TIME;
	}

	const unsigned int FRAME_NO = static_cast<int>(motionFPS * elapsedTime);

	std::fill(boneMatrices.begin(), boneMatrices.end(), SimpleMath::Matrix::Identity);
	for (const auto& motion : motionData) {
		auto& node = m_pmxData.boneNodeTable[motion.first];

		const auto& keyframes = motion.second;
		auto rit = std::find_if(
			keyframes.rbegin(), keyframes.rend(),
			[FRAME_NO](const VMDKeyFrame& keyFrame)
			{
				return keyFrame.frame_no <= FRAME_NO;
			}
		);

		if (rit == keyframes.rend())
			continue;

		XMMATRIX rotation;
		auto it = rit.base();
		if (it != keyframes.end()) {
			auto t = static_cast<float>(FRAME_NO - rit->frame_no) /
				static_cast<float>(it->frame_no - rit->frame_no);
			t = GetYFromXOnBezier(t, it->p1, it->p2, 12);

			rotation = XMMatrixRotationQuaternion(
				XMQuaternionSlerp(rit->quaternion, it->quaternion, t)
			);
		}
		else {
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}

		const auto& pos = node.startPos;
		const auto  transform =
			XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIndex] = transform;
	}
}

float VMDLoader::GetYFromXOnBezier(const float x, const XMFLOAT2& a, const XMFLOAT2& b, const uint8_t n)
{
	if (a.x == a.y && b.x == b.y)
		return x;	//計算不要

	float t = x;
	const float k0 = 1 + 3 * a.x - 3 * b.x;	//t^3の係数
	const float k1 = 3 * b.x - 6 * a.x;		//t^2の係数
	const float k2 = 3 * a.x;				//t  の係数

	//誤差の範囲内かどうかに使用する定数
	constexpr float epsilon = 0.0005f;

	for (int i = 0; i < n; ++i) {
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;

		if (ft <= epsilon && ft >= -epsilon)
			break;

		t -= ft / 2;
	}

	const auto r = 1 - t;
	return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
}