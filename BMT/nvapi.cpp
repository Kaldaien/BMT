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

#include <d3d11.h>

#include "nvapi.h"
#include "nvapi/NvApiDriverSettings.h"

#include <Windows.h>
#include <dxgi.h>
#include <string>

#include "utility.h"

#include "resource.h"
#include <windowsx.h>

#pragma comment (lib, "nvapi/amd64/nvapi64.lib")

using namespace bmt;
using namespace bmt::NVAPI;

static bool nvapi_silent = false;
//static bool support_mfaa = false;

#define NVAPI_SILENT()  { nvapi_silent = true;  }
#define NVAPI_VERBOSE() { nvapi_silent = false; }

#if 0
#define NVAPI_CALL(x) { NvAPI_Status ret = NvAPI_##x;        \
                        if (nvapi_silent != true &&          \
                                     ret != NVAPI_OK)        \
                          BMT_MessageBox (                   \
                            BMT_NVAPI_ErrorMessage (         \
                              ret, #x, __LINE__,             \
                              __FUNCTION__, __FILE__),       \
                            L"Error Calling NVAPI Function", \
                            MB_OK | MB_ICONASTERISK );       \
                      }
#else
#define NVAPI_CALL(x) { NvAPI_Status ret = NvAPI_##x; if (nvapi_silent != true && ret != NVAPI_OK) BMT_MessageBox (ErrorMessage (ret, #x, __LINE__, __FUNCTION__, __FILE__), L"Error Calling NVAPI Function", MB_OK | MB_ICONASTERISK ); }
#define NVAPI_CALL2(x,y) { ##y = NvAPI_##x; if (nvapi_silent != true && ##y != NVAPI_OK) BMT_MessageBox (ErrorMessage (##y, #x, __LINE__, __FUNCTION__, __FILE__).c_str (), L"Error Calling NVAPI Function", MB_OK | MB_ICONASTERISK); }
#endif

#define NVAPI_SET_DWORD(x,y,z) (x).version = NVDRS_SETTING_VER; (x).settingId = (y); (x).settingType = NVDRS_DWORD_TYPE; (x).u32CurrentValue = (z);


std::wstring
NVAPI::ErrorMessage (_NvAPI_Status err,
                     const char*   args,
                     UINT          line_no,
                     const char*   function_name,
                     const char*   file_name)
{
  char szError [64];

  NvAPI_GetErrorMessage (err, szError);

  wchar_t wszError          [64];
  wchar_t wszFile           [256];
  wchar_t wszFunction       [256];
  wchar_t wszArgs           [256];
  wchar_t wszFormattedError [1024];

  MultiByteToWideChar (CP_OEMCP, 0, szError,       -1, wszError,     64);
  MultiByteToWideChar (CP_OEMCP, 0, file_name,     -1, wszFile,     256);
  MultiByteToWideChar (CP_OEMCP, 0, function_name, -1, wszFunction, 256);
  MultiByteToWideChar (CP_OEMCP, 0, args,          -1, wszArgs,     256);
  *wszFormattedError = L'\0';

  swprintf ( wszFormattedError, 1024,
              L"Line %u of %s (in %s (...)):\n"
              L"------------------------\n\n"
              L"NvAPI_%s\n\n\t>> %s <<",
               line_no,
                wszFile,
                 wszFunction,
                  wszArgs,
                   wszError );

  return wszFormattedError;
}

class nvcfg_SLI
{
public:
  nvcfg_SLI (void);

  bool setup_ui  (HWND hDlg);

  int  poll_mode (void);

//protected:
  NVDRS_SETTING mode;

private:
  HWND hWndMode;
  HWND hWndFramePacing;
} *sli = nullptr;

class nvcfg_VSYNC
{
public:
  nvcfg_VSYNC (void);

  bool setup_ui      (HWND hDlg);

  int  poll_mode     (void);
  int  poll_smooth   (void);
  int  poll_adaptive (void);

//protected:
  NVDRS_SETTING mode;
  NVDRS_SETTING smooth;   // AFR Smoothing (frame pacing)
  NVDRS_SETTING adaptive; // Tear Control

private:
  HWND hWndVSYNC;
  HWND hWndSmooth;
  HWND hWndAdaptive;
} *vsync = nullptr;

class nvcfg_LODBias
{
public:
  nvcfg_LODBias (void);

  bool setup_ui            (HWND hDlg);

  int  poll_adjust         (void);
  bool poll_allow_negative (void);

//protected:
  NVDRS_SETTING adjust;
  NVDRS_SETTING allow_negative;

private:
  HWND hWndAdjust;
  HWND hWndAllowNegative;
} *lodbias = nullptr;

