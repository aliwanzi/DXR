#pragma once
#include "common.h"
#include <dxgi1_6.h>

class Graphics
{
public:
	Graphics();
	~Graphics();
	void CreateResources();
	void SetWindow(HWND window, int width, int height);

	void BeginFrame();
	void EndFrame();
	
	ComPtr<ID3D12GraphicsCommandList> GetCommandList()const;
	ComPtr<ID3D12DescriptorHeap> GetRTVHeap()const;
	UINT GetFrameIndex()const;
	UINT GetDescriptorSize()const;

private:
	void WaitForPreviousFrame();

protected:
	ComPtr<IDXGIFactory4> m_pDXGIFactory;
	ComPtr<ID3D12Device> m_pDevice;

	ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

	ComPtr<IDXGISwapChain3> m_pSwapChain;

	ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
	UINT m_RTVDescriptorSize;

	ComPtr<ID3D12Resource> m_pRenderTargets[FrameCount];

	ComPtr<ID3D12PipelineState> m_pPipelineState;

	UINT m_frameIndex;
	UINT64 m_fenceValue;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_pFence;

	HWND m_Window;
	RECT m_OutputSize;
};