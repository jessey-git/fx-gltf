// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <fx/gltf.h>

class MaterialData
{
public:
    void SetData(fx::gltf::Material const & material)
    {
        m_material = material;
        m_hasData = true;
    }

    fx::gltf::Material const & Data() const noexcept
    {
        return m_material;
    }

    bool HasData() const
    {
        return m_hasData && !m_material.pbrMetallicRoughness.empty();
    }

private:
    fx::gltf::Material m_material{};
    bool m_hasData{};
};