class nvcfg_Miscellaneous {
public:
  nvcfg_Miscellaneous (void);

  bool setup_ui             (HWND hDlg);

//int  poll_mfaa            (void);
  int  poll_prerender_limit (void);
  int  poll_power_policy    (void);

//protected:
  NVDRS_SETTING prerender_limit;
  NVDRS_SETTING power_policy;
  NVDRS_SETTING mfaa;

private:
  HWND hWndPreRenderLimit;
  HWND hWndPowerPolicy;
//HWND hWndUseMFAA;
} *misc = nullptr;

int
bmt::NVAPI::CountPhysicalGPUs (void)
{
  static int nv_gpu_count = -1;

  if (nv_gpu_count == -1) {
    if (nv_hardware) {
      NvPhysicalGpuHandle gpus [64];
      NvU32               gpu_count = 0;

      NVAPI_CALL (EnumPhysicalGPUs (gpus, &gpu_count));

      nv_gpu_count = gpu_count;
    }
    else {
      nv_gpu_count = 0;
    }
  }

  return nv_gpu_count;
}

int
NVAPI::CountSLIGPUs (void)
{
  static int nv_sli_count = -1;

  DXGI_ADAPTER_DESC* adapters = NVAPI::EnumGPUs_DXGI ();

  if (nv_sli_count == -1) {
    if (nv_hardware) {
      while (adapters != nullptr) {
        if (adapters->AdapterLuid.LowPart > 1)
          nv_sli_count++;

        ++adapters;

        if (*adapters->Description == '\0')
          break;
      }
    }
  }

  return nv_sli_count;
}

/**
 * These were hoisted out of EnumGPUs_DXGI (...) to reduce stack size.
 **/
static DXGI_ADAPTER_DESC   _nv_dxgi_adapters [64];
static NvPhysicalGpuHandle _nv_dxgi_gpus     [64];
static NvPhysicalGpuHandle phys [64];

// This function does much more than it's supposed to -- consider fixing that!
DXGI_ADAPTER_DESC*
bmt::NVAPI::EnumGPUs_DXGI (void)
{
  // Only do this once...
  static bool enumerated = false;

  // Early-out if this was already called once before.
  if (enumerated)
    return _nv_dxgi_adapters;

  if (! nv_hardware) {
    enumerated = true;
    *_nv_dxgi_adapters [0].Description = L'\0';
    return _nv_dxgi_adapters;
  }

  NvU32 gpu_count     = 0;

  NVAPI_CALL (EnumPhysicalGPUs (_nv_dxgi_gpus, &gpu_count));

  for (int i = 0; i < CountPhysicalGPUs (); i++) {
    DXGI_ADAPTER_DESC adapterDesc;

    NvAPI_ShortString name;

    int   sli_group = 0;
    int   sli_size  = 0;

    NVAPI_CALL (EnumPhysicalGPUs (_nv_dxgi_gpus,     &gpu_count));

    NvU32              phys_count;
    NvLogicalGpuHandle logical;

    NVAPI_CALL (GetLogicalGPUFromPhysicalGPU  (_nv_dxgi_gpus [i], &logical));
    NVAPI_CALL (GetPhysicalGPUsFromLogicalGPU (logical, phys, &phys_count));

    sli_group = (int)logical;
    sli_size  = phys_count;

    NVAPI_CALL (GPU_GetFullName (_nv_dxgi_gpus [i], name));

    NV_DISPLAY_DRIVER_MEMORY_INFO meminfo;
    meminfo.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER;

    NVAPI_CALL (GPU_GetMemoryInfo (_nv_dxgi_gpus [i], &meminfo));

    MultiByteToWideChar (CP_OEMCP, 0, name, -1, adapterDesc.Description, 64);

    adapterDesc.VendorId = 0x10de;

    adapterDesc.AdapterLuid.HighPart = sli_group;
    adapterDesc.AdapterLuid.LowPart  = sli_size;

    // NVIDIA's driver measures these numbers in KiB (to store as a 32-bit int)
    //  * We want the numbers in bytes (64-bit)
    adapterDesc.DedicatedVideoMemory  = (size_t)meminfo.dedicatedVideoMemory * 1024;
    adapterDesc.DedicatedSystemMemory = (size_t)meminfo.systemVideoMemory    * 1024;
    adapterDesc.SharedSystemMemory    = (size_t)meminfo.sharedSystemMemory   * 1024;

    _nv_dxgi_adapters [i] = adapterDesc;
  }

  *_nv_dxgi_adapters [gpu_count].Description = L'\0';

  NvDRSSessionHandle hSession;
  NVAPI_CALL (DRS_CreateSession (&hSession));

  NvDRSProfileHandle hProfile;

  NVDRS_APPLICATION app;
  app.version = NVDRS_APPLICATION_VER;

  NVAPI_CALL (DRS_LoadSettings (hSession));

  //extern std::wstring executable;

  NvAPI_Status ret;

  NVAPI_CALL2 (DRS_FindApplicationByName (hSession, (NvU16 *)L"batmanak.exe", &hProfile, &app), ret);

#if 0
  NvU32 max_count = 2048;
  NvU32 setting_ids [2048];
  NvAPI_DRS_EnumAvailableSettingIds (setting_ids, &max_count);

  for (int i = 0; i < max_count; i++) {
    wchar_t setting [2048];
    NvAPI_DRS_GetSettingNameFromId (setting_ids [i], (NvAPI_UnicodeString *)setting);
    MessageBox (NULL, setting, L"Driver Setting", MB_OK);
  }
#endif

  if (ret == NVAPI_OK) {
    NVAPI_SILENT ();

  //NVAPI_CALL2 (DRS_GetSetting (hSession, hProfile, MAXWELL_B_SAMPLE_INTERLEAVE_ID, &misc->mfaa), ret);

  //if (ret == NVAPI_OK)
    //support_mfaa = true;

    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, PRERENDERLIMIT_ID,   &misc->prerender_limit));
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, PREFERRED_PSTATE_ID, &misc->power_policy));

    // Don't load this stuff if there's only 1 GPU...
    if (CountPhysicalGPUs () > 1) {
      NVAPI_CALL (DRS_GetSetting (hSession, hProfile, SLI_RENDERING_MODE_ID, &sli->mode));
      NVAPI_CALL (DRS_GetSetting (hSession, hProfile, VSYNCSMOOTHAFR_ID,     &vsync->smooth));
    }

    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, VSYNCMODE_ID,        &vsync->mode));
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, VSYNCTEARCONTROL_ID, &vsync->adaptive));

    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, LODBIASADJUST_ID,               &lodbias->adjust));
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, PS_TEXFILTER_NO_NEG_LODBIAS_ID, &lodbias->allow_negative));

    NVAPI_VERBOSE ();
  }

  NVAPI_CALL (DRS_DestroySession (hSession));

  enumerated = true;

  return _nv_dxgi_adapters;
}


