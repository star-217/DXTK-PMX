#pragma once

namespace DX12 {
	struct SPRITE {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		D3D12_GPU_DESCRIPTOR_HANDLE            handle;
		DirectX::XMUINT2                       size;
	};

	inline DX12::SPRITE CreateSpriteSRV(
		ID3D12Device* device, const wchar_t* szFileName,
		DirectX::ResourceUploadBatch& resourceUpload,
		DirectX::DescriptorHeap* descripterHeap, const size_t index)
	{
		DX12::SPRITE sprite;

		DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(
			device, resourceUpload, szFileName,
			sprite.resource.GetAddressOf()
		));
		DirectX::CreateShaderResourceView(
			device, sprite.resource.Get(),
			descripterHeap->GetCpuHandle(index)
		);
		sprite.handle = descripterHeap->GetGpuHandle(index);
		sprite.size   = DirectX::GetTextureSize(sprite.resource.Get());

		return sprite;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE CreateTextureSRV(
		ID3D12Device* device, const wchar_t* szFileName,
		DirectX::ResourceUploadBatch& resourceUpload,
		DirectX::DescriptorHeap* descripterHeap, const size_t index,
		_Outptr_ ID3D12Resource** texture
	)
	{
		DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(device, resourceUpload, szFileName, texture));
		DirectX::CreateShaderResourceView(device, *texture, descripterHeap->GetCpuHandle(index));
		return descripterHeap->GetGpuHandle(index);
	}

	inline std::unique_ptr<DirectX::DescriptorHeap> CreateDescriptorHeap(
		ID3D12Device* device, size_t count
	)
	{ return std::make_unique<DirectX::DescriptorHeap>(device, count); }

	inline std::unique_ptr<DirectX::SpriteBatch> CreateSpriteBatch(
		ID3D12Device* device,
		DirectX::ResourceUploadBatch& upload,
		const DirectX::SpriteBatchPipelineStateDescription& psoDesc,
		const D3D12_VIEWPORT* viewport = nullptr
	)
	{ return std::make_unique<DirectX::SpriteBatch>(device, upload, psoDesc, viewport); }

	typedef std::unique_ptr<DirectX::DescriptorHeap> DESCRIPTORHEAP;
	typedef std::unique_ptr<DirectX::SpriteBatch>    SPRITEBATCH;
	typedef D3D12_GPU_DESCRIPTOR_HANDLE              HGPUDESCRIPTOR;
}

#include "Camera.h"
namespace DX12 {
	typedef DX9::Camera CAMERA;
}

namespace XAudio {
	inline std::unique_ptr<DirectX::SoundEffect> CreateSoundEffect(
		DirectX::AudioEngine* audioEngine, const wchar_t* waveFileName
	)
	{ return std::make_unique<DirectX::SoundEffect>(audioEngine, waveFileName); }

	typedef std::unique_ptr<DirectX::SoundEffect>         SOUNDEFFECT;
	typedef std::unique_ptr<DirectX::SoundEffectInstance> SOUNDEFFECTINSTANCE;
}

struct Rect : public RECT {
	Rect()
	{
		left   = 0;
		top    = 0;
		right  = 0;
		bottom = 0;
	}

	Rect(const int l, const int t, const int r, const int b)
	{
		left   = l;
		top    = t;
		right  = r;
		bottom = b;
	}

	Rect(const RECT& rect)
	{
		left   = rect.left;
		top    = rect.top;
		right  = rect.right;
		bottom = rect.bottom;
	}

	inline Rect& operator =(const RECT& rect)
	{
		left   = rect.left;
		top    = rect.top;
		right  = rect.right;
		bottom = rect.bottom;
		return *this;
	}

	inline bool Intersect(const RECT& rect)
	{
		if (right < rect.left || left   > rect.right ||
			top > rect.bottom || bottom < rect.top)
			return false;
		return true;
	}

	inline bool Intersect(const POINT& pt)
	{
		if (left > pt.x || top > pt.y || right < pt.x || bottom < pt.y)
			return false;
		return true;
	}
};

struct RectWH : public Rect {
	RectWH() : Rect()
	{
	}

	RectWH(const int l, const int t, const int w, const int h)
	{
		left   = l;
		top    = t;
		right  = left + w;
		bottom = top  + h;
	}
};
