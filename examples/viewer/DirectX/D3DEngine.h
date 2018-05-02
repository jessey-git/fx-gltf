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
#include "D3DUploadBuffer.h"

class D3DEngine : public DX::IDeviceNotify
{
public:
    D3DEngine(std::string modelPath);

    void Initialize(HWND window, int width, int height);

    void Update(float elapsedTime);
    void Render();

    void WindowSizeChanged(int width, int height);

    void Shutdown();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    // clang-format off
    fx::gltf::Document                              m_doc{};

    std::unique_ptr<DX::D3DDeviceResources>         m_deviceResources{};
    Microsoft::WRL::ComPtr<ID3D12RootSignature>     m_rootSignature{};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_cbvHeap{};

    Microsoft::WRL::ComPtr<ID3D12PipelineState>     m_lambertPipelineState{};

    Microsoft::WRL::ComPtr<ID3D12Fence>             m_fence{};
    Microsoft::WRL::Wrappers::Event                 m_fenceEvent{};

    DirectX::XMFLOAT4X4                             m_viewMatrix{};
    DirectX::XMFLOAT4X4                             m_projectionMatrix{};
    DirectX::XMFLOAT4                               m_lightDirs[2]{};
    DirectX::XMFLOAT4                               m_lightColors[2]{};

    std::vector<D3DMesh>                            m_meshes{};
    std::vector<D3DMeshInstance>                    m_meshInstances{};

    float                                           m_curRotationAngleRad{};
    // clang-format on

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void PrepareRender();
    void CompleteRender(int frameIdx);

    void BuildScene();

    void BuildRootSignature();
    void BuildDescriptorHeaps();
    void BuildPipelineStateObjects();
    void BuildConstantBufferUploadBuffers();

    std::vector<uint8_t> ReadData(const wchar_t * name);
};