nvcfg_SLI::nvcfg_SLI (void)
{
  mode.version = NVDRS_SETTING_VER;
}

bool
nvcfg_SLI::setup_ui (HWND hDlg)
{
  hWndMode        = GetDlgItem (hDlg, IDC_SLI_MODE);
  hWndFramePacing = GetDlgItem (hDlg, IDC_SMOOTH_VSYNC);

  if (CountSLIGPUs () < 2) {
    ComboBox_Enable (hWndMode,        false);
    Button_Enable   (hWndFramePacing, false);
  }
  else {
    ComboBox_Enable (hWndMode,        true);

    // Pretty pointless in windowed-mode...
    if (! bmt_fullscreen)
      Button_Enable (hWndFramePacing, false);
    else
      Button_Enable (hWndFramePacing, true);
  }

  ComboBox_ResetContent (hWndMode);

  ComboBox_InsertString (hWndMode, 0, L"* Driver Auto-Select");
  ComboBox_InsertString (hWndMode, 1, L"Force Single-GPU");
  ComboBox_InsertString (hWndMode, 2, L"* Alternate Frame Render (1)");
  ComboBox_InsertString (hWndMode, 3, L"* Alternate Frame Render (2)");
  ComboBox_InsertString (hWndMode, 4, L"* Split Frame Rendering");
  ComboBox_InsertString (hWndMode, 5, L"* Alternating Split Frame Rendering");

  ComboBox_SetCurSel (hWndMode, mode.u32CurrentValue);

  return true;
}

int
nvcfg_SLI::poll_mode (void)
{
  return ComboBox_GetCurSel (hWndMode);
}

nvcfg_VSYNC::nvcfg_VSYNC (void)
{
  mode.version     = NVDRS_SETTING_VER;
  smooth.version   = NVDRS_SETTING_VER;
  adaptive.version = NVDRS_SETTING_VER;
}

