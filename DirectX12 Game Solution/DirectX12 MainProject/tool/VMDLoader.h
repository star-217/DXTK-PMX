//
// TemplateScene.h
//

#pragma once

#include "PmxStructList.h"

using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::make_unique;
using namespace DirectX;


class VMDLoader{
public:
	VMDLoader();
	virtual ~VMDLoader() { }

	VMDLoader(VMDLoader&&) = default;
	VMDLoader& operator= (VMDLoader&&) = default;

	VMDLoader(VMDLoader const&) = delete;
	VMDLoader& operator= (VMDLoader const&) = delete;

	std::unordered_map<std::wstring, std::vector<VMDKeyFrame>> LoadVMD(const char*);
	std::unordered_map<std::wstring, std::vector<VMDKeyFrame>> GetData() { return motionData; }
	unsigned int GetVMDFrame() { return maxFrame; }
	void Animetion();
	void Initialize();
	void Update();

private:

	void  MatrixMultiplyChildren(PmxData::BoneNode* node, const XMMATRIX& matrix);
	void  UpdateBoneMatrices(const float deltaTime);
	float GetYFromXOnBezier(const float x, const XMFLOAT2& a, const XMFLOAT2& b, const uint8_t n);

	unsigned int maxFrame;
	std::vector<VMDData> data;
	std::unordered_map<std::wstring, std::vector<VMDKeyFrame>> motionData;

};