#include "Graphics.h"
namespace
{
	ComPtr<IDXGIAdapter1> GetSupportAdapter(ComPtr<IDXGIFactory4>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel)
	{
		ComPtr<IDXGIAdapter1> adapter;
		for (uint32_t adapterIndex = 0; ;++adapterIndex)
		{
			ComPtr<IDXGIAdapter1> currentAdapter;
			if (DXGI_ERROR_NOT_FOUND == 
				dxgiFactory->EnumAdapters1(adapterIndex,currentAdapter.ReleaseAndGetAddressOf()))
			{
				break;
			}
			const HRESULT hres = D3D12CreateDevice(currentAdapter.Get(), featureLevel, _uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hres))
			{
				adapter = currentAdapter;
				break;
			}
			currentAdapter->Release();
		}
		return adapter;
	}
}

Graphics::Graphics()
{
}

Graphics::~Graphics()
{
	WaitForPreviousFrame();
	CloseHandle(m_fenceEvent);
}

void Graphics::CreateResources()
{
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif 
	ThrowIfFailed(
		CreateDXGIFactory2(0, IID_PPV_ARGS(m_pDXGIFactory.ReleaseAndGetAddressOf())));

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	ComPtr<IDXGIAdapter1> adapter;
	for (uint32_t i = 0u; i < _countof(featureLevels); ++i)
	{
		adapter = GetSupportAdapter(m_pDXGIFactory, featureLevels[i]);
		if (adapter != nullptr)
		{
			break;
		}
	}

	if (adapter != nullptr)
	{
		ThrowIfFailed(D3D12CreateDevice(
			adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_pDevice.ReleaseAndGetAddressOf())));
	}

	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(m_pDevice->CreateCommandQueue(
			&queueDesc, IID_PPV_ARGS(m_pCommandQueue.ReleaseAndGetAddressOf())));
		ThrowIfFailed(m_pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pCommandAllocator.ReleaseAndGetAddressOf())));
		ThrowIfFailed(m_pDevice->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_pCommandList.ReleaseAndGetAddressOf())));
		ThrowIfFailed(m_pCommandList->Close());
	}

	{
		ThrowIfFailed(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.ReleaseAndGetAddressOf())));
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	{
		UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_OutputSize.right - m_OutputSize.left), 1u);
		UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_OutputSize.bottom - m_OutputSize.top), 1u);

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = backBufferWidth;
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChian1;
		ThrowIfFailed(m_pDXGIFactory->CreateSwapChainForHwnd(
			m_pCommandQueue.Get(), m_Window, &swapChainDesc, nullptr, nullptr, swapChian1.ReleaseAndGetAddressOf()));
		ThrowIfFailed(swapChian1.As(&m_pSwapChain));
		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(
			&rtvHeapDesc, IID_PPV_ARGS(m_pRTVHeap.ReleaseAndGetAddressOf())));
		m_RTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; ++n)
		{
			ThrowIfFailed(m_pSwapChain->GetBuffer(
				n, IID_PPV_ARGS(m_pRenderTargets[n].ReleaseAndGetAddressOf())));
			m_pDevice->CreateRenderTargetView(m_pRenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RTVDescriptorSize);
		}
	}
}

void Graphics::SetWindow(HWND window, int width, int height)
{
	m_Window = window;
	m_OutputSize.left = m_OutputSize.top = 0;
	m_OutputSize.right = width;
	m_OutputSize.bottom = height;
}

void Graphics::BeginFrame()
{
	ThrowIfFailed(m_pCommandAllocator->Reset());
	ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocator.Get(), m_pPipelineState.Get()));

	D3D12_RESOURCE_BARRIER resBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_pCommandList->ResourceBarrier(1, &resBarrier);
}

void Graphics::EndFrame()
{
	D3D12_RESOURCE_BARRIER resBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_pCommandList->ResourceBarrier(1, &resBarrier);
	ThrowIfFailed(m_pCommandList->Close());

	ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(m_pSwapChain->Present(1, 0));
	WaitForPreviousFrame();
}

ComPtr<ID3D12GraphicsCommandList> Graphics::GetCommandList() const
{
	return m_pCommandList;
}

ComPtr<ID3D12DescriptorHeap> Graphics::GetRTVHeap() const
{
	return m_pRTVHeap;
}

UINT Graphics::GetFrameIndex() const
{
	return m_frameIndex;
}

UINT Graphics::GetDescriptorSize() const
{
	return m_RTVDescriptorSize;
}

void Graphics::WaitForPreviousFrame()
{
	const UINT64 tempFenceValue = m_fenceValue;
	ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), tempFenceValue));
	m_fenceValue++;

	if (m_pFence->GetCompletedValue() < tempFenceValue)
	{
		ThrowIfFailed(m_pFence->SetEventOnCompletion(tempFenceValue, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}