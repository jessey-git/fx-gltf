// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"

#include "D3DEngine.h"
#include "D3DGraph.h"
#include "D3DMeshInstance.h"
#include "Engine.h"
#include "EngineOptions.h"

using Microsoft::WRL::ComPtr;

D3DEngine::D3DEngine(EngineOptions const & config)
    : Engine(config)
{
}

D3DEngine::~D3DEngine()
{
    if (m_deviceResources != nullptr)
    {
        m_deviceResources->WaitForGpu();
    }
}

void D3DEngine::InitializeCore(HWND window)
{
    if (Config().ModelPath.rfind(".glb") != std::string::npos)
    {
        m_doc = fx::gltf::LoadFromBinary(Config().ModelPath);
    }
    else
    {
        m_doc = fx::gltf::LoadFromText(Config().ModelPath);
    }

    const uint32_t BackBufferCount = 3;
    m_deviceResources = std::make_unique<DX::D3DDeviceResources>(BackBufferCount, D3D_FEATURE_LEVEL_12_0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    m_deviceResources->SetWindow(window, Config().Width, Config().Height);
    m_deviceResources->RegisterDeviceNotify(this);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

void D3DEngine::Update(float elapsedTime) noexcept
{
    if (Config().AutoRotate)
    {
        m_curRotationAngleRad += elapsedTime / 3.0f;
        if (m_curRotationAngleRad >= DirectX::XM_2PI)
        {
            m_curRotationAngleRad -= DirectX::XM_2PI;
        }
    }
}

void D3DEngine::Render()
{
    // Prepare the command list to render a new frame.
    PrepareRender();

    D3DFrameResource const & currentFrame = m_deviceResources->GetCurrentFrameResource();

    SceneConstantBuffer sceneParameters{};
    sceneParameters.ViewProj = DirectX::XMLoadFloat4x4(&m_viewProjectionMatrix);
    sceneParameters.Eye = m_eye;
    sceneParameters.AutoLightDir = DirectX::XMLoadFloat4(&m_autoLightDir);
    sceneParameters.AutoLightFactor = DirectX::XMLoadFloat4(&m_autoLightFactor);
    sceneParameters.Lights[0] = m_lights[0];
    sceneParameters.Lights[1] = m_lights[1];
    currentFrame.SceneCB->CopyData(0, sceneParameters);

    ID3D12GraphicsCommandList * commandList = m_deviceResources->GetCommandList();

    // Set the root signature and pipeline state for the command list
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetGraphicsRootConstantBufferView(0, currentFrame.SceneCB->GetGPUVirtualAddress(0));
    commandList->SetGraphicsRootShaderResourceView(2, currentFrame.MeshDataBuffer->GetGPUVirtualAddress(0));
    commandList->SetPipelineState(m_pipelineStates[m_currentPipelineState].Get());

    if (!m_textures.empty())
    {
        ID3D12DescriptorHeap * descriptorHeaps[] = { m_cbvHeap.Get() };
        commandList->SetDescriptorHeaps(1, descriptorHeaps);

        const CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
        commandList->SetGraphicsRootDescriptorTable(4, tex);
    }

    std::size_t currentCBIndex = 0;
    DirectX::CXMMATRIX viewProj = DirectX::XMLoadFloat4x4(&m_viewProjectionMatrix);
    for (auto & meshInstance : m_meshInstances)
    {
        D3DMesh & mesh = m_meshes[meshInstance.MeshIndex];
        mesh.SetWorldMatrix(meshInstance.Transform, m_boundingBox.centerTranslation, m_curRotationAngleRad, m_autoScaleFactor);
        mesh.Render(commandList, currentFrame, viewProj, currentCBIndex);

        currentCBIndex += mesh.MeshPartCount();
    }

    CompleteRender();
}

void D3DEngine::WindowSizeChangedCore(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void D3DEngine::CreateDeviceDependentResources()
{
    m_deviceResources->PrepareCommandList();

    BuildScene();

    BuildRootSignature();
    BuildPipelineStateObjects();
    BuildConstantBufferUploadBuffers();
    BuildDescriptorHeaps();

    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu();

    for (auto & mesh : m_meshes)
    {
        mesh.FinishUpload();
    }

    // Initialize the view matrix
    m_eye = { 0.0f, 0.0f, 6.0f, 0.0f };
    static const DirectX::XMVECTORF32 c_at = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const DirectX::XMVECTORF32 c_up = { 0.0f, 1.0f, 0.0f, 0.0 };
    DirectX::XMStoreFloat4x4(&m_viewMatrix, DirectX::XMMatrixLookAtLH(m_eye, c_at, c_up));

    // Initialize the lighting parameters
    m_autoLightDir = DirectX::XMFLOAT4(-0.57f, 0.57f, 0.57f, 1.0f);
    m_autoLightFactor = DirectX::XMFLOAT4(1.0f, 0.83f, 0.57f, 1.0f);
    m_lights[0].Position = DirectX::XMFLOAT3(-6.0f, 6.0f, 6.0f);
    m_lights[0].Strength = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
    m_lights[0].FalloffStart = 0.0f;
    m_lights[0].FalloffEnd = 100.0f;
    m_lights[1].Position = DirectX::XMFLOAT3(6.0f, 6.0f, 6.0f);
    m_lights[1].Strength = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
    m_lights[1].FalloffStart = 0.0f;
    m_lights[1].FalloffEnd = 100.0f;
}

void D3DEngine::CreateWindowSizeDependentResources()
{
    // Initialize the projection matrix
    const auto size = m_deviceResources->GetOutputSize();
    DirectX::CXMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, static_cast<float>(size.right) / size.bottom, 0.01f, 400.0f);
    DirectX::CXMMATRIX viewProj = DirectX::XMLoadFloat4x4(&m_viewMatrix) * projection;

    DirectX::XMStoreFloat4x4(&m_projectionMatrix, projection);
    DirectX::XMStoreFloat4x4(&m_viewProjectionMatrix, viewProj);
}

void D3DEngine::PrepareRender()
{
    m_deviceResources->Prepare();

    ID3D12GraphicsCommandList * commandList = m_deviceResources->GetCommandList();

    // Clear the views.
    const auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    const auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);

    // Use linear clear color for gamma-correct rendering.
    const DirectX::XMVECTORF32 Background = { 0.26f, 0.24f, 0.24f, 1.0f };
    commandList->ClearRenderTargetView(rtvDescriptor, Background, 0, nullptr);

    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    const auto viewport = m_deviceResources->GetScreenViewport();
    const auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
}

void D3DEngine::CompleteRender()
{
    // Show the new frame.
    m_deviceResources->Present();
}

void D3DEngine::BuildScene()
{
    Logger::WriteLine("Building scene...");

    if (Config().UseMaterials)
    {
        Logger::WriteLine("Building textures...");
        m_textures.resize(m_doc.textures.size());
        for (uint32_t i = 0; i < m_doc.textures.size(); i++)
        {
            std::string image = fx::gltf::detail::GetDocumentRootPath(Config().ModelPath) + "/" + m_doc.images[m_doc.textures[i].source].uri;
            Logger::WriteLine("  {0}", image);
            m_textures[i].Create(image, m_deviceResources.get());
        }
    }

    Logger::WriteLine("Building meshes...");
    m_meshes.resize(m_doc.meshes.size());
    for (uint32_t i = 0; i < m_doc.meshes.size(); i++)
    {
        m_meshes[i].Create(m_doc, i, m_deviceResources.get());
        Util::AdjustBBox(m_boundingBox, m_meshes[i].MeshBBox());
    }

    Util::CenterBBox(m_boundingBox);

    if (!m_doc.scenes.empty())
    {
        Logger::WriteLine("Building scene graph...");
        std::vector<Graph::Node> graphNodes(m_doc.nodes.size());
        const DirectX::XMMATRIX rootTransform = DirectX::XMMatrixMultiply(DirectX::XMMatrixIdentity(), DirectX::XMMatrixScaling(-1, 1, 1));
        for (const uint32_t sceneNode : m_doc.scenes[0].nodes)
        {
            Graph::Visit(m_doc, sceneNode, rootTransform, graphNodes);
        }

        Logger::WriteLine("Traversing scene...");
        for (auto & graphNode : graphNodes)
        {
            if (graphNode.meshIndex >= 0)
            {
                m_meshInstances.push_back({ static_cast<uint32_t>(graphNode.meshIndex), graphNode.currentTransform });
            }
        }
    }
    else
    {
        Logger::WriteLine("No scene graph data. Displaying individual meshes...");
        for (uint32_t i = 0; i < m_doc.meshes.size(); i++)
        {
            m_meshInstances.push_back({ i, DirectX::XMMatrixIdentity() });
        }
    }

    DirectX::FXMVECTOR sizeVec = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&m_boundingBox.max), DirectX::XMLoadFloat3(&m_boundingBox.min));
    DirectX::XMFLOAT3 size{};
    DirectX::XMStoreFloat3(&size, sizeVec);
    m_autoScaleFactor = 4.0f / std::max({ size.x, size.y, size.z });

    Logger::WriteLine("Found {0} mesh instance(s)", m_meshInstances.size());
    Logger::WriteLine("Scene bbox       : [{0},{1},{2}] [{3},{4},{5}]", m_boundingBox.min.x, m_boundingBox.min.y, m_boundingBox.min.z, m_boundingBox.max.x, m_boundingBox.max.y, m_boundingBox.max.z);
    Logger::WriteLine("Scene translation: [{0},{1},{2}]", m_boundingBox.centerTranslation.x, m_boundingBox.centerTranslation.y, m_boundingBox.centerTranslation.z);
    Logger::WriteLine("Auto-scale factor: {0:F4}", m_autoScaleFactor);
}

