// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include "Platform/platform.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <d3d12.h>
#include <dxgi1_5.h>
#include <shellapi.h>
#include <wrl.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "DirectX/d3dx12.h"

#include "Logger.h"
#include "StringFormatter.h"

