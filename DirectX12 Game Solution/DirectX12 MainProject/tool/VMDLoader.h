/**
 * @file VMDLoader.h
 * @brief VMDì«Ç›çûÇ›
 * @author hoshi hirofumi
 * @date 2021/10/11
 */

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
	void Initialize(PmxData data, ComPtr<ID3D12Resource> constantBuffer, const char* name);
	void Update(float deltaTime);

private:

	void  MatrixMultiplyChildren(PmxData::BoneNode* node, const XMMATRIX& matrix);
	void  UpdateBoneMatrices(const float deltaTime);
	float GetYFromXOnBezier(const float x, const XMFLOAT2& a, const XMFLOAT2& b, const uint8_t n);

	unsigned int maxFrame;
	float elapsedTime;
	float motionFPS;

	std::vector<VMDData> data;
	std::unordered_map<std::wstring, std::vector<VMDKeyFrame>> motionData;

	PmxData m_pmxData;
	ComPtr<ID3D12Resource> m_constantBuffer;
	XMMATRIX* constantBufferPrt;
	std::vector<SimpleMath::Matrix> boneMatrices;

};