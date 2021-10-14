//
// GameBase.h
//

#pragma once

#include "GameBase.h"

class dxtk final
{
public:
	~dxtk() = default;

	dxtk(dxtk&&) = default;
	dxtk& operator= (dxtk&&) = default;

	dxtk(dxtk const&) = delete;
	dxtk& operator= (dxtk const&) = delete;

	static dxtk* GetInstance()
	{
		static dxtk instance;
		return &instance;
	}

	inline UINT GetBackBufferIndex() const { return m_gameBase->m_backBufferIndex; }

	inline void SetFixedFrameRate(const UINT fps)
	{
		m_gameBase->m_timer.SetFixedTimeStep(true);
		m_gameBase->m_timer.SetTargetElapsedSeconds(1.0 / fps);
	}

	inline void SetVariableFrameRate() { m_gameBase->m_timer.SetFixedTimeStep(false); }
	inline void WaitForGpu() const { m_gameBase->WaitForGpu(); }

#if DXTK_MULTIGPU
	inline void Clear(const UINT adapter, const DirectX::XMVECTORF32& color)
	{ m_gameBase->Clear(adapter, color); }

	inline void ExecuteCommandList(const UINT adapter) const
	{ m_gameBase->ExecuteCommandList(adapter); }

	inline void ExecuteCommandLists(const UINT adapter, UINT NumCommandLists, ID3D12GraphicsCommandList* const* ppCommandList) const
	{ m_gameBase->ExecuteCommandLists(adapter, NumCommandLists, ppCommandList); }

	inline ID3D12Resource* PopulateCopyCommandList() { return m_gameBase->PopulateCopyCommandList(); }
	inline void ExecuteCopyCommandList() { m_gameBase->ExecuteCopyCommandList(); }

	inline void CommandQueueSignal(ID3D12CommandQueue* queue) { m_gameBase->CommandQueueSignal(queue); }
	inline void CommandQueueWait(ID3D12CommandQueue* queue) { m_gameBase->CommandQueueWait(queue); }

	inline void CrossAdapterQueueSignal() { m_gameBase->CrossAdapterQueueSignal(); }
	inline void CrossAdapterQueueWait() { m_gameBase->CrossAdapterQueueWait(); }

	inline void CreateCommandAllocator(const UINT adapter, ID3D12CommandAllocator** commandAllocator)
	{
		DX::ThrowIfFailed(m_gameBase->m_devices[adapter].Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator)
		));
	}

	inline void CreateCommandList(
		const UINT adapter,
		ID3D12CommandAllocator* commandAllocator,
		ID3D12GraphicsCommandList** commandList)
	{
		DX::ThrowIfFailed(m_gameBase->m_devices[adapter].Device->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(commandList)
		));
		DX::ThrowIfFailed((*commandList)->Close());
	}
#else
	inline void ResetCommand() const
	{
		m_gameBase->ResetCommand(PrimaryAdapter);
	}

	inline void ClearRenderTarget(const DirectX::XMVECTORF32& color) const
	{
		m_gameBase->ClearRenderTarget(PrimaryAdapter, color);
	}

	inline void ExecuteCommandList() const
	{
		m_gameBase->ExecuteCommandList(PrimaryAdapter);
	}

	inline void ExecuteCommandLists(UINT NumCommandLists, ID3D12GraphicsCommandList* const* ppCommandList) const
	{
		m_gameBase->ExecuteCommandLists(PrimaryAdapter, NumCommandLists, ppCommandList);
	}

	inline void CreateCommandAllocator(ID3D12CommandAllocator** commandAllocator)
	{
		DX::ThrowIfFailed(m_gameBase->m_devices[PrimaryAdapter].Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator)
		));
	}

	inline void CreateCommandList(
		ID3D12CommandAllocator* commandAllocator,
		ID3D12GraphicsCommandList** commandList)
	{
		DX::ThrowIfFailed(m_gameBase->m_devices[PrimaryAdapter].Device->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(commandList)
		));
		DX::ThrowIfFailed((*commandList)->Close());
	}
