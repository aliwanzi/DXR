#include "RasterScene.h"

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
}

void RasterScene::Run()
{
	OnUpdate();
	OnRender();
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
	float clearColor[] = { color[0],color[1],color[2],color[3] };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_spGraphics->EndFrame();
}