// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <string>

struct EngineOptions
{
    int Width{ 960 };
    int Height{ 540 };

    bool AutoRotate{};
    bool EnableMaterials{};
    bool EnableIBL{};
    bool EnableGround{};

    float CameraX{ 0 };
    float CameraY{ 0 };
    float CameraZ{ 8.0f };

    std::string ModelPath{};
};