void D3DEngine::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTableEnvironment;
    texTableEnvironment.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableMesh;
    texTableMesh.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 1);

    CD3DX12_ROOT_PARAMETER slotRootParameter[5];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTableEnvironment, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTableMesh, D3D12_SHADER_VISIBILITY_PIXEL);

    const std::array<const CD3DX12_STATIC_SAMPLER_DESC, 1> staticSamplers{
        CD3DX12_STATIC_SAMPLER_DESC(
            0, // shaderRegister
            D3D12_FILTER_ANISOTROPIC, // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressW
            0.0f, // mipLODBias
            8) // maxAnisotropy
    };

    const CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
        5,
        slotRootParameter,
        static_cast<UINT>(staticSamplers.size()),
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    const HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(hr))
    {
        if (error)
        {
            throw std::runtime_error(reinterpret_cast<const char *>(error->GetBufferPointer()));
        }

        throw DX::com_exception(hr);
    }

    ID3D12Device * device = m_deviceResources->GetD3DDevice();
    DX::ThrowIfFailed(device->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())));
}

void D3DEngine::BuildDescriptorHeaps()
{
    if (!m_textures.empty())
    {
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
        cbvHeapDesc.NumDescriptors = static_cast<UINT>(m_textures.size());
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.NodeMask = 0;

        ID3D12Device * device = m_deviceResources->GetD3DDevice();
        DX::ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

        CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

        const uint32_t size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (auto & texture : m_textures)
        {
            texture.CreateSRV(device, hDescriptor);
            hDescriptor.Offset(1, size);
        }
    }
}

