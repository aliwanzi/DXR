#include "RasterScene.h"
#include "Geometry.h"

RasterScene::RasterScene() :m_spGraphics(std::make_shared<Graphics>())
{

}

RasterScene::~RasterScene()
{
	
}

void RasterScene::Init(HWND window, int width, int height)
{
	m_spGraphics->SetWindow(window, width, height);
	m_spGraphics->CreateResources();
	LoadAsset();
}

void RasterScene::Run()
{
	OnUpdate();
	OnRender();
}

void RasterScene::LoadAsset()
{
	std::vector<Vertex> vertexPostion;
	std::vector<UINT> vertexIndex;
	Geometry::CreateSphere(vertexPostion, vertexIndex);

	//Vertex vertexPostion[] =
	//{
	//	{ { 0.5, -0.5, 0.5 }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	//	{ { -0.5, -0.5, 0.5 }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	//	{ { 0.5, 0.5, 0.5}, { 0.0f, 0.0f, 1.0f, 1.0f } },
	//	{ { -0.5, 0.5, 0.5 }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	//};

	//UINT vertexIndex[] =
	//{
	//	0,1,3,0,3,2
	//};

	{
		const UINT vertexBufferSize = sizeof(Vertex) * vertexPostion.size();

		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		ThrowIfFailed(m_spGraphics->GetDevice()->CreateCommittedResource(
			&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDes, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pVertexBuffer.ReleaseAndGetAddressOf())));
		
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_pVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, vertexPostion.data(), vertexBufferSize);
		m_pVertexBuffer->Unmap(0, nullptr);

		vertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	{
		m_instanceNum = vertexIndex.size();
		const UINT indexBufferSize = sizeof(UINT) * m_instanceNum;

		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDes = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
		ThrowIfFailed(m_spGraphics->GetDevice()->CreateCommittedResource(
			&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDes, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pIndexBuffer.ReleaseAndGetAddressOf())));

		UINT8* pIndexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_pIndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, vertexIndex.data(), indexBufferSize);
		m_pIndexBuffer->Unmap(0, nullptr);

		indexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = indexBufferSize;
	}

}

void RasterScene::OnUpdate()
{
	if (color[0] <= 1.0f && isRAdd)
	{
		color[0] += 0.001f;
		isRAdd = true;
	}
	else
	{
		color[0] -= 0.002f;
		color[0] <= 0 ? isRAdd = true : isRAdd = false;

	}

	if (color[1] <= 1.0f && isGAdd)
	{
		color[1] += 0.002f;
		isGAdd = true;
	}
	else
	{
		color[1] -= 0.001f;
		color[1] <= 0 ? isGAdd = true : isGAdd = false;

	}

	if (color[2] <= 1.0f && isBAdd)
	{
		color[2] += 0.001f;
		isBAdd = true;
	}
	else
	{
		color[2] -= 0.001f;
		color[2] <= 0 ? isBAdd = true : isBAdd = false;

	}

	color[3] = 1.0f;
}

void RasterScene::OnRender()
{
	m_spGraphics->BeginFrame();

	auto commandList = m_spGraphics->GetCommandList();
	auto rtvHeap = m_spGraphics->GetRTVHeap();
	UINT frameIndex = m_spGraphics->GetFrameIndex();
	UINT descriptorSize = m_spGraphics->GetDescriptorSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, descriptorSize);
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	float clearColor[] = { color[0],color[1],color[2],color[3] };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(m_instanceNum, 1, 0, 0, 0);

	m_spGraphics->EndFrame();
}