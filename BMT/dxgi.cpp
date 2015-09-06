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

#define __NvAPI_GetPhysicalGPUFromDisplay                 0x1890E8DA
#define __NvAPI_GetPhysicalGPUFromGPUID                   0x5380AD1A
#define __NvAPI_GetGPUIDfromPhysicalGPU                   0x6533EA3E

#define __NvAPI_GetInfoFrameStatePvt                      0x7FC17574
#define __NvAPI_GPU_GetMemoryInfo                         0x07F9B368

#define __NvAPI_LoadMicrocode                             0x3119F36E
#define __NvAPI_GetLoadedMicrocodePrograms                0x919B3136
#define __NvAPI_GetDisplayDriverBuildTitle                0x7562E947
#define __NvAPI_GetDisplayDriverCompileType               0x988AEA78
#define __NvAPI_GetDisplayDriverSecurityLevel             0x9D772BBA
#define __NvAPI_AccessDisplayDriverRegistry               0xF5579360
#define __NvAPI_GetDisplayDriverRegistryPath              0x0E24CEEE
#define __NvAPI_GetUnAttachedDisplayDriverRegistryPath    0x633252D8
#define __NvAPI_GPU_GetRawFuseData                        0xE0B1DCE9
#define __NvAPI_GPU_GetFoundry                            0x5D857A00
#define __NvAPI_GPU_GetVPECount                           0xD8CBF37B

#define __NvAPI_GPU_GetTargetID                           0x35B5FD2F

#define __NvAPI_GPU_GetShortName                          0xD988F0F3

#define __NvAPI_GPU_GetVbiosMxmVersion                    0xE1D5DABA
#define __NvAPI_GPU_GetVbiosImage                         0xFC13EE11
#define __NvAPI_GPU_GetMXMBlock                           0xB7AB19B9

#define __NvAPI_GPU_SetCurrentPCIEWidth                   0x3F28E1B9
#define __NvAPI_GPU_SetCurrentPCIESpeed                   0x3BD32008
#define __NvAPI_GPU_GetPCIEInfo                           0xE3795199
#define __NvAPI_GPU_ClearPCIELinkErrorInfo                0x8456FF3D
#define __NvAPI_GPU_ClearPCIELinkAERInfo                  0x521566BB
#define __NvAPI_GPU_GetFrameBufferCalibrationLockFailures 0x524B9773
#define __NvAPI_GPU_SetDisplayUnderflowMode               0x387B2E41
#define __NvAPI_GPU_GetDisplayUnderflowStatus             0xED9E8057

#define __NvAPI_GPU_GetBarInfo                            0xE4B701E3

#define __NvAPI_GPU_GetPSFloorSweepStatus                 0xDEE047AB
#define __NvAPI_GPU_GetVSFloorSweepStatus                 0xD4F3944C
#define __NvAPI_GPU_GetSerialNumber                       0x14B83A5F
#define __NvAPI_GPU_GetManufacturingInfo                  0xA4218928

#define __NvAPI_GPU_GetRamConfigStrap                     0x51CCDB2A
#define __NvAPI_GPU_GetRamBusWidth                        0x7975C581

#define __NvAPI_GPU_GetRamBankCount                       0x17073A3C
#define __NvAPI_GPU_GetArchInfo                           0xD8265D24
#define __NvAPI_GPU_GetExtendedMinorRevision              0x25F17421
#define __NvAPI_GPU_GetSampleType                         0x32E1D697
#define __NvAPI_GPU_GetHardwareQualType                   0xF91E777B
#define __NvAPI_GPU_GetAllClocks                          0x1BD69F49
#define __NvAPI_GPU_SetClocks                             0x6F151055
#define __NvAPI_GPU_SetPerfHybridMode                     0x7BC207F8
#define __NvAPI_GPU_GetPerfHybridMode                     0x5D7CCAEB
#define __NvAPI_GPU_GetHybridControllerInfo               0xD26B8A58
#define __NvAPI_GetHybridMode                             0x0E23B68C1