void D3DEngine::BuildPipelineStateObjects()
{
    ID3D12Device * device = m_deviceResources->GetD3DDevice();

    UINT inputSlot = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, inputSlot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, inputSlot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

    const D3D_SHADER_MACRO defines[]{ "USE_AUTO_COLOR", "1", nullptr, nullptr };

    Logger::WriteLine("Compiling shaders...");
    auto standardVS = DX::CompileShader(L"DirectX\\Shaders\\Default.hlsl", "StandardVS", "vs_5_1", nullptr);
    auto lambertPS = DX::CompileShader(L"DirectX\\Shaders\\Default.hlsl", "UberPS", "ps_5_1", defines);
    auto pbrPS = DX::CompileShader(L"DirectX\\Shaders\\Default.hlsl", "UberPS", "ps_5_1", nullptr);
    auto skyVS = DX::CompileShader(L"DirectX\\Shaders\\Sky.hlsl", "VS", "vs_5_1", nullptr);
    auto skyPS = DX::CompileShader(L"DirectX\\Shaders\\Sky.hlsl", "PS", "ps_5_1", nullptr);

    m_pipelineStates.resize(2);
    m_currentPipelineState = Config().UseMaterials ? 1 : 0;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { &inputLayout[0], static_cast<UINT>(inputLayout.size()) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { standardVS->GetBufferPointer(), standardVS->GetBufferSize() };
    psoDesc.PS = { lambertPS->GetBufferPointer(), lambertPS->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat = m_deviceResources->GetDepthBufferFormat();
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
    psoDesc.SampleDesc.Count = 1;
    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineStates[0].ReleaseAndGetAddressOf())));

    psoDesc.PS = { pbrPS->GetBufferPointer(), pbrPS->GetBufferSize() };
    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineStates[1].ReleaseAndGetAddressOf())));

    psoDesc.VS = { skyVS->GetBufferPointer(), skyVS->GetBufferSize() };
    psoDesc.PS = { skyPS->GetBufferPointer(), skyPS->GetBufferSize() };
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineStateSky.ReleaseAndGetAddressOf())));
}

void D3DEngine::BuildConstantBufferUploadBuffers()
{
    std::size_t cbCount = 0;
    for (auto & meshInstance : m_meshInstances)
    {
        D3DMesh const & mesh = m_meshes[meshInstance.MeshIndex];
        cbCount += mesh.MeshPartCount();
    }

    m_deviceResources->CreateConstantBuffers(1, cbCount);
}

void D3DEngine::OnDeviceLost()
{
    m_rootSignature.Reset();

    for (auto & mesh : m_meshes)
    {
        mesh.Reset();
    }
}

void D3DEngine::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
