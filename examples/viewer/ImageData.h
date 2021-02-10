// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <fx/gltf.h>
#include <string>

class ImageData
{
public:
    struct ImageInfo
    {
        std::string FileName{};

        uint32_t BinarySize{};
        uint8_t const * BinaryData{};

        bool IsBinary() const noexcept
        {
            return BinaryData != nullptr;
        }
    };

    explicit ImageData(std::string const & texture)
    {
        m_info.FileName = texture;
    }

    ImageData(fx::gltf::Document const & doc, std::size_t textureIndex, std::string const & modelPath)
    {
        fx::gltf::Image const & image = doc.images[doc.textures[textureIndex].source];

        const bool isEmbedded = image.IsEmbeddedResource();
        if (!image.uri.empty() && !isEmbedded)
        {
            m_info.FileName = fx::gltf::detail::GetDocumentRootPath(modelPath) + "/" + image.uri;
        }
        else
        {
            if (isEmbedded)
            {
                image.MaterializeData(m_embeddedData);
                m_info.BinaryData = &m_embeddedData[0];
                m_info.BinarySize = static_cast<uint32_t>(m_embeddedData.size());
            }
            else
            {
                fx::gltf::BufferView const & bufferView = doc.bufferViews[image.bufferView];
                fx::gltf::Buffer const & buffer = doc.buffers[bufferView.buffer];

                m_info.BinaryData = &buffer.data[bufferView.byteOffset];
                m_info.BinarySize = bufferView.byteLength;
            }
        }
    }

    ImageInfo const & Info() const noexcept
    {
        return m_info;
    }

private:
    ImageInfo m_info{};

    std::vector<uint8_t> m_embeddedData{};
};