#define __NvAPI_RestartDisplayDriver                      0xB4B26B65
#define __NvAPI_GPU_GetAllGpusOnSameBoard                 0x4DB019E6

#define __NvAPI_SetTopologyDisplayGPU                     0xF409D5E5
#define __NvAPI_GetTopologyDisplayGPU                     0x813D89A8
#define __NvAPI_SYS_GetSliApprovalCookie                  0xB539A26E

#define __NvAPI_CreateUnAttachedDisplayFromDisplay        0xA0C72EE4
#define __NvAPI_GetDriverModel                            0x25EEB2C4
#define __NvAPI_GPU_CudaEnumComputeCapableGpus            0x5786CC6E
#define __NvAPI_GPU_PhysxSetState                         0x4071B85E
#define __NvAPI_GPU_PhysxQueryRecommendedState            0x7A4174F4
#define __NvAPI_GPU_GetDeepIdleState                      0x1AAD16B4
#define __NvAPI_GPU_SetDeepIdleState                      0x568A2292

#define __NvAPI_GetScalingCaps                            0x8E875CF9
#define __NvAPI_GPU_GetThermalTable                       0xC729203C
#define __NvAPI_GPU_GetHybridControllerInfo               0xD26B8A58
#define __NvAPI_SYS_SetPostOutput                         0xD3A092B1

#include "utility.h"
#define NVAPI_CALL(x) { NvAPI_Status ret = NvAPI_##x; if (ret != NVAPI_OK) BMT_MessageBox (ErrorMessage (ret, #x, __LINE__, __FUNCTION__, __FILE__), L"Error Calling NVAPI Function", MB_OK | MB_ICONASTERISK ); }

std::wstring
ErrorMessage (_NvAPI_Status err,
  const char*   args,
  UINT          line_no,
  const char*   function_name,
  const char*   file_name)
{
  char szError [64];

  NvAPI_GetErrorMessage (err, szError);

  wchar_t wszError [64];
  wchar_t wszFile [256];
  wchar_t wszFunction [256];
  wchar_t wszArgs [256];
  wchar_t wszFormattedError [1024];

  MultiByteToWideChar (CP_OEMCP, 0, szError, -1, wszError, 64);
  MultiByteToWideChar (CP_OEMCP, 0, file_name, -1, wszFile, 256);
  MultiByteToWideChar (CP_OEMCP, 0, function_name, -1, wszFunction, 256);
  MultiByteToWideChar (CP_OEMCP, 0, args, -1, wszArgs, 256);
  *wszFormattedError = L'\0';

  swprintf (wszFormattedError, 1024,
    L"Line %u of %s (in %s (...)):\n"
    L"------------------------\n\n"
    L"NvAPI_%s\n\n\t>> %s <<",
    line_no,
    wszFile,
    wszFunction,
    wszArgs,
    wszError);

  return wszFormattedError;
}

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

static char* dump_ints (void* data, const unsigned int& len)
{
  static char szOut [4096];
  char* pszOut = szOut;
  pszOut += sprintf (pszOut, "Size:  %d\n", len);

  for (int i = 1; i < len / 4; i++) {
    pszOut += sprintf (pszOut, "NvU32 [%d] : %u\n", i - 1, ((NvU32 *)data) [i]);
  }

  for (int i = 2; i < len / 2; i++) {
    pszOut += sprintf (pszOut, "NvU16 [%d] : %u\n", i - 2, ((NvU16 *)data) [i]);
  }

  return szOut;
}

