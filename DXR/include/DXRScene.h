#pragma once
#include "Graphics.h"
#include "Common.h"
#include "Utils.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

class DXRScene
{
public:
	DXRScene(ConfigInfo& config);
	~DXRScene() = default;
	void Update();
	void Render();
	void CleanUp();

private:
	HWND window;
	Model model;
	Material material;

	DXRGlobal dxr = {};
	D3D12Global d3d = {};
	D3D12Resources resources = {};
	D3D12ShaderCompilerInfo shaderCompiler;
};

