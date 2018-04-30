// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"

#include "D3DEngine.h"

using Microsoft::WRL::ComPtr;

D3DEngine::D3DEngine()
{
    // Use gamma-correct rendering.
    m_deviceResources = std::make_unique<DX::D3DDeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    m_deviceResources->RegisterDeviceNotify(this);
}

void D3DEngine::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
    if (!m_fenceEvent.IsValid())
    {
        throw std::exception("CreateEvent");
    }
}

// Updates the world.
void D3DEngine::Update(float elapsedTime)
{
    // Update the rotation constant
    m_curRotationAngleRad += elapsedTime / 2.f;
    if (m_curRotationAngleRad >= DirectX::XM_2PI)
    {
        m_curRotationAngleRad -= DirectX::XM_2PI;
    }

    // Rotate the cube around the origin
    m_d3dMesh.Rotate(m_curRotationAngleRad);
}

// Draws the scene.
void D3DEngine::Render()
{
    // Check to see if the GPU is keeping up
    int frameIdx = m_deviceResources->GetCurrentFrameIndex();
    int numBackBuffers = m_deviceResources->GetBackBufferCount();
    uint64_t completedValue = m_fence->GetCompletedValue();
    if ((frameIdx > completedValue) // if frame index is reset to zero it may temporarily be smaller than the last GPU signal
        && (frameIdx - completedValue > numBackBuffers))
    {
        // GPU not caught up, wait for at least one available frame
        DX::ThrowIfFailed(m_fence->SetEventOnCompletion(frameIdx - numBackBuffers, m_fenceEvent.Get()));
        WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
    }

    SceneConstantBuffer sceneParameters{};
    sceneParameters.lightDir[0] = XMLoadFloat4(&m_lightDirs[0]);
    sceneParameters.lightDir[1] = XMLoadFloat4(&m_lightDirs[1]);
    sceneParameters.lightColor[0] = XMLoadFloat4(&m_lightColors[0]);
    sceneParameters.lightColor[1] = XMLoadFloat4(&m_lightColors[1]);

    MeshConstantBuffer meshParameters{};
    DirectX::CXMMATRIX viewProj = XMLoadFloat4x4(&m_viewMatrix) * XMLoadFloat4x4(&m_projectionMatrix);
    meshParameters.worldViewProj = DirectX::XMMatrixTranspose(XMLoadFloat4x4(&m_d3dMesh.WorldMatrix()) * viewProj);
    meshParameters.world = DirectX::XMMatrixTranspose(XMLoadFloat4x4(&m_d3dMesh.WorldMatrix()));

    D3DFrameResource & currentFrame = m_deviceResources->GetCurrentFrameResource();
    currentFrame.SceneCB->CopyData(0, sceneParameters);
    currentFrame.MeshCB->CopyData(0, meshParameters);

    // Prepare the command list to render a new frame.
    PrepareRender();

    auto commandList = m_deviceResources->GetCommandList();

    ID3D12DescriptorHeap * descriptorHeaps[] = { m_cbvHeap.Get() };
    commandList->SetDescriptorHeaps(1, descriptorHeaps);

    // Set the root signature and pipeline state for the command list
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    commandList->SetGraphicsRootConstantBufferView(0, currentFrame.SceneCB->Resource()->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, currentFrame.MeshCB->Resource()->GetGPUVirtualAddress());

    commandList->SetPipelineState(m_lambertPipelineState.Get());
    m_d3dMesh.Render(commandList);

    CompleteRender(frameIdx);
}

// Helper method to clear the back buffers.
void D3DEngine::PrepareRender()
{
    m_deviceResources->Prepare();

    auto commandList = m_deviceResources->GetCommandList();

    // Clear the views.
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);

    // Use linear clear color for gamma-correct rendering.
    DirectX::XMVECTORF32 Background = { 0.38f, 0.36f, 0.36f, 1.f };
    commandList->ClearRenderTargetView(rtvDescriptor, Background, 0, nullptr);

    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
}

void D3DEngine::CompleteRender(int frameIdx)
{
    // Show the new frame.
    m_deviceResources->Present();

    // GPU will signal an increasing value each frame
    m_deviceResources->GetCommandQueue()->Signal(m_fence.Get(), frameIdx);
}