bool
nvcfg_VSYNC::setup_ui (HWND hDlg)
{
  hWndVSYNC    = GetDlgItem (hDlg, IDC_VSYNC);
  hWndSmooth   = GetDlgItem (hDlg, IDC_SMOOTH_VSYNC);
  hWndAdaptive = GetDlgItem (hDlg, IDC_ADAPTIVE_VSYNC);

  // Pretty pointless in windowed-mode...
  if (! bmt_fullscreen) {
    EnableWindow (hWndVSYNC,    false);
    EnableWindow (hWndAdaptive, false);
  }
  else {
    EnableWindow (hWndAdaptive, true);
    EnableWindow (hWndVSYNC,    true);
  }

  ComboBox_ResetContent (hWndVSYNC);

  ComboBox_InsertString (hWndVSYNC, 0, L"Use Application Settings");
  ComboBox_InsertString (hWndVSYNC, 1, L"Force Off");
  ComboBox_InsertString (hWndVSYNC, 2, L"Force On");
  ComboBox_InsertString (hWndVSYNC, 3, L"Force On (1/2 Refresh)");
  ComboBox_InsertString (hWndVSYNC, 4, L"Force On (1/3 Refresh)");
  ComboBox_InsertString (hWndVSYNC, 5, L"Force On (1/4 Refresh)");

  switch (mode.u32CurrentValue) {
  case VSYNCMODE_PASSIVE:
    ComboBox_SetCurSel (hWndVSYNC, 0);
    break;

  case VSYNCMODE_FORCEOFF:
    ComboBox_SetCurSel (hWndVSYNC, 1);
    break;

  case VSYNCMODE_FORCEON:
    ComboBox_SetCurSel (hWndVSYNC, 2);
    break;

  case VSYNCMODE_FLIPINTERVAL2:
    ComboBox_SetCurSel (hWndVSYNC, 3);
    break;

  case VSYNCMODE_FLIPINTERVAL3:
    ComboBox_SetCurSel (hWndVSYNC, 4);
    break;

  case VSYNCMODE_FLIPINTERVAL4:
    ComboBox_SetCurSel (hWndVSYNC, 5);
    break;
  }

  if (smooth.u32CurrentValue == 1)
    Button_SetCheck (hWndSmooth, true);
  else
    Button_SetCheck (hWndSmooth, false);

  if (adaptive.u32CurrentValue == 0x99941284)
    Button_SetCheck (hWndAdaptive, true);
  else
    Button_SetCheck (hWndAdaptive, false);

  return true;
}

int
nvcfg_VSYNC::poll_adaptive (void)
{
  if (Button_GetCheck (hWndAdaptive))
    return 0x99941284;
  else
    return 0x96861077;
}

int
nvcfg_VSYNC::poll_smooth (void)
{
  return Button_GetCheck (hWndSmooth);
}

int
nvcfg_VSYNC::poll_mode (void)
{
  int sel = ComboBox_GetCurSel (hWndVSYNC);

  switch (sel) {
    case 0:
      return VSYNCMODE_PASSIVE;
    case 1:
      return VSYNCMODE_FORCEOFF;
    case 2:
      return VSYNCMODE_FORCEON;
    case 3:
      return VSYNCMODE_FLIPINTERVAL2;
    case 4:
      return VSYNCMODE_FLIPINTERVAL3;
    case 5:
      return VSYNCMODE_FLIPINTERVAL4;
    default:
      return VSYNCMODE_PASSIVE;
  }
}


nvcfg_LODBias::nvcfg_LODBias (void)
{
  adjust.version         = NVDRS_SETTING_VER;
  allow_negative.version = NVDRS_SETTING_VER;
}

bool
nvcfg_LODBias::setup_ui (HWND hDlg)
{
  hWndAdjust = GetDlgItem (hDlg, IDC_COMBO5);

  ComboBox_Enable (hWndAdjust, true);

  ComboBox_ResetContent (hWndAdjust);

  ComboBox_InsertString (hWndAdjust, 0, L"Neutral");
  ComboBox_InsertString (hWndAdjust, 1, L"Sharp (-1)");
  ComboBox_InsertString (hWndAdjust, 2, L"Sharper (-2)");
  ComboBox_InsertString (hWndAdjust, 3, L"Sharpest (-3)");

  if ((int32_t)adjust.u32CurrentValue < -127 || (int32_t)adjust.u32CurrentValue > 0)
    ComboBox_SetCurSel (hWndAdjust, 3);
  else
  {
    switch ((uint32_t)adjust.u32CurrentValue)
    {
      case 0xFFFFFFE8:
        ComboBox_SetCurSel (hWndAdjust, 3);
        break;
      case 0xFFFFFFF0:
        ComboBox_SetCurSel (hWndAdjust, 2);
        break;
      case 0xFFFFFFF8:
        ComboBox_SetCurSel (hWndAdjust, 1);
        break;
      default:
        ComboBox_SetCurSel (hWndAdjust, 0);
        break;
    }
  }

  hWndAllowNegative = GetDlgItem (hDlg, IDC_NEGATIVE_LOD);

  // This is actually reversed because of the setting name in the driver...
  if (allow_negative.u32CurrentValue)
    Button_SetCheck (hWndAllowNegative, false);
  else
    Button_SetCheck (hWndAllowNegative, true);

  return true;
}