// Generic dump, for unknown data where lexical analysis is needed.
static char* dump (void* data, const unsigned int& len)
{
  static char szOut [4096];
  char* pszOut = szOut;
  pszOut += sprintf (pszOut, "Size:  %d\n", len);

  if (len > 0) {
    unsigned width = 16;
    unsigned char *str = (unsigned char *)data;
    unsigned int j, i = 0;

    while (i < len) {
      pszOut += sprintf (pszOut, " ");

      for (j = 0; j < width; j++) {
        if (i + j < len)
          pszOut += sprintf (pszOut, "%02x ", (unsigned char)str [j]);
        else
          pszOut += sprintf (pszOut, "   ");

        if ((j + 1) % (width / 2) == 0)
          pszOut += sprintf (pszOut, " -  ");
      }

      for (j = 0; j < width; j++) {
        if (i + j < len)
          pszOut += sprintf (pszOut, "%c", isprint (str [j]) ? str [j] : '.');
        else
          pszOut += sprintf (pszOut, " ");
      }

      str += width;
      i += j;

      pszOut += sprintf (pszOut, "\n");
    }
  }

  return szOut;
}

std::wstring
DXGI::GetGPUInfo (void)
{
  static wchar_t adapters [4096];
  *adapters = L'\0';

  if (NVAPI::CountPhysicalGPUs ()) {

    typedef NvU32 NvGPUID;

    typedef void* (*NvAPI_QueryInterface_t)(unsigned int offset);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetRamType_t)(NvPhysicalGpuHandle handle, NvU32* memtype);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetShaderPipeCount_t)(NvPhysicalGpuHandle handle, NvU32* count);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetShaderSubPipeCount_t)(NvPhysicalGpuHandle handle, NvU32* count);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetFBWidthAndLocation_t)(NvPhysicalGpuHandle handle, NvU32* width, NvU32* loc);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetPartitionCount_t)(NvPhysicalGpuHandle handle, NvU32* count);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetTotalSMCount_t)(NvPhysicalGpuHandle handle, NvU32* count);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetTotalSPCount_t)(NvPhysicalGpuHandle handle, NvU32* count);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetTotalTPCCount_t)(NvPhysicalGpuHandle handle, NvU32* count);

    typedef NvAPI_Status (__cdecl *NvAPI_RestartDisplayDriver_t)(void);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetSerialNumber_t)(NvPhysicalGpuHandle handle, NvU32* num);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetManufacturingInfo_t)(NvPhysicalGpuHandle handle, void* data);
    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetFoundry_t)(NvPhysicalGpuHandle handle, void* data);
    typedef NvAPI_Status (__cdecl *NvAPI_GetDriverModel_t)(NvPhysicalGpuHandle handle, NvU32* data);
    typedef NvAPI_Status (__cdecl *NvAPI_GetGPUIDFromPhysicalGPU_t)(NvPhysicalGpuHandle handle, NvGPUID* gpuid);

    typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetShortName_t)(NvPhysicalGpuHandle handle, NvAPI_ShortString str);
    typedef NvAPI_Status (__cdecl *NvAPI_GetHybridMode_t)(NvPhysicalGpuHandle handle, NvU32* mode);

    HMODULE hLib = LoadLibrary (L"nvapi64.dll");
    NvAPI_QueryInterface_t NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress (hLib, "nvapi_QueryInterface");
    NvAPI_GPU_GetRamType_t NvAPI_GPU_GetRamType = (NvAPI_GPU_GetRamType_t)NvAPI_QueryInterface (0x57F7CAAC);
    NvAPI_GPU_GetShaderPipeCount_t NvAPI_GPU_GetShaderPipeCount = (NvAPI_GPU_GetShaderPipeCount_t)NvAPI_QueryInterface (0x63E2F56F);
    NvAPI_GPU_GetShaderSubPipeCount_t NvAPI_GPU_GetShaderSubPipeCount = (NvAPI_GPU_GetShaderSubPipeCount_t)NvAPI_QueryInterface (0x0BE17923);
    NvAPI_GPU_GetFBWidthAndLocation_t NvAPI_GPU_GetFBWidthAndLocation = (NvAPI_GPU_GetFBWidthAndLocation_t)NvAPI_QueryInterface (0x11104158);
    NvAPI_GPU_GetPartitionCount_t NvAPI_GPU_GetPartitionCount = (NvAPI_GPU_GetPartitionCount_t)NvAPI_QueryInterface (0x86F05D7A);
    NvAPI_RestartDisplayDriver_t NvAPI_RestartDisplayDriver = (NvAPI_RestartDisplayDriver_t)NvAPI_QueryInterface (__NvAPI_RestartDisplayDriver);
    NvAPI_GPU_GetSerialNumber_t NvAPI_GPU_GetSerialNumber = (NvAPI_GPU_GetSerialNumber_t)NvAPI_QueryInterface (__NvAPI_GPU_GetSerialNumber);
    NvAPI_GPU_GetManufacturingInfo_t NvAPI_GPU_GetManufacturingInfo = (NvAPI_GPU_GetManufacturingInfo_t)NvAPI_QueryInterface (__NvAPI_GPU_GetManufacturingInfo);
    NvAPI_GPU_GetFoundry_t NvAPI_GPU_GetFoundry = (NvAPI_GPU_GetFoundry_t)NvAPI_QueryInterface (__NvAPI_GPU_GetFoundry);
    NvAPI_GetDriverModel_t NvAPI_GetDriverModel = (NvAPI_GetDriverModel_t)NvAPI_QueryInterface (__NvAPI_GetDriverModel);
    NvAPI_GPU_GetShortName_t NvAPI_GPU_GetShortName = (NvAPI_GPU_GetShortName_t)NvAPI_QueryInterface (__NvAPI_GPU_GetShortName);

    NvAPI_GPU_GetTotalSMCount_t NvAPI_GPU_GetTotalSMCount = (NvAPI_GPU_GetTotalSMCount_t)NvAPI_QueryInterface (0x0AE5FBCFE);// 0x329D77CD);// 0x0AE5FBCFE);
    NvAPI_GPU_GetTotalSPCount_t NvAPI_GPU_GetTotalSPCount = (NvAPI_GPU_GetTotalSPCount_t)NvAPI_QueryInterface (0xE4B701E3);// 0xE0B1DCE9);// 0x0B6D62591);
    NvAPI_GPU_GetTotalTPCCount_t NvAPI_GPU_GetTotalTPCCount = (NvAPI_GPU_GetTotalTPCCount_t)NvAPI_QueryInterface (0x4E2F76A8);
    //NvAPI_GPU_GetTotalTPCCount_t NvAPI_GPU_GetTotalTPCCount = (NvAPI_GPU_GetTotalTPCCount_t)NvAPI_QueryInterface (0xD8265D24);// 0x4E2F76A8);// __NvAPI_GPU_Get
    NvAPI_GetGPUIDFromPhysicalGPU_t NvAPI_GetGPUIDFromPhysicalGPU = (NvAPI_GetGPUIDFromPhysicalGPU_t)NvAPI_QueryInterface (__NvAPI_GetGPUIDfromPhysicalGPU);
    NvAPI_GetHybridMode_t NvAPI_GetHybridMode = (NvAPI_GetHybridMode_t)NvAPI_QueryInterface (__NvAPI_GetHybridMode);

    // NvU32 [16] == ROPs
    //NvAPI_GPU_GetTotalTPCCount_t NvAPI_GPU_GetTotalTPCCount = (NvAPI_GPU_GetTotalTPCCount_t)NvAPI_QueryInterface (0xE0B1DCE9);


    //TPC:SM:SP.... : 0 : 241 : 4130992

    //NvAPI_GPU_GetTotalSMCount_t    NvAPI_GPU_GetTotalSMCount = (NvAPI_GPU_GetTotalSMCount_t)NvAPI_QueryInterface (0x0AE5FBCFE);
    FreeLibrary (hLib);

    //NVAPI_CALL (RestartDisplayDriver ())

    DXGI_ADAPTER_DESC* adapter_descs = NVAPI::EnumGPUs_DXGI ();
