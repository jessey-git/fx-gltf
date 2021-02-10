// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include "d3dx12.h"

#include "D3DFrameResources.h"

// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
struct ID3DDeviceNotify
{
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;
};

// Controls all the DirectX device resources.
class D3DDeviceResources
{
public:
    static const unsigned int c_AllowTearing = 0x1;

    D3DDeviceResources(UINT backBufferCount, D3D_FEATURE_LEVEL minFeatureLevel, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT, unsigned int flags = 0);

    D3DDeviceResources(D3DDeviceResources const &) = delete;
    D3DDeviceResources(D3DDeviceResources &&) = delete;
    D3DDeviceResources & operator=(D3DDeviceResources const &) = delete;
    D3DDeviceResources & operator=(D3DDeviceResources &&) = delete;

    ~D3DDeviceResources();

    void CreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void CreateUploadBuffers(std::size_t sceneCount, std::size_t meshCount);

    void SetWindow(HWND window, int width, int height) noexcept;
    bool WindowSizeChanged(int width, int height);
    void HandleDeviceLost();
    void RegisterDeviceNotify(ID3DDeviceNotify * deviceNotify) noexcept
    {
        m_deviceNotify = deviceNotify;
    }

    void PrepareCommandList();
    void ExecuteCommandList();

    void Prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT);
    void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
    void WaitForGpu();

    // Device Accessors.
    RECT GetOutputSize() const noexcept
    {
        return m_outputSize;
    }

    // Direct3D Accessors.
    ID3D12Device * GetD3DDevice() const noexcept
    {
        return m_d3dDevice.Get();
    }

    IDXGISwapChain3 * GetSwapChain() const noexcept
    {
        return m_swapChain.Get();
    }

    D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const noexcept
    {
        return m_d3dFeatureLevel;
    }

    ID3D12Resource * GetDepthStencil() const noexcept
    {
        return m_depthStencil.Get();
    }

    ID3D12CommandQueue * GetCommandQueue() const noexcept
    {
        return m_commandQueue.Get();
    }

    ID3D12GraphicsCommandList * GetCommandList() const noexcept
    {
        return m_commandList.Get();
    }

    DXGI_FORMAT GetBackBufferFormat() const noexcept
    {
        return m_backBufferFormat;
    }

    DXGI_FORMAT GetDepthBufferFormat() const noexcept
    {
        return m_depthBufferFormat;
    }

    D3D12_VIEWPORT GetScreenViewport() const noexcept
    {
        return m_screenViewport;
    }

    D3D12_RECT GetScissorRect() const noexcept
    {
        return m_scissorRect;
    }

    UINT GetCurrentFrameIndex() const noexcept
    {
        return m_backBufferIndex;
    }

    UINT GetBackBufferCount() const noexcept
    {
        return m_backBufferCount;
    }

    unsigned int GetDeviceOptions() const noexcept
    {
        return m_options;
    }

    D3DFrameResource & GetCurrentFrameResource()
    {
        return m_frameResources[m_backBufferIndex];
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex, m_rtvDescriptorSize);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    }

private:
    void MoveToNextFrame();
    void GetAdapter(IDXGIAdapter1 ** ppAdapter);

    const static size_t MAX_BACK_BUFFER_COUNT = 3;

    UINT m_backBufferIndex;

    // clang-format off
    // Direct3D objects.
    Microsoft::WRL::ComPtr<ID3D12Device>                m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>          m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   m_commandList;

    std::vector<D3DFrameResource>                       m_frameResources;

    // Swap chain objects.
    Microsoft::WRL::ComPtr<IDXGIFactory4>           m_dxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain3>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_depthStencil;

    // Presentation fence objects.
    Microsoft::WRL::ComPtr<ID3D12Fence>             m_fence;
    Microsoft::WRL::Wrappers::Event                 m_fenceEvent;

    // Direct3D rendering objects.
    UINT                                            m_rtvDescriptorSize;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_rtvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_dsvDescriptorHeap;
    D3D12_VIEWPORT                                  m_screenViewport;
    D3D12_RECT                                      m_scissorRect;

    // Direct3D properties.
    UINT                m_backBufferCount;
    DXGI_FORMAT         m_backBufferFormat;
    DXGI_FORMAT         m_depthBufferFormat;
    D3D_FEATURE_LEVEL   m_d3dMinFeatureLevel;
    D3D_FEATURE_LEVEL   m_d3dFeatureLevel;

    // Cached device properties.
    HWND                m_window;
    RECT                m_outputSize;

    // DeviceResources options (see flags above)
    uint32_t            m_options;

    // The IDeviceNotify can be held directly as it owns the DeviceResources.
    ID3DDeviceNotify *  m_deviceNotify;
    // clang-format on
};
