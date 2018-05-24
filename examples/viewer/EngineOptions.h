// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

struct EngineOptions
{
    int Width{ 960 };
    int Height{ 540 };

    bool AutoRotate{};
    bool UseMaterials{};
    bool UseIBL{};

    std::string ModelPath{};
};