int
nvcfg_LODBias::poll_adjust (void)
{
  switch (ComboBox_GetCurSel (hWndAdjust))
  {
  case 3:
    return 0xFFFFFFE8;
  case 2:
    return 0xFFFFFFF0;
  case 1:
    return 0xFFFFFFF8;
  default:
    return 0;
  }
}

bool
nvcfg_LODBias::poll_allow_negative (void)
{
  if (Button_GetCheck (hWndAllowNegative))
    return false;
  else
    return true;
}


nvcfg_Miscellaneous::nvcfg_Miscellaneous (void)
{
  prerender_limit.version = NVDRS_SETTING_VER;
  power_policy.version    = NVDRS_SETTING_VER;
//mfaa.version            = NVDRS_SETTING_VER;
}

bool
nvcfg_Miscellaneous::setup_ui (HWND hDlg)
{
  hWndPreRenderLimit = GetDlgItem (hDlg, IDC_PRERENDERED_FRAMES);
  hWndPowerPolicy    = GetDlgItem (hDlg, IDC_POWER_POLICY);
//hWndUseMFAA        = GetDlgItem (hDlg, IDC_CHECK2);

  ComboBox_ResetContent (hWndPreRenderLimit);

  ComboBox_InsertString (hWndPreRenderLimit, 0, L"* Use Application Settings");
  ComboBox_InsertString (hWndPreRenderLimit, 1, L"1 Frame");
  ComboBox_InsertString (hWndPreRenderLimit, 2, L"* 2 Frames");
  ComboBox_InsertString (hWndPreRenderLimit, 3, L"* 3 Frames");
  ComboBox_InsertString (hWndPreRenderLimit, 4, L"* 4 Frames");
  ComboBox_InsertString (hWndPreRenderLimit, 5, L"* 5 Frames");
  ComboBox_InsertString (hWndPreRenderLimit, 6, L"* 6 Frames");

  if (prerender_limit.u32CurrentValue > 6 || prerender_limit.u32CurrentValue < 0)
    ComboBox_SetCurSel (hWndPreRenderLimit, 0);
  else
    ComboBox_SetCurSel (hWndPreRenderLimit, prerender_limit.u32CurrentValue);

  ComboBox_ResetContent (hWndPowerPolicy);

  ComboBox_InsertString (hWndPowerPolicy, 0, L"Adaptive");
  ComboBox_InsertString (hWndPowerPolicy, 1, L"Maximum Performance");
  ComboBox_InsertString (hWndPowerPolicy, 2, L"Driver Controlled");
  ComboBox_InsertString (hWndPowerPolicy, 3, L"Consistent Performance");
  ComboBox_InsertString (hWndPowerPolicy, 4, L"Maximum Power Saving");

  ComboBox_SetCurSel (hWndPowerPolicy, power_policy.u32CurrentValue);

//Button_Enable (hWndUseMFAA, support_mfaa);

//if (support_mfaa && mfaa.u32CurrentValue)
//  Button_SetCheck (hWndUseMFAA, true);
//else
//  Button_SetCheck (hWndUseMFAA, false);

  return true;
}

int
nvcfg_Miscellaneous::poll_prerender_limit (void)
{
  return ComboBox_GetCurSel (hWndPreRenderLimit);
}

int
nvcfg_Miscellaneous::poll_power_policy (void)
{
  return ComboBox_GetCurSel (hWndPowerPolicy);
}

//int
//nvcfg_Miscellaneous::poll_mfaa (void)
//{
//  return Button_GetCheck (hWndUseMFAA);
//}


void SaveDriverTweaksNV (HWND hDlg);

INT_PTR
CALLBACK
DriverConfigNV (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      extern HICON nv_icon;
      SendMessage (hDlg, WM_SETICON, ICON_BIG,   (LPARAM)nv_icon);
      SendMessage (hDlg, WM_SETICON, ICON_SMALL, (LPARAM)nv_icon);

      sli->setup_ui     (hDlg);
      vsync->setup_ui   (hDlg);
      lodbias->setup_ui (hDlg);
      misc->setup_ui    (hDlg);

      Edit_SetText (GetDlgItem (hDlg, IDC_QUALITY_DESC),
        L"A negative LOD bias will sharpen textures throughout the game, but creates artifacts on glass and various other surfaces... it is intended to make signs easier to read.\r\n\r\n"
        );//L"MFAA requires a Maxwell \"B\"-class NVIDIA GPU (9xx series) and SLI must be disabled; enabling it here does not guarantee it will work.\n");
    }

    case WM_COMMAND:
    {
      if (LOWORD (wParam) == IDOK)
      {
        SaveDriverTweaksNV (hDlg);
        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }

      if (LOWORD (wParam) == IDCANCEL)
      {
        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }
    }
  }

  return (INT_PTR)FALSE;
}