void D3DEngine::BuildRootSignature()
{
    CD3DX12_ROOT_PARAMETER slotRootParameter[2];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init(
        2,
        slotRootParameter,
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
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

void D3DEngine::BuildFrameResources()
{
}

void D3DEngine::BuildPipelineStateObjects()
{
    ID3D12Device * device = m_deviceResources->GetD3DDevice();

    UINT inputSlot = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, inputSlot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, inputSlot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    auto triangleVSBlob = ReadData(L"TriangleVS.cso");
    auto lambertPSBlob = ReadData(L"LambertPS.cso");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { &inputLayout[0], static_cast<UINT>(inputLayout.size()) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { triangleVSBlob.data(), triangleVSBlob.size() };
    psoDesc.PS = { lambertPSBlob.data(), lambertPSBlob.size() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat = m_deviceResources->GetDepthBufferFormat();
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
    psoDesc.SampleDesc.Count = 1;
    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_lambertPipelineState.ReleaseAndGetAddressOf())));
}

void D3DEngine::BuildDescriptorHeaps()
{
    UINT objCount = 1;

    // Need a CBV descriptor for each object for each frame resource,
    // +1 for the perPass CBV for each frame resource.
    UINT numDescriptors = (objCount + 1) * m_deviceResources->GetBackBufferCount();

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = numDescriptors;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;

    ID3D12Device * device = m_deviceResources->GetD3DDevice();
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void D3DEngine::BuildConstantBufferUploadBuffers()
{
    m_deviceResources->CreateConstantBuffers(1, 1);
}

void D3DEngine::BuildScene()
{
    fx::gltf::Document doc = fx::gltf::LoadFromText("E:/source/fx-gltf/test/data/glTF-Sample-Models/2.0/SciFiHelmet/glTF/SciFiHelmet.gltf");
    m_d3dMesh.CreateDeviceDependentResources(doc, 0, m_deviceResources);
}

void D3DEngine::CreateDeviceDependentResources()
{
    ID3D12Device * device = m_deviceResources->GetD3DDevice();

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Create a root signature with one constant buffer view
    BuildRootSignature();
    BuildPipelineStateObjects();

    BuildScene();

    BuildConstantBufferUploadBuffers();
    BuildDescriptorHeaps();

    // Wait until assets have been uploaded to the GPU.
    m_deviceResources->WaitForGpu();

    // Create a fence for synchronizing between the CPU and the GPU
    DX::ThrowIfFailed(
        device->CreateFence(m_deviceResources->GetCurrentFrameIndex(), D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));

    // Initialize the world matrix
    DirectX::XMStoreFloat4x4(&m_d3dMesh.WorldMatrix(), DirectX::XMMatrixIdentity());

    // Initialize the view matrix
    static const DirectX::XMVECTORF32 c_eye = { 0.0f, 0.0f, 5.0f, 0.0f };
    static const DirectX::XMVECTORF32 c_at = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const DirectX::XMVECTORF32 c_up = { 0.0f, 1.0f, 0.0f, 0.0 };
    DirectX::XMStoreFloat4x4(&m_viewMatrix, DirectX::XMMatrixLookAtLH(c_eye, c_at, c_up));

    // Initialize the lighting parameters
    m_lightDirs[0] = DirectX::XMFLOAT4(-0.67f, 0.67f, 0.67f, 1.0f);
    m_lightDirs[1] = DirectX::XMFLOAT4(0.57f, 0.0f, -0.57f, 1.0f);

    m_lightColors[0] = DirectX::XMFLOAT4(1.0f, 0.83f, 0.57f, 1.0f);
    m_lightColors[1] = DirectX::XMFLOAT4(0.45f, 0.78f, 0.85f, 1.0f);
}

void D3DEngine::CreateWindowSizeDependentResources()
{
    // Initialize the projection matrix
    auto size = m_deviceResources->GetOutputSize();
    DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, float(size.right) / float(size.bottom), 0.01f, 400.0f);

    XMStoreFloat4x4(&m_projectionMatrix, projection);

    // The frame index will be reset to zero when the window size changes
    // So we need to tell the GPU to signal our fence starting with zero
    uint64_t currentIdx = m_deviceResources->GetCurrentFrameIndex();
    m_deviceResources->GetCommandQueue()->Signal(m_fence.Get(), currentIdx);
}

void D3DEngine::WindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

std::vector<uint8_t> D3DEngine::ReadData(const wchar_t * name)
{
    std::ifstream inFile(name, std::ios::in | std::ios::binary | std::ios::ate);
    if (!inFile)
        throw std::exception("ReadData");

    std::streampos len = inFile.tellg();

    std::vector<uint8_t> blob;
    blob.resize(size_t(len));

    inFile.seekg(0, std::ios::beg);
    inFile.read(reinterpret_cast<char *>(blob.data()), len);

    return blob;
}

void D3DEngine::OnDeviceLost()
{
    m_rootSignature.Reset();

    m_fence.Reset();

    m_d3dMesh.Reset();

}

void D3DEngine::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