#if 0
    adapter_descs->

      NV_GPU_DISPLAYIDS disp_ids [8];
    NvU32             disp_cnt;

    disp_ids [0].version = NV_GPU_DISPLAYIDS_VER;
    disp_ids [1].version = NV_GPU_DISPLAYIDS_VER;
    disp_ids [2].version = NV_GPU_DISPLAYIDS_VER;
    disp_ids [3].version = NV_GPU_DISPLAYIDS_VER;
    disp_ids [4].version = NV_GPU_DISPLAYIDS_VER;
    disp_ids [5].version = NV_GPU_DISPLAYIDS_VER;
    disp_ids [6].version = NV_GPU_DISPLAYIDS_VER;
    disp_ids [7].version = NV_GPU_DISPLAYIDS_VER;

    NvPhysicalGpuHandle gpus [16];
    NvU32               gpu_count = 16;

    NV_GET_CURRENT_SLI_STATE sli_state;
    sli_state.version = NV_GET_CURRENT_SLI_STATE_VER;
    NvAPI_D3D11_CreateDevice ()
      NvAPI_D3D_GetCurrentSLIState ()
      NvAPI_D3D_GetCurrentSLIState ()
      NVAPI_CALL (EnumPhysicalGPUs (gpus, &gpu_count));
    NVAPI_CALL (GPU_GetConnectedDisplayIds (gpus [0], disp_ids, &disp_cnt, NV_GPU_CONNECTED_IDS_FLAG_SLI));

    if (disp_cnt > 0) {
      MessageBox (NULL, L"SLI", L"SLI", MB_OK);
    }
