//
// TemplateScene.h
//

#pragma once

#include "Base/pch.h"
#include "Base/dxtk.h"

using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::make_unique;
using namespace DirectX;

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

private:

	unsigned int maxFrame;
	std::vector<VMDData> data;
	std::unordered_map<std::wstring, std::vector<VMDKeyFrame>> motionData;

};