#endif

#if DXTK_D3D11 || DXTK_DWRITE
	inline void CreateD3D11Device() const
	{ m_gameBase->D3D11CreateDevice(); }

	inline void Direct2DBegin()
	{
		m_gameBase->m_d3d11on12Device->AcquireWrappedResources(
			m_gameBase->m_wrappedBackBuffers[m_gameBase->m_backBufferIndex].GetAddressOf(), 1
		);
		m_gameBase->m_d2dDeviceContext->SetTarget(
			m_gameBase->m_d2dRenderTargets[m_gameBase->m_backBufferIndex].Get()
		);
		m_gameBase->m_d2dDeviceContext->BeginDraw();
	}

	inline void Direct2DEnd()
	{
		m_gameBase->m_d2dDeviceContext->EndDraw();
		m_gameBase->m_d3d11on12Device->ReleaseWrappedResources(
			m_gameBase->m_wrappedBackBuffers[m_gameBase->m_backBufferIndex].GetAddressOf(), 1
		);
		m_gameBase->m_d3d11DeviceContext->Flush();
	}
#endif

#if DXTK_D3D9
	inline void CreateD3D9Device() const { m_gameBase->D3D9CreateDevice();  }
#endif

	inline void ResetAudioEngine()
	{
		m_gameBase->m_audioEngine->Reset();
		m_gameBase->m_retryAudio = false;
	}

	__declspec(property(get = GetBackBufferIndex)) UINT BackBufferIndex;

public:
	int                                      ScreenWidth           = static_cast<int>(screen::width);
	int                                      ScreenHeight          = static_cast<int>(screen::height);

	UINT                                     BackBufferCount       = GameBase::c_swapBufferCount;

#if DXTK_MULTIGPU
	ID3D12Device*                            PrimaryDevice         = nullptr;
	ID3D12CommandQueue*                      PrimaryCommandQueue   = nullptr;
	ID3D12GraphicsCommandList*               PrimaryCommandList    = nullptr;

	ID3D12Device*                            SecondaryDevice       = nullptr;
	ID3D12CommandQueue*                      SecondaryCommandQueue = nullptr;
	ID3D12GraphicsCommandList*               SecondaryCommandList  = nullptr;
#else
	ID3D12Device*                            Device                = nullptr;
	ID3D12CommandQueue*                      CommandQueue          = nullptr;
	ID3D12GraphicsCommandList*               CommandList           = nullptr;

	ID3D12Device*                            ComputeDevice         = nullptr;
	ID3D12CommandQueue*                      ComputeCommandQueue   = nullptr;
	ID3D12GraphicsCommandList*               ComputeCommandList    = nullptr;
#endif
#if DXTK_D3D11 || DXTK_DWRITE
	ID3D11Device*                            Device11              = nullptr;
	ID2D1DeviceContext2*                     Direct2D              = nullptr;
	IDWriteFactory*                          DirectWrite           = nullptr;
#endif
#if DXTK_D3D9
	IDirect3DDevice9*                        Device9               = nullptr;

	#include "Direct3D9.inc"
	direct3d9*                               Direct3D9             = nullptr;
#endif

#if DXTK_KEYBOARD
	DirectX::Keyboard::State*                KeyState              = nullptr;
	DirectX::Keyboard::KeyboardStateTracker* KeyEvent              = nullptr;
#endif
#if DXTK_MOUSE
	DirectX::Mouse::State*                   MouseState            = nullptr;
	DirectX::Mouse::ButtonStateTracker*      MouseEvent            = nullptr;

	inline DirectX::XMINT2 GetMousePosition() const
	{
		DirectX::XMINT2 pos;
		GetCursorPos((POINT*)&pos);
		ScreenToClient(m_gameBase->m_window, (POINT*)&pos);
		return pos;
	}
	__declspec(property(get = GetMousePosition)) DirectX::XMINT2 MousePosition;
