/**
 * @file VMDLoader.h
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
VMDLoader::VMDLoader()
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
