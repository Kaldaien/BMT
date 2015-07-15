#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3d11.lib")

extern bool nv_hardware;

#include <string>

// TODO: There's a lot of re-used code here, fix that!

extern int BMT_CountNVGPUs (void);

enum BMT_GPUMemoryPool {
  Dedicated,
  GART
};

size_t
BMT_GetAdapterPool (BMT_GPUMemoryPool pool)
{
  if (! BMT_CountNVGPUs ()) {
    IDXGIFactory* pFactory = NULL;
    CreateDXGIFactory (__uuidof(IDXGIFactory), (void **)&pFactory);

    IDXGIAdapter * pDXGIAdapter;
    for (UINT i = 0; pFactory->EnumAdapters (i, &pDXGIAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
      DXGI_ADAPTER_DESC adapterDesc;
      pDXGIAdapter->GetDesc (&adapterDesc);

      // Skip Microsoft's virtual adapters...
      if (adapterDesc.VendorId != 0x1414) {
        pDXGIAdapter->Release ();
        pFactory->Release ();

        if (pool == GART)
          return adapterDesc.SharedSystemMemory;
        else
          return adapterDesc.DedicatedVideoMemory;
      }

      pDXGIAdapter->Release ();
    }

    pFactory->Release ();
  }

  else {
    extern DXGI_ADAPTER_DESC* BMT_EnumNVGPUs (void);

    DXGI_ADAPTER_DESC* adapter_descs = BMT_EnumNVGPUs ();

    int i = 0;

    while (*adapter_descs [i].Description != L'\0') {
      if (pool == GART)
        return adapter_descs [i].SharedSystemMemory;
      else
        return adapter_descs [i].DedicatedVideoMemory;
    }
  }

  return 0;
}

size_t
BMT_GetGART (void)
{
  return BMT_GetAdapterPool (GART);
}

size_t
BMT_GetGPUVRAM (void)
{
  return BMT_GetAdapterPool (Dedicated);
}

std::wstring
BMT_GetGPUInfo (void)
{
  wchar_t adapters [4096];
  *adapters = L'\0';

  if (! BMT_CountNVGPUs ()) {
    IDXGIFactory* pFactory = NULL;
    CreateDXGIFactory (__uuidof(IDXGIFactory), (void **)&pFactory);

    IDXGIAdapter* pDXGIAdapter;
    for (UINT i = 0; pFactory->EnumAdapters (i, &pDXGIAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
      DXGI_ADAPTER_DESC adapterDesc;
      pDXGIAdapter->GetDesc (&adapterDesc);

      // Skip Microsoft's virtual adapters...
      if (adapterDesc.VendorId != 0x1414) {
        swprintf (adapters, L"%sAdapter %02u (Vendor ID: %04X):\r\n"
          L"----------------------------------------\r\n\r\n"
          L"  %s\r\n\r\n"
          L"    %4.1f GiB Dedicated VRAM\r\n"
          L"    %4.1f GiB Shared Memory (GART)\r\n\r\n",
          adapters,
          i,
          adapterDesc.VendorId,
          adapterDesc.Description,
          (float)adapterDesc.DedicatedVideoMemory / 1024.0f / 1024.0f / 1024.0f,
          (float)adapterDesc.SharedSystemMemory / 1024.0f / 1024.0f / 1024.0f);
      }

      pDXGIAdapter->Release ();
    }

    pFactory->Release ();
  }
  else {
    extern DXGI_ADAPTER_DESC* BMT_EnumNVGPUs (void);

    DXGI_ADAPTER_DESC* adapter_descs = BMT_EnumNVGPUs ();

    int i = 0;

    while (*adapter_descs [i].Description != L'\0') {
      swprintf (adapters, L"%sAdapter %02d (Vendor ID: %04X):\r\n"
        L"----------------------------------------\r\n\r\n"
        L"  %s\r\n\r\n"
        L"    %4.1f GiB Dedicated VRAM\r\n"
        L"    %4.1f GiB Shared Memory (GART)\r\n\r\n",
        adapters,
        i,
        adapter_descs [i].VendorId,
        adapter_descs [i].Description,
        (float)adapter_descs [i].DedicatedVideoMemory / 1024.0f / 1024.0f / 1024.0f,
        (float)adapter_descs [i].SharedSystemMemory / 1024.0f / 1024.0f / 1024.0f);

      ++i;
    }
  }

  return std::wstring (adapters);
}