#include "nvapi/nvapi_lite_common.h"



void SaveDriverTweaksNV (HWND hDlg)
{
  NvDRSSessionHandle hSession;
  NVAPI_CALL (DRS_CreateSession (&hSession));

  NvDRSProfileHandle hProfile;

  NVDRS_APPLICATION app;
  app.version = NVDRS_APPLICATION_VER;

  NVAPI_CALL (DRS_LoadSettings (hSession));

  extern std::wstring executable;

  // We expect to fail here, just go with it if it happens... we can fix it.
  NVAPI_SILENT ();

  NvAPI_Status ret;
  NVAPI_CALL2 (DRS_FindApplicationByName (hSession, (NvU16 *)L"batmanak.exe", &hProfile, &app), ret);

  if (ret == NVAPI_EXECUTABLE_NOT_FOUND) {
    NVAPI_CALL2 (DRS_FindApplicationByName (hSession, (NvU16 *)L"bmgame.exe", &hProfile, &app), ret);

    NVAPI_VERBOSE ();

    if (ret == NVAPI_OK) {
      lstrcpyW ((wchar_t *)app.appName, L"batmanak.exe");
      lstrcpyW ((wchar_t *)app.userFriendlyName, L"Batman: Arkham Knight");
      app.isPredefined = false;

      NvAPI_DRS_CreateApplication (hSession, hProfile, &app);
    }
    else {
      return;
    }
  }

  NVAPI_VERBOSE ();

  NvU32 setting_id = 0;

  NVAPI_CALL (DRS_GetSettingIdFromName ((NvU16 *)PRERENDERLIMIT_STRING, &setting_id));

  if (ret == NVAPI_OK) {
    NVAPI_SET_DWORD (misc->prerender_limit, setting_id, misc->poll_prerender_limit ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &misc->prerender_limit));

    NVAPI_SET_DWORD (misc->power_policy, PREFERRED_PSTATE_ID, misc->poll_power_policy ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &misc->power_policy));

    // Don't save this stuff if there's only 1 GPU...
    if (CountSLIGPUs () > 1) {
      NVAPI_SET_DWORD (sli->mode, SLI_RENDERING_MODE_ID, sli->poll_mode ());
      NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &sli->mode));

      NVAPI_SET_DWORD (vsync->smooth, VSYNCSMOOTHAFR_ID, vsync->poll_smooth ());
      NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &vsync->smooth));
    }

    NVAPI_SET_DWORD (vsync->mode, VSYNCMODE_ID, vsync->poll_mode ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &vsync->mode));

    NVAPI_SET_DWORD (vsync->adaptive, VSYNCTEARCONTROL_ID, vsync->poll_adaptive ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &vsync->adaptive));

    NVDRS_SETTING lodbias_auto_adjust;
    NVAPI_SET_DWORD (lodbias_auto_adjust, AUTO_LODBIASADJUST_ID, 0);
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &lodbias_auto_adjust));

    NVAPI_SET_DWORD (lodbias->adjust, LODBIASADJUST_ID, lodbias->poll_adjust ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &lodbias->adjust));

    NVAPI_SET_DWORD (lodbias->allow_negative, PS_TEXFILTER_NO_NEG_LODBIAS_ID, lodbias->poll_allow_negative ())
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &lodbias->allow_negative));

//    if (support_mfaa) {
//      NVAPI_SET_DWORD (misc->mfaa, MAXWELL_B_SAMPLE_INTERLEAVE_ID, misc->poll_mfaa ());
//      NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &misc->mfaa));
//    }
  }

  NVAPI_CALL (DRS_SaveSettings   (hSession));
  NVAPI_CALL (DRS_DestroySession (hSession));
}

std::wstring
NVAPI::GetDriverVersion (NvU32* pVer)
{
  NvU32             ver;
  NvAPI_ShortString ver_str;       // ANSI
  wchar_t           ver_wstr [64]; // Unicode

  NvAPI_SYS_GetDriverAndBranchVersion (&ver, ver_str);

  // The driver-branch string's not particularly user frieldy,
  //   let's do this the right way and report a number the end-user
  //     is actually going to recognize...
  swprintf (ver_wstr, 64, L"%u.%u", ver / 100, ver % 100);
  //MultiByteToWideChar (CP_OEMCP, 0, ver_str, -1, ver_wstr, 64);

  if (pVer != NULL)
    *pVer = ver;

  return ver_wstr;
}


