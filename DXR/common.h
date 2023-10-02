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

struct SceneConstantBuffer
{
	XMFLOAT4X4 MVP;
};

template <typename T>
constexpr UINT CalcConstantBufferByteSize()
{
	// Constant buffers must be a multiple of the minimum hardware
	// allocation size (usually 256 bytes).  So round up to nearest
	// multiple of 256.  We do this by adding 255 and then masking off
	// the lower 2 bytes which store all bits < 256.
	// Example: Suppose byteSize = 300.
	// (300 + 255) & ~255
	// 555 & ~255
	// 0x022B & ~0x00ff
	// 0x022B & 0xff00
	// 0x0200
	// 512
	UINT byteSize = sizeof(T);
	return (byteSize + 255) & ~255;
}