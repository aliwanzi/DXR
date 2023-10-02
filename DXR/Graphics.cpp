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
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_pDSVHeap.ReleaseAndGetAddressOf())));
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_pCBVHeap.ReleaseAndGetAddressOf())));
	}

	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;
		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC tex2D = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT, m_viewPort.Width, m_viewPort.Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties, D3D12_HEAP_FLAG_NONE, &tex2D, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, IID_PPV_ARGS(m_pDepthStencilBuffer.ReleaseAndGetAddressOf())));
		m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &depthStencilDesc, m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
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
	
	LoadGraphicsAsset();
}

void Graphics::SetWindow(HWND window, int width, int height)
{
	m_Window = window;
	m_OutputSize.left = m_OutputSize.top = 0;
	m_OutputSize.right = width;
	m_OutputSize.bottom = height;

	UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_OutputSize.right - m_OutputSize.left), 1u);
	UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_OutputSize.bottom - m_OutputSize.top), 1u);
	m_viewPort = CD3DX12_VIEWPORT(0.f, 0.f, backBufferWidth, backBufferHeight);
	m_scissorRect = CD3DX12_RECT(0, 0, backBufferWidth, backBufferHeight);
}

void Graphics::BeginFrame()
{
	ThrowIfFailed(m_pCommandAllocator->Reset());
	ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocator.Get(), m_pPipelineState.Get()));

	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());
	m_pCommandList->RSSetViewports(1, &m_viewPort);
	m_pCommandList->RSSetScissorRects(1, &m_scissorRect);

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

ComPtr<ID3D12CommandAllocator> Graphics::GetCommandAllocator() const
{
	return ComPtr<ID3D12CommandAllocator>();
}

ComPtr<ID3D12DescriptorHeap> Graphics::GetRTVHeap() const
{
	return m_pRTVHeap;
}

ComPtr<ID3D12DescriptorHeap> Graphics::GetDSVHeap() const
{
	return m_pDSVHeap;
}

ComPtr<ID3D12DescriptorHeap> Graphics::GetCBVHeap() const
{
	return m_pCBVHeap;
}

ComPtr<ID3D12Device> Graphics::GetDevice() const
{
	return m_pDevice;
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

void Graphics::LoadGraphicsAsset()
{
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		if (FAILED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC  rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(
			&rootSignatureDesc, featureData.HighestVersion, signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()));
		ThrowIfFailed(m_pDevice->CreateRootSignature(
			0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pRootSignature.ReleaseAndGetAddressOf())));
	}

	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ThrowIfFailed(D3DCompileFromFile(L"./Asset//shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, m_pVertexShader.ReleaseAndGetAddressOf(), nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"./Asset//shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, m_pPixelShader.ReleaseAndGetAddressOf(), nullptr));
	}

	{
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs,_countof(inputElementDescs) };
		psoDesc.pRootSignature = m_pRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_pVertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pPixelShader.Get());

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(m_pPipelineState.ReleaseAndGetAddressOf())));
	}

	{
		ThrowIfFailed(m_pDevice->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), m_pPipelineState.Get(), IID_PPV_ARGS(m_pCommandList.ReleaseAndGetAddressOf())));
		ThrowIfFailed(m_pCommandList->Close());
	}
}
