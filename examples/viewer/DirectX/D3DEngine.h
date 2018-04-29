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
#include "D3DUploadBuffer.h"

class D3DEngine : public DX::IDeviceNotify
{
public:
    D3DEngine();

    void Initialize(HWND window, int width, int height);

    void Update(float elapsedTime);
    void Render();

    void WindowSizeChanged(int width, int height);

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    // Device and Frame resources.
    std::unique_ptr<DX::D3DDeviceResources>         m_deviceResources;

    Microsoft::WRL::ComPtr<ID3D12RootSignature>     m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_cbvHeap;
    UINT                                            m_descriptorSize;

    std::vector<std::unique_ptr<D3DUploadBuffer<SceneConstantBuffer>>> m_sceneCB;
    std::vector<std::unique_ptr<D3DUploadBuffer<MeshConstantBuffer>>>  m_meshCB;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_lambertPipelineState;

    Microsoft::WRL::ComPtr<ID3D12Fence>     m_fence;
    Microsoft::WRL::Wrappers::Event         m_fenceEvent;

    // These computed values will be loaded into a ConstantBuffer
    // during Render
    DirectX::XMFLOAT4X4                     m_viewMatrix;
    DirectX::XMFLOAT4X4                     m_projectionMatrix;
    DirectX::XMFLOAT4                       m_lightDirs[2];
    DirectX::XMFLOAT4                       m_lightColors[2];

    float                                   m_curRotationAngleRad{};

    D3DMesh                                 m_d3dMesh;

    void PrepareRender();
    void CompleteRender(int frameIdx);

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void BuildRootSignature();
    void BuildFrameResources();
    void BuildPipelineStateObjects();
    void BuildDescriptorHeaps();
    void BuildConstantBufferUploadBuffers();

    void BuildScene();

    std::vector<uint8_t> ReadData(const wchar_t * name);
};
