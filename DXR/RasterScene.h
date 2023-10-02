#pragma once
#include "Graphics.h"

class RasterScene
{
public:
	RasterScene();
	~RasterScene();
	void Init(HWND window, int width, int height);
	void Run();
private:
	void LoadAsset();
	void OnUpdate();
	void OnRender();
private:
	std::shared_ptr<Graphics> m_spGraphics;
	ComPtr<ID3D12Resource> m_pVertexBuffer;
	ComPtr<ID3D12Resource> m_pIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	float color[4];
	bool isRAdd = true;
	bool isGAdd = true;
	bool isBAdd = true;

	UINT m_instanceNum = 0;
};

