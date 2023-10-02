#pragma once

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

#include <Windows.h>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace DirectX;
const UINT FrameCount = 2;

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        std::string message = std::system_category().message(hr);
    }
}

struct Vertex
{
	XMFLOAT3 position;
    XMFLOAT4 color;
};