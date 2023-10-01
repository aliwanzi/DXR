#pragma once
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <d3dcompiler.h>

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <iostream>
#include <system_error>
#include <cmath>

#include <Windows.h>
#include "d3dx12.h"
#include <wrl.h>
using namespace Microsoft::WRL;

const UINT FrameCount = 2;

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        std::string message = std::system_category().message(hr);
    }
}