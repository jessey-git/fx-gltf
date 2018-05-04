// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include "platform.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <wrl.h>
#include <shellapi.h>

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "DirectX/d3dx12.h"

#include "StringFormatter.h"
#include "Logger.h"

