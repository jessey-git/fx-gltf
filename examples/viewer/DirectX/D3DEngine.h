// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once
#include <fx/gltf.h>
#include <vector>

#include "D3DDeviceResources.h"
#include "D3DFrameResources.h"
#include "D3DMesh.h"
#include "D3DMeshInstance.h"
#include "D3DTexture.h"
#include "D3DUploadBuffer.h"
#include "Engine.h"
#include "EngineOptions.h"

class D3DEngine : public Engine, public DX::IDeviceNotify
{
public:
    explicit D3DEngine(EngineOptions const & config);

    D3DEngine(D3DEngine const &) = delete;
    D3DEngine(D3DEngine &&) = delete;
    D3DEngine & operator=(D3DEngine const &) = delete;
    D3DEngine & operator=(D3DEngine &&) = delete;

    ~D3DEngine();

    void InitializeCore(HWND window) override;

    void Update(float elapsedTime) noexcept override;
    void Render() override;

    void WindowSizeChangedCore(int width, int height) override;

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    // clang-format off
    fx::gltf::Document                              m_doc{};

    std::unique_ptr<DX::D3DDeviceResources>         m_deviceResources{};
    Microsoft::WRL::ComPtr<ID3D12RootSignature>     m_rootSignature{};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_cbvHeap{};

    std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>>    m_pipelineStates{};
    std::size_t                                                 m_currentPipelineState{};

    DirectX::XMFLOAT4X4                             m_viewMatrix{};
    DirectX::XMFLOAT4X4                             m_projectionMatrix{};
    DirectX::XMFLOAT4X4                             m_viewProjectionMatrix{};

    DirectX::XMVECTORF32                            m_eye{};
    DirectX::XMFLOAT4                               m_lightDirs[2]{};
    DirectX::XMFLOAT4                               m_lightColors[2]{};

    std::vector<D3DTexture>                         m_textures{};
    std::vector<D3DMesh>                            m_meshes{};
    std::vector<D3DMeshInstance>                    m_meshInstances{};

    float                                           m_curRotationAngleRad{};
    float                                           m_autoScaleFactor{};
    Util::BBox                                      m_boundingBox{};
    // clang-format on

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void PrepareRender();
    void CompleteRender();

    void BuildScene();

    void BuildRootSignature();
    void BuildDescriptorHeaps();
    void BuildPipelineStateObjects();
    void BuildConstantBufferUploadBuffers();

    std::vector<uint8_t> ReadData(const wchar_t * name);
};
