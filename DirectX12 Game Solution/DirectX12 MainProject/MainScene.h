//
// MainScene.h
//

#pragma once

#include "Scene.h"
#include "tool/PmxLoader.h"
#include "tool/VMDLoader.h"


using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::make_unique;
using namespace DirectX;

class MainScene final : public Scene {
public:
	MainScene();
	virtual ~MainScene() { Terminate(); }

	MainScene(MainScene&&) = default;
	MainScene& operator= (MainScene&&) = default;

	MainScene(MainScene const&) = delete;
	MainScene& operator= (MainScene const&) = delete;

	// These are the functions you will implement.
	void Initialize() override;
	void LoadAssets() override;

	void Terminate() override;

	void OnDeviceLost() override;
	void OnRestartSound() override;

	NextScene Update(const float deltaTime) override;
	void Render() override;

private:

	float leftArmAngle;
	float elapsedTime;
	float motionFPS;

	PmxLoader reimu;
	PmxLoader back;

	VMDLoader vmdLoad;

	DX12::SPRITE chara;
	DX12::CAMERA mainCamera;

	DX9::MEDIARENDERER bgm;
	Vector3 rote;
	Vector3 pos;
};