#endif
    NvPhysicalGpuHandle gpus [16];
    NvU32               gpu_count = 16;

    NVAPI_CALL (EnumPhysicalGPUs (gpus, &gpu_count));

    int i = 0;

    while (*adapter_descs [i].Description != L'\0') {
      NV_GPU_CLOCK_FREQUENCIES freq;
      freq.version = NV_GPU_CLOCK_FREQUENCIES_VER;

      freq.ClockType = NV_GPU_CLOCK_FREQUENCIES_BASE_CLOCK;
      NVAPI_CALL (GPU_GetAllClockFrequencies (gpus [i], &freq));

      float base_gpu = (float)freq.domain [NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency / 1000.0f / 1000.0f;
      float base_mem = (float)freq.domain [NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency / 1000.0f / 1000.0f;

      freq.ClockType = NV_GPU_CLOCK_FREQUENCIES_BOOST_CLOCK;
      NVAPI_CALL (GPU_GetAllClockFrequencies (gpus [i], &freq));

      float boost_gpu = (float)freq.domain [NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency / 1000.0f / 1000.0f;
      float boost_mem = (float)freq.domain [NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency / 1000.0f / 1000.0f;

      freq.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
      NVAPI_CALL (GPU_GetAllClockFrequencies (gpus [i], &freq));

      float current_gpu = (float)freq.domain [NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency / 1000.0f / 1000.0f;
      float current_mem = (float)freq.domain [NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency / 1000.0f / 1000.0f;

      NvU32 cuda_cores;
      NVAPI_CALL (GPU_GetGpuCoreCount (gpus [i], &cuda_cores));

      NV_CHIPSET_INFO chipset;
      chipset.version = NV_CHIPSET_INFO_VER;

      NvAPI_ShortString szCodename;
      wchar_t           wszCodename [8];

      NVAPI_CALL (GPU_GetShortName (gpus [i], szCodename));
      MultiByteToWideChar (CP_OEMCP, 0, szCodename, -1, wszCodename, 8);

      NvU32 ram_type;
      NVAPI_CALL (GPU_GetRamType (gpus [i], &ram_type));

      NvU32 pipes, subpipes;
      NvU32 rops;

      struct mem_width {
        NvU32 width;
        NvU32 location;
      } memory_type;

      NvU32 mem_parts;

      NVAPI_CALL (GPU_GetShaderPipeCount (gpus [i], &pipes));
      NVAPI_CALL (GPU_GetShaderSubPipeCount (gpus [i], &subpipes));
      NVAPI_CALL (GPU_GetFBWidthAndLocation (gpus [i], &memory_type.width, &memory_type.location));
      NVAPI_CALL (GPU_GetPartitionCount (gpus [i], &mem_parts));

      NvU32 serial;
      NVAPI_CALL (GPU_GetSerialNumber (gpus [i], &serial));


      struct mfg_info_t {
        NvU32 version;
        NvU32 unknown;
        NvU32 data [4];
      } mfg_info;

      mfg_info.version = MAKE_NVAPI_VERSION (mfg_info_t, 1);

      struct foundry_t {
        NvU32 version;
        NvU32 unknown;
        NvU32 data [128];// [32];
      } foundry;

      NvU32 model = 0x0100; // WDDM 1.0

      foundry.version = 0;// MAKE_NVAPI_VERSION (foundry_t, 1);

#if 0
      NvAPI_Status stat = NvAPI_GPU_GetFoundry (gpus [i], &foundry);
      if (stat == NVAPI_OK) {
        //MessageBox (NULL, L"Found something that works!", wszNum, MB_OK);
        //MessageBoxA (NULL, dump (&foundry, sizeof (foundry_t)), "Blah", MB_OK);
      }
#endif

      NVAPI_CALL (GetDriverModel (gpus [i], &model));

      //NvGPUID gpuid;
      //NVAPI_CALL (GetGPUIDFromPhysicalGPU (gpus [i], &gpuid));

      //NvU32 hybrid;
      //NVAPI_CALL (GetHybridMode (gpus [i], &hybrid));

#if 0
      NvU32 TPC, SM, SP;
      memset (&foundry, 0xff, sizeof (foundry_t));
      for (int j = 1000; j > 0; j--) {
        //for (int j = 1; j < 1000; j++) {
        foundry.version = j | (1 << 16);
        stat = NvAPI_GPU_GetTotalSMCount ((NvPhysicalGpuHandle)i, (NvU32 *)&foundry);
        if (stat == NVAPI_OK) {
          MessageBoxA (NULL, dump (&foundry, 32), "Blah", MB_OK);
          //MessageBoxA (NULL, dump_ints (&foundry, foundry.version & 0xffff/* (foundry_t)*/), "Blah", MB_OK);
          break;
        }
      }
      //NVAPI_CALL (GPU_GetTotalSPCount (gpus [i], (NvU32 *)&foundry));
      NVAPI_CALL (GPU_GetTotalTPCCount (gpus [i], (NvU32 *)&foundry));
      NVAPI_CALL (GPU_GetTotalSMCount (gpus [i], &SM));
#endif



#if 1
      /*
      wchar_t wszVendor    [64];
      wchar_t wszSubVendor [64];
      wchar_t wszChipset   [64];

      MultiByteToWideChar (CP_OEMCP, 0, chipset.szVendorName,       -1, wszVendor,    64);
      MultiByteToWideChar (CP_OEMCP, 0, chipset.szSubSysVendorName, -1, wszSubVendor, 64);
      MultiByteToWideChar (CP_OEMCP, 0, chipset.szChipsetName,      -1, wszChipset,   64);
      */

      NvU32 perf_flags;
      NVAPI_CALL (GPU_GetPerfDecreaseInfo (gpus [i], &perf_flags));

      std::wstring perf_str;
      if (perf_flags == NV_GPU_PERF_DECREASE_NONE)
        perf_str = L"\r\n                   (No Slowdown)";
      else {
        if (perf_flags & NV_GPU_PERF_DECREASE_REASON_THERMAL_PROTECTION)
          perf_str += L"\r\n                   (Thermal)";
        if (perf_flags & NV_GPU_PERF_DECREASE_REASON_POWER_CONTROL)
          perf_str += L"\r\n                   (Power Saving)";
        if (perf_flags & NV_GPU_PERF_DECREASE_REASON_AC_BATT)
          perf_str += L"\r\n                   (Battery)";
        if (perf_flags & NV_GPU_PERF_DECREASE_REASON_API_TRIGGERED)
          perf_str += L"\r\n                   (API)";
        if (perf_flags & NV_GPU_PERF_DECREASE_REASON_INSUFFICIENT_POWER)
          perf_str += L"\r\n                   (Power Supply)";
      }
#endif

      swprintf (adapters, L"%sNvAPI Adapter %02u:\r\n"
        L"----------------------------------------\r\n\r\n"
        L"  %s  < %s >\r\n\r\n"
        L"    %4.1f GiB Dedicated VRAM\r\n"
        L"    %4.1f GiB Shared Memory (GART)\r\n\r\n"

        L"    Memory Type..: %u-bit %s\r\n"
        L"    Memory Banks.: %u\r\n"
        L"    Driver Model.: WDDM %1u.%02u\r\n"
        //L"    ROP Count....: %u\r\n"
        L"    CUDA Cores...: %u\r\n"
        L"    Shader Pipes.: %u:%u\r\n"
        //L"    TPC:SM:SP....: %u:%u:%u\r\n"
        L"    SLI..........: %s (%d adapters)\r\n\r\n"

        L"    Power State..: %s\r\n\r\n"

        L" ╔══════════╦══════╤═══════╤═══════╗\r\n"
        L" ║ Clock    ║ Base │  ▲▲▲  │  Now  ║\r\n"
        L" ║     Type ║  ▬▬  │ Boost │ (GHz) ║\r\n"
        L" ╠══════════╬══════╪═══════╪═══════╣\r\n"
        L" ║   GPU    ║ %3.2f │ %4.3f │ %4.3f ║\r\n"
        L" ╟──────────╫──────┼───────┼───────╢\r\n"
        L" ║  Memory  ║ %3.2f │ %4.3f │ %4.3f ║\r\n"
        L" ╚══════════╩══════╧═══════╧═══════╝\r\n",

        adapters,
        i,
        //adapter_descs [i].VendorId,
        adapter_descs [i].Description,
        wszCodename,
        (float)adapter_descs [i].DedicatedVideoMemory / 1024.0f / 1024.0f / 1024.0f,
        (float)adapter_descs [i].SharedSystemMemory / 1024.0f / 1024.0f / 1024.0f,
        memory_type.width, ram_type > 7 ? L"GDDR5" : L"GDDR3",
        mem_parts,
        (model >> 12) & 0xf, (model >> 16) & 0xf,
        cuda_cores,
        pipes, subpipes,
        //TPC, SM, SP,
        adapter_descs [i].AdapterLuid.LowPart > 1 ? L"Yes" : L"No",
        adapter_descs [i].AdapterLuid.LowPart > 1 ? adapter_descs [i].AdapterLuid.LowPart : 0,
        perf_str.c_str (),
        base_gpu, boost_gpu, current_gpu,
        base_mem, boost_mem, current_mem
        );

      ++i;

      // Extra line-break at end of NvAPI adapter list
      if (*adapter_descs [i].Description == L'\0') {
        swprintf (adapters, L"%s\r\n", adapters);
        break;
      }

      swprintf (adapters, L"%s\r\n", adapters);
    }

  }
  
  IDXGIFactory* pFactory = NULL;
  CreateDXGIFactory (__uuidof(IDXGIFactory), (void **)&pFactory);

  IDXGIAdapter* pDXGIAdapter;
  for (UINT i = 0; pFactory->EnumAdapters (i, &pDXGIAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    DXGI_ADAPTER_DESC adapterDesc;
    pDXGIAdapter->GetDesc (&adapterDesc);

    // Skip Microsoft's virtual adapters...
    if (adapterDesc.VendorId != 0x1414) {
      swprintf (adapters, L"%sDXGI Adapter %02u (Vendor ID: %04X):\r\n"
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

  return std::wstring (adapters);
}