BOOL bLibShutdown = FALSE;
BOOL bLibInit     = FALSE;

BOOL
NVAPI::UnloadLibrary (void)
{
  if (bLibInit == TRUE && bLibShutdown == FALSE) {
    // Whine very loudly if this fails, because that's not
    //   supposed to happen!
    NVAPI_VERBOSE ()

    NvAPI_Status ret;

    NVAPI_CALL2 (Unload (), ret);

    if (ret == NVAPI_OK) {
      bLibShutdown = TRUE;
      bLibInit     = FALSE;

      delete lodbias; lodbias = nullptr;
      delete vsync;   vsync   = nullptr;
      delete sli;     sli     = nullptr;
      delete misc;    misc    = nullptr;
    }
  }

  return bLibShutdown;
}

BOOL
NVAPI::InitializeLibrary (void)
{
  // It's silly to call this more than once, but not necessarily
  //  an error... just ignore repeated calls.
  if (bLibInit == TRUE)
    return TRUE;

  // If init is not false and not true, it's because we failed to
  //   initialize the API once before. Just return the failure status
  //     again.
  if (bLibInit != FALSE)
    return FALSE;

  NvAPI_Status ret;

  // We want this error to be silent, because this tool works on AMD GPUs too!
  NVAPI_SILENT ()
  {
    NVAPI_CALL2 (Initialize (), ret);
  }
  NVAPI_VERBOSE ()

    if (ret != NVAPI_OK) {
      nv_hardware = false;
      bLibInit    = TRUE + 1; // Clearly this isn't a boolean, it just looks like one
      return FALSE;
    }

  sli     = new nvcfg_SLI           ();
  vsync   = new nvcfg_VSYNC         ();
  lodbias = new nvcfg_LODBias       ();
  misc    = new nvcfg_Miscellaneous ();

  if (! CheckDriverVersion ()) {
    BMT_MessageBox (L"WARNING:  Your display drivers are too old to play this game!\n",
                    L"Please update your display drivers (Minimum Version = 353.30)",
                    MB_OK | MB_ICONEXCLAMATION);
  }

  return (bLibInit = TRUE);
}

bool
TEST_SLI (void)
{
  return false;
  //NvAPI_D3D_GetCurrentSLIState ()
}

#if 0
std::wstring
EDID_Audio_CodecName (uint8_t SAD1)
{
  switch ((SAD1 & 0x78) >> 3)
  {
  case 1:
    return L"LPCM";
  case 2:
    return L"AC-3";
  case 3:
    return L"MPEG1";
  case 4:
    return L"MP3";
  case 5:
    return L"MPEG2";
  case 6:
    return L"AAC";
  case 7:
    return L"DTS";
  case 8:
    return L"ATRAC";
  case 9:
    return L"SACD";
  case 10:
    return L"Dolby Digital+";
  case 11:
    return L"DTS-HD";
  case 12:
    return L"Dolby TrueHD";
  case 13:
    return L"DST Audio";
  case 14:
    return L"Microsoft WMA Pro";
  default:
    return L"INVALID";
  }
}

std::wstring
EDID_Audio_SampleRates (uint8_t SAD2)
{
  std::wstring rates = L" ";

  if (SAD2 & 0x1)
    rates += L"32 kHz ";
  if (SAD2 & 0x2)
    rates += L"44 kHz ";
  if (SAD2 & 0x4)
    rates += L"48 kHz ";
  if (SAD2 & 0x8)
    rates += L"88 kHz ";
  if (SAD2 & 0x10)
    rates += L"96 kHz ";
  if (SAD2 & 0x20)
    rates += L"176 kHz ";
  if (SAD2 & 0x40)
    rates += L"192 kHz ";

  return rates;
}

std::wstring
EDID_Audio_BitRates (uint8_t SAD3)
{
  std::wstring rates = L" ";

  if (SAD3 & 0x1)
    rates += L"16-bit ";
  if (SAD3 & 0x2)
    rates += L"20-bit ";
  if (SAD3 & 0x4)
    rates += L"24-bit ";

  return rates;
}

wchar_t wszFormat [16384];
#endif

