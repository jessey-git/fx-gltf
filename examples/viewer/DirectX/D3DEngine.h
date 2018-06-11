// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once
#include <fx/gltf.h>
#include <memory>
#include <unordered_map>
#include <vector>

#include "D3DDeviceResources.h"
#include "D3DEnvironmentIBL.h"
#include "D3DMesh.h"
#include "D3DMeshInstance.h"
#include "D3DTexture.h"
#include "Engine.h"
#include "EngineOptions.h"
#include "ShaderOptions.h"

class D3DEngine : public Engine, public ID3DDeviceNotify
{
public:
    explicit D3DEngine(EngineOptions const & config);

    D3DEngine(D3DEngine const &) = delete;
    D3DEngine(D3DEngine &&) = delete;
    D3DEngine & operator=(D3DEngine const &) = delete;
    D3DEngine & operator=(D3DEngine &&) = delete;

    ~D3DEngine() override;

    void InitializeCore(HWND window) override;

    void Update(float elapsedTime) noexcept override;
    void Render() override;

    void ChangeWindowSizeCore(int width, int height) override;

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    // clang-format off
    fx::gltf::Document                              m_gltfScene{};
    fx::gltf::Document                              m_gltfGround{};

    std::unique_ptr<D3DDeviceResources>             m_deviceResources{};
    Microsoft::WRL::ComPtr<ID3D12RootSignature>     m_rootSignature{};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_cbvHeap{};

    Microsoft::WRL::ComPtr<ID3D12PipelineState>     m_pipelineStateSky{};
    std::unordered_map<ShaderOptions, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_pipelineStateMap{};

    DirectX::XMFLOAT4X4                             m_viewMatrix{};
    DirectX::XMFLOAT4X4                             m_projectionMatrix{};
    DirectX::XMFLOAT4X4                             m_viewProjectionMatrix{};

    DirectX::XMVECTORF32                            m_eye{};
    Light                                           m_directionalLight{};
    Light                                           m_pointLights[2]{};

    D3DEnvironmentIBL                               m_environment{};
    std::vector<D3DTexture>                         m_textures{};
    std::vector<D3DMesh>                            m_meshes{};
    std::vector<D3DMeshInstance>                    m_meshInstances{};
    D3DMesh                                         m_groundMesh{};

    float                                           m_curRotationAngleRad{};
    float                                           m_autoScaleFactor{};
    Util::BBox                                      m_boundingBox{};
    Util::BBox                                      m_transformedBox{};
    // clang-format on

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void PrepareRender();
    void CompleteRender();

    void BuildEnvironmentMaps();
    void BuildScene();

    void BuildRootSignature();
    void BuildDescriptorHeaps();
    void BuildPipelineStateObjects();
    void BuildUploadBuffers();

    void CompileShaderPerumutation(std::string const & entryPoint, ShaderOptions options, D3D12_GRAPHICS_PIPELINE_STATE_DESC & psoDescTemplate);
};
