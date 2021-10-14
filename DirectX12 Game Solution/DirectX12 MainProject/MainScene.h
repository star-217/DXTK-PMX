//
// MainScene.h
//

#pragma once

#include "Scene.h"
#include "tool/PmxLoader.h"


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

	void  MatrixMultiplyChildren(PmxData::BoneNode* node, const XMMATRIX& matrix);
	void  UpdateBoneMatrices(const float deltaTime);
	float GetYFromXOnBezier(const float x, const XMFLOAT2& a, const XMFLOAT2& b, const uint8_t n);

	float leftArmAngle;
	float elapsedTime;
	float motionFPS;

	PmxLoader reimu;
	PmxLoader back;

	VMDLoader vmdLoad;

	DX12::SPRITE chara;
	DX12::CAMERA mainCamera;

	DX9::MEDIARENDERER bgm;

};