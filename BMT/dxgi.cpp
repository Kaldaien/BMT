/**
* This file is part of Batman Tweak.
*
* Batman Tweak is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* The Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Batman Tweak is distributed in the hope that it will be useful,
* But WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Batman Tweak.If not, see <http://www.gnu.org/licenses/>.
**/

#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <cstdio>

#include "dxgi.h"
#include "nvapi.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

using namespace bmt;

size_t
DXGI::GetAdapterPool (DXGI::GPUMemoryPool pool)
{
  if (! NVAPI::CountPhysicalGPUs ()) {
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
    DXGI_ADAPTER_DESC* adapter_descs = NVAPI::EnumGPUs_DXGI ();

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
DXGI::GetGART (void)
{
  return GetAdapterPool (GART);
}

size_t
DXGI::GetVRAM (void)
{
  return GetAdapterPool (Dedicated);
}

std::wstring
DXGI::GetGPUInfo (void)
{
  static wchar_t adapters [4096];
  *adapters = L'\0';

  if (!NVAPI::CountPhysicalGPUs ()) {
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
    DXGI_ADAPTER_DESC* adapter_descs = NVAPI::EnumGPUs_DXGI ();

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