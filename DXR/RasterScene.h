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
	void OnUpdate();
	void OnRender();
private:
	std::shared_ptr<Graphics> m_spGraphics;

	float color[4];
	bool isRAdd = true;
	bool isGAdd = true;
	bool isBAdd = true;
};

