//
// MainScene.cpp
//

#include "Base/pch.h"
#include "Base/dxtk.h"
#include "SceneFactory.h"
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

// Initialize member variables.
MainScene::MainScene()
{

}

// Initialize a variable and audio resources.
void MainScene::Initialize()
{

	mainCamera.SetView(
		SimpleMath::Vector3(0, 10, -30),
		SimpleMath::Vector3(0, 0, 0)
	);
	mainCamera.SetPerspectiveFieldOfView(
		XMConvertToRadians(45.0f),
		16.0f / 9.0f,
		1.0f, 10000.0f
	);

	leftArmAngle = 0;
	elapsedTime = 0;
	motionFPS = 90;
}

// Allocate all memory the Direct3D and Direct2D resources.
void MainScene::LoadAssets()
{

	bgm = DX9::MediaRenderer::CreateFromFile(DXTK->Device9, L"conqurer.mp3");
	bgm->Play();

	reimu.PmxRead("Model/にがもん式霊夢/reimu.pmx");
	reimu.SetCamera(mainCamera);
	reimu.SetPosition(SimpleMath::Vector3(0, 0, 0));
	//back.PmxRead("Model/Stage_ST43/ST43.pmx");
	//data.boneMatrices.resize(data.bones.size());
	//std::fill(data.boneMatrices.begin(), data.boneMatrices.end(), XMMatrixIdentity());
	//data.motionData = vmdLoad.LoadVMD("Motion/IA_Conqueror_light_version.vmd");
	//data.maxFrame = vmdLoad.GetVMDFrame();


	// トランスフォーム行列
	//constantBuffer->Map(0, nullptr, (void**)&constantBufferPrt);
	//motionData = data.motionData;

	//for (auto& bonemotion : motionData) {
	//	const auto& node = data.boneNodeTable[bonemotion.first];
	//	const auto& pos = node.startPos;
	//	const auto  transform =
	//		XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
	//		* XMMatrixRotationQuaternion(bonemotion.second[0].quaternion)
	//		* XMMatrixTranslation(pos.x, pos.y, pos.z);
	//	data.boneMatrices[node.boneIndex] = transform;
	//}
	//MatrixMultiplyChildren(&data.boneNodeTable[L"センター"], SimpleMath::Matrix::Identity);

	//CopyMemory(constantBufferPrt + 1, data.boneMatrices.data(), sizeof(XMMATRIX) * data.boneMatrices.size());
}

// Releasing resources required for termination.
void MainScene::Terminate()
{
	//constantBuffer->Unmap(0, nullptr);

	DXTK->ResetAudioEngine();
	DXTK->WaitForGpu();

	// TODO: Add your Termination logic here.

}

// Direct3D resource cleanup.
void MainScene::OnDeviceLost()
{

}

// Restart any looped sounds here
void MainScene::OnRestartSound()
{

}

// Updates the scene.
NextScene MainScene::Update(const float deltaTime)
{


	//if (DXTK->KeyState->Z)
	//	rotation.x += 1;
	//if (DXTK->KeyState->X)
	//	rotation.y += 2;
	//if (DXTK->KeyState->C)
	//	rotation.z += 1;



	/*UpdateBoneMatrices(deltaTime);
	auto& left_node = data.boneNodeTable[L"右足"];
	const auto& left_pos = left_node.startPos;
	const auto  left_transform =
		XMMatrixTranslation(-left_pos.x, -left_pos.y, -left_pos.z)
		* XMMatrixRotationX(XMConvertToRadians(leftArmAngle * deltaTime))
		* XMMatrixTranslation(left_pos.x, left_pos.y, left_pos.z);
	data.boneMatrices[left_node.boneIndex] *= left_transform;

	constantBuffer->Map(0, nullptr, (void**)&constantBufferPrt);

	MatrixMultiplyChildren(&data.boneNodeTable[L"センター"], SimpleMath::Matrix::Identity);
	CopyMemory(constantBufferPrt + 1, data.boneMatrices.data(), sizeof(XMMATRIX) * data.boneMatrices.size());*/

	return NextScene::Continue;
}

// Draws the scene.
void MainScene::Render()
{
	DXTK->ResetCommand();
	DXTK->ClearRenderTarget(Colors::White);

	reimu.Render();
	//back.Render();

	DXTK->ExecuteCommandList();
}

void MainScene::MatrixMultiplyChildren(PmxData::BoneNode* node, const XMMATRIX& matrix)
{
	/*data.boneMatrices[node->boneIndex] *= matrix;
	for (auto& children : node->children) {
		MatrixMultiplyChildren(children, data.boneMatrices[node->boneIndex]);
	}*/
}

void MainScene::UpdateBoneMatrices(const float deltaTime)
{
	/*elapsedTime += deltaTime;


	const float MAX_TIME = data.maxFrame / motionFPS;
	if (elapsedTime > MAX_TIME) {
		elapsedTime -= MAX_TIME;
	}

	const unsigned int FRAME_NO = static_cast<int>(motionFPS * elapsedTime);

	std::fill(data.boneMatrices.begin(), data.boneMatrices.end(), SimpleMath::Matrix::Identity);*/
	/*for (const auto& motion : motionData) {
		auto& node =data.boneNodeTable[motion.first];

		const auto& keyframes = motion.second;
		auto rit = std::find_if(
			keyframes.rbegin(), keyframes.rend(),
			[FRAME_NO](const VMDKeyFrame& keyFrame)
			{
				return keyFrame.frame_no <= FRAME_NO;
			}
		);*/

		/*if (rit == keyframes.rend())
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
		data.boneMatrices[node.boneIndex] = transform;
	}*/
}

float MainScene::GetYFromXOnBezier(const float x, const XMFLOAT2& a, const XMFLOAT2& b, const uint8_t n)
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