#endif
#if DXTK_GAMEPAD
	DirectX::GamePad::State*                 GamePadState          = nullptr;
	DirectX::GamePad::ButtonStateTracker*    GamePadEvent          = nullptr;
	inline bool GamePadVibration(
		const int player, const float leftMotor, const float rightMotor,
		const float leftTrigger = 0.0f, const float rightTrigger = 0.0f
	) const
	{ return m_gameBase->m_gamePad->SetVibration(player, leftMotor, rightMotor, leftTrigger, rightTrigger); }
#endif

	DirectX::AudioEngine*                    AudioEngine           = nullptr;

#if DXTK_AMP
	std::unique_ptr<concurrency::accelerator_view> PrimaryAMP;
	std::unique_ptr<concurrency::accelerator_view> SecondaryAMP;
#endif

private:
	friend class GameBase;

	dxtk() = default;

	inline void Attach(GameBase* const __restrict gameBase) noexcept
	{
		m_gameBase            = gameBase;

#if DXTK_MULTIGPU
		PrimaryDevice         = gameBase->m_devices[0].Device.Get();
		PrimaryCommandQueue   = gameBase->m_devices[0].CommandQueue.Get();
		PrimaryCommandList    = gameBase->m_devices[0].CommandList.Get();

		SecondaryDevice       = gameBase->m_devices[1].Device.Get();
		SecondaryCommandQueue = gameBase->m_devices[1].CommandQueue.Get();
		SecondaryCommandList  = gameBase->m_devices[1].CommandList.Get();
#else
		Device                = gameBase->m_devices[0].Device.Get();
		CommandQueue          = gameBase->m_devices[0].CommandQueue.Get();
		CommandList           = gameBase->m_devices[0].CommandList.Get();

		ComputeDevice         = gameBase->m_devices[1].Device.Get();
		ComputeCommandQueue   = gameBase->m_devices[1].CommandQueue.Get();
		ComputeCommandList    = gameBase->m_devices[1].CommandList.Get();
#endif
#if DXTK_D3D11 || DXTK_DWRITE
		Device11              = gameBase->m_d3d11Device.Get();
		Direct2D              = gameBase->m_d2dDeviceContext.Get();
		DirectWrite           = gameBase->m_dWriteFactory.Get();
#endif
#if DXTK_D3D9
		Device9               = gameBase->m_d3d9Device.Get();
		Direct3D9             =&m_d3d9;
		Direct3D9->Attach(m_gameBase);
		DX9::SpriteBatch      = std::make_unique<DX9::SpriteBatch9>(Device9);
#endif

#if DXTK_KEYBOARD
		KeyState              =&m_keyState;
		KeyEvent              =&gameBase->m_keys;
#endif
#if DXTK_MOUSE
		MouseState            =&m_mouseState;
		MouseEvent            =&gameBase->m_mouseButtons;
#endif
#if DXTK_GAMEPAD
		GamePadState          = m_gamePadState;
		GamePadEvent          = gameBase->m_gamepadButtons;
#endif

		AudioEngine           = gameBase->m_audioEngine.get();

#if DXTK_AMP
		PrimaryAMP   = std::make_unique<concurrency::accelerator_view>(m_gameBase->m_primaryAccelerator  ->get_default_view());
		SecondaryAMP = std::make_unique<concurrency::accelerator_view>(m_gameBase->m_secondaryAccelerator->get_default_view());
#endif
	}

	GameBase*                m_gameBase = nullptr;

#if DXTK_D3D9
	direct3d9                m_d3d9;
#endif

#if DXTK_KEYBOARD
	DirectX::Keyboard::State m_keyState{};
#endif
#if DXTK_MOUSE
	DirectX::Mouse::State    m_mouseState{};
#endif
#if DXTK_GAMEPAD
	DirectX::GamePad::State  m_gamePadState[DirectX::GamePad::MAX_PLAYER_COUNT]{};
#endif
};

extern dxtk* DXTK;
