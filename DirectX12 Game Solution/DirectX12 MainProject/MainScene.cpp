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

	reimu.PmxRead("Model/Eunice211107NewM/Eunice13EL_Mf.pmx");
	reimu.SetRotation(Vector3(0, 0, 0));
	reimu.SetCamera(mainCamera);
	back.SetShader(L"Shaders/VS2.hlsl", L"Shaders/PS2.hlsl");
	back.PmxRead("Model/Stage_ST43/ST43.pmx");
	back.SetCamera(mainCamera);

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
	//reimu.Animetion(deltaTime);

	return NextScene::Continue;
}

// Draws the scene.
void MainScene::Render()
{
	DXTK->ResetCommand();
	DXTK->ClearRenderTarget(Colors::White);

	reimu.Render();
	back.Render();

	DXTK->ExecuteCommandList();
}