bool
NVAPI::CheckDriverVersion (void)
{
  NvU32 ver;
  GetDriverVersion (&ver);

#if 0
  NV_MONITOR_CAPABILITIES mon_caps;
  mon_caps.version = NV_MONITOR_CAPABILITIES_VER;
  mon_caps.size = sizeof (NV_MONITOR_CAPABILITIES);

  NvPhysicalGpuHandle gpus [16];
  NvU32               gpu_count = 16;

  NV_GPU_DISPLAYIDS disp_ids [4];
  disp_ids [0].version = NV_GPU_DISPLAYIDS_VER;
  disp_ids [1].version = NV_GPU_DISPLAYIDS_VER;
  disp_ids [2].version = NV_GPU_DISPLAYIDS_VER;
  disp_ids [3].version = NV_GPU_DISPLAYIDS_VER;

  NvU32             disp_count = 4;

  NVAPI_CALL (EnumPhysicalGPUs (gpus, &gpu_count));
  NVAPI_CALL (GPU_GetConnectedDisplayIds (gpus [0], disp_ids, &disp_count, NV_GPU_CONNECTED_IDS_FLAG_SLI));

  for (int i = 0; i < disp_count; i++) {
    //NVAPI_CALL (DISP_GetMonitorCapabilities (disp_ids [i].displayId, &mon_caps));

    NV_EDID edid;
    edid.version    = NV_EDID_VER;
    edid.sizeofEDID = NV_EDID_DATA_SIZE;

    NVAPI_CALL (GPU_GetEDID (gpus [0], disp_ids [i].displayId, &edid));

    if (((uint64_t *)edid.EDID_Data) [0] == 0x00ffffffffffff00)
      MessageBox (NULL, L"We did it!", L"Neat", MB_OK);
    else
      return true;

    uint8_t* data = &edid.EDID_Data [128 + 4];
    while (true) {
#if 0
      uint8_t type = (*data & 0x7);// (*data & 0xE0) >> 5;
      uint8_t adv = (*data & 0xF8) >> 5;// (*data & 0x1F);
#else
      uint8_t type = (*data & 0xE0) >> 5;
      uint8_t adv  = (*data & 0x1F);
#endif

      data++;

      wchar_t* pwszFormat = wszFormat;

      if (type == 1) {
        //MessageBox (NULL, L"Audio Block", L"EDID Debug", MB_OK);
        uint8_t* end = data + adv;
        while (data < end) {
          pwszFormat += swprintf (pwszFormat, 16384, L"%s (%d Channels - [%s] @ (%s)))\n",
            EDID_Audio_CodecName (*data).c_str (),
              (*data & 0x7) + 1,
            EDID_Audio_SampleRates (*(data + 1)).c_str (),
            EDID_Audio_BitRates    (*(data + 2)).c_str ());

          data += 3;
        }

        MessageBox (NULL, wszFormat, L"Audio Channels", MB_OK);
      }

      if (type == 2) {
        //MessageBox (NULL, L"Video Block", L"EDID Debug", MB_OK);
        data += adv;
      }

      if (type == 3) {
        MessageBox (NULL, L"Vendor Block", L"EDID Debug", MB_OK);

        if (data [8]) {
          swprintf (wszFormat, 512, L"%d MHz Max TMDS", data [8] * 5);
          MessageBox (NULL, wszFormat, L"Pixel Clock", MB_OK);
        }

        bool latency = (data [9] & 0x80);

        if (latency) {
          swprintf (wszFormat, 512, L"(%d : %d) ms <video : audio> latency", data [10] * 2 - 1, data [11] * 2 - 1);
          MessageBox (NULL, wszFormat, L"Video Latency", MB_OK);
        }
          
        //uint8_t* end = 
        data += adv;
      }

      if (type == 4) {
        //MessageBox (NULL, L"Speaker Block", L"EDID Debug", MB_OK);
        data += adv;
      }

      if (data > &edid.EDID_Data [127] + edid.EDID_Data [128 + 2])
        break;
    }

    //NvAPI_GetAssociatedNvidiaDisplayName (disp_ids [i]., disp_name);
    //MessageBoxA (NULL, disp_name, "Test", MB_OK);

    //wchar_t wszSummary [1024];
    //disp_ids [i].connectorType

    //swprintf (wszSummary, 1024, L"%d ms Latency\n", mon_caps.data.vcdb.quantizationRangeYcc);// mon_caps.data.vsdb.supportDeepColor36bits);// mon_caps.data.vsdb.videoLatency);
    //MessageBox (NULL, wszSummary, L"Monitor Info", MB_OK);
    //mon_caps.data.vsdb.videoLatency;
  }
#endif

  return ver >= 35330;
  if (ver < 35330) {
    return false;
  }

  return true;
}

bool NVAPI::bmt_fullscreen = false;
bool NVAPI::nv_hardware    = true;