#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS

#include "nvapi/nvapi.h"

#include <Windows.h>
#include <string>

#include "utility.h"

#include <dxgi.h>

#pragma comment (lib, "nvapi/amd64/nvapi64.lib")

bool nv_hardware  = true;
bool nvapi_silent = false;
bool support_mfaa = false;

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
#define NVAPI_CALL(x) { NvAPI_Status ret = NvAPI_##x; if (nvapi_silent != true && ret != NVAPI_OK) BMT_MessageBox (BMT_NVAPI_ErrorMessage (ret, #x, __LINE__, __FUNCTION__, __FILE__), L"Error Calling NVAPI Function", MB_OK | MB_ICONASTERISK ); }
#define NVAPI_CALL2(x,y) { ##y = NvAPI_##x; if (nvapi_silent != true && ##y != NVAPI_OK) BMT_MessageBox (BMT_NVAPI_ErrorMessage (##y, #x, __LINE__, __FUNCTION__, __FILE__).c_str (), L"Error Calling NVAPI Function", MB_OK | MB_ICONASTERISK); }
#endif

#define NVAPI_SET_DWORD(x,y,z) (x).version = NVDRS_SETTING_VER; (x).settingId = (y); (x).settingType = NVDRS_DWORD_TYPE; (x).u32CurrentValue = (z);



std::wstring
BMT_NVAPI_ErrorMessage ( _NvAPI_Status err,
                         const char*   args,
                         UINT          line_no,
                         const char*   function_name,
                         const char*   file_name )
{
  char szError [64];

  NvAPI_GetErrorMessage (err, szError);

  wchar_t wszError          [64];
  wchar_t wszFile           [256];
  wchar_t wszFunction       [256];
  wchar_t wszArgs           [256];
  wchar_t wszFormattedError [1024];

  MultiByteToWideChar (CP_OEMCP, 0, szError,       -1, wszError,    64);
  MultiByteToWideChar (CP_OEMCP, 0, file_name,     -1, wszFile,     256);
  MultiByteToWideChar (CP_OEMCP, 0, function_name, -1, wszFunction, 256);
  MultiByteToWideChar (CP_OEMCP, 0, args,          -1, wszArgs,     256);
  *wszFormattedError = L'\0';

  swprintf (wszFormattedError, L"Line %u of %s (in %s (...)):\n"
                               L"------------------------\n\n"
                               L"NvAPI_%s\n\n\t>> %s <<",
              line_no,
                wszFile,
                  wszFunction,
                    wszArgs,
                      wszError);

  return wszFormattedError;
}

void BMT_NVAPI_Init (void)
{
  NvAPI_Status ret;

  NVAPI_SILENT ()
  {
    NVAPI_CALL2 (Initialize (), ret);
  }
  NVAPI_VERBOSE ()

  if (ret != NVAPI_OK) {
    nv_hardware = false;
  }
}

#include "nvapi/NvApiDriverSettings.h"

#include "resource.h"
#include <windowsx.h>

struct sli_settings_t
{
  void init (HWND hDlg);

  int poll_mode (void);

  NVDRS_SETTING mode;

  HWND hWndMode;
  HWND hWndFramePacing;
} sli;

struct vsync_settings_t
{
  void init (HWND hDlg);

  int poll_mode (void);
  int poll_smooth (void);
  int poll_adaptive (void);

  NVDRS_SETTING mode;
  NVDRS_SETTING smooth;   // AFR Smoothing (frame pacing)
  NVDRS_SETTING adaptive; // Tear Control

  HWND hWndVSYNC;
  HWND hWndSmooth;
  HWND hWndAdaptive;
} vsync;

NVDRS_SETTING prerender_limit;

NVDRS_SETTING power_policy;

struct lodbias_settings_t
{
  void init (HWND hDlg);

  int  poll_adjust         (void);
  bool poll_allow_negative (void);

  NVDRS_SETTING adjust;
  NVDRS_SETTING allow_negative;

  HWND hWndAdjust;
  HWND hWndAllowNegative;
} lodbias;

NVDRS_SETTING mfaa;

extern size_t BMT_GetGPUVRAM (void);
extern size_t BMT_GetGART (void);

int
BMT_CountNVGPUs (void)
{
  static int  nv_gpu_count = -1;

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

DXGI_ADAPTER_DESC*
BMT_EnumNVGPUs (void)
{
  // Only do this once...
  static bool              enumerated = false;
  static DXGI_ADAPTER_DESC adapters [64];

  // Early-out if this was already called once before.
  if (enumerated)
    return adapters;

  if (! nv_hardware) {
    enumerated = true;
    *adapters [0].Description = L'\0';
    return adapters;
  }

  static NvPhysicalGpuHandle gpus [64];
  NvU32 gpu_count = 0;

  NVAPI_CALL (EnumPhysicalGPUs (gpus, &gpu_count));

  for (INT i = 0; i < BMT_CountNVGPUs (); i++) {
    DXGI_ADAPTER_DESC adapterDesc;

    NvAPI_ShortString name;

    NVAPI_CALL (GPU_GetFullName (gpus [i], name));

    NV_DISPLAY_DRIVER_MEMORY_INFO meminfo;
    meminfo.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER;

    NVAPI_CALL (GPU_GetMemoryInfo (gpus [i], &meminfo));

    MultiByteToWideChar (CP_OEMCP, 0, name, -1, adapterDesc.Description, 64);

    adapterDesc.VendorId = 0x10de;

    // NVIDIA's driver measures these numbers in KiB (to store as a 32-bit int)
    //  * We want the numbers in bytes (64-bit)
    adapterDesc.DedicatedVideoMemory  = (size_t)meminfo.dedicatedVideoMemory * 1024;
    adapterDesc.DedicatedSystemMemory = (size_t)meminfo.systemVideoMemory    * 1024;
    adapterDesc.SharedSystemMemory    = (size_t)meminfo.sharedSystemMemory   * 1024;

    adapters [i] = adapterDesc;
  }

  *adapters [gpu_count].Description = L'\0';

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

    prerender_limit.version = NVDRS_SETTING_VER;
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, PRERENDERLIMIT_ID, &prerender_limit));

    power_policy.version = NVDRS_SETTING_VER;
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, PREFERRED_PSTATE_ID, &power_policy));

    // Don't load this stuff if there's only 1 GPU...
    if (BMT_CountNVGPUs () > 1) {
      sli.mode.version = NVDRS_SETTING_VER;
      NVAPI_CALL (DRS_GetSetting (hSession, hProfile, SLI_RENDERING_MODE_ID, &sli.mode));

      vsync.smooth.version = NVDRS_SETTING_VER;
      NVAPI_CALL (DRS_GetSetting (hSession, hProfile, VSYNCSMOOTHAFR_ID, &vsync.smooth));
    }

    vsync.mode.version = NVDRS_SETTING_VER;
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, VSYNCMODE_ID, &vsync.mode));

    vsync.adaptive.version = NVDRS_SETTING_VER;
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, VSYNCTEARCONTROL_ID, &vsync.adaptive));

    lodbias.adjust.version = NVDRS_SETTING_VER;
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, LODBIASADJUST_ID, &lodbias.adjust));

    lodbias.allow_negative.version = NVDRS_SETTING_VER;
    NVAPI_CALL (DRS_GetSetting (hSession, hProfile, PS_TEXFILTER_NO_NEG_LODBIAS_ID, &lodbias.allow_negative));

    {
      NvAPI_Status ret;

      mfaa.version = NVDRS_SETTING_VER;
      NVAPI_CALL2 (DRS_GetSetting (hSession, hProfile, MAXWELL_B_SAMPLE_INTERLEAVE_ID, &mfaa), ret);

      if (ret == NVAPI_OK)
        support_mfaa = true;
    }

    NVAPI_VERBOSE ();
  }

  NVAPI_CALL (DRS_DestroySession (hSession));

  enumerated = true;

  return adapters;
}


void
sli_settings_t::init (HWND hDlg)
{
  hWndMode        = GetDlgItem (hDlg, IDC_SLI_MODE);
  hWndFramePacing = GetDlgItem (hDlg, IDC_SMOOTH_VSYNC);

  if (BMT_CountNVGPUs () < 2) {
    ComboBox_Enable (hWndMode,        false);
    Button_Enable   (hWndFramePacing, false);
  }
  else {
    ComboBox_Enable (hWndMode,        true);
    Button_Enable   (hWndFramePacing, true);
  }

  ComboBox_ResetContent (hWndMode);

  ComboBox_InsertString (hWndMode, 0, L"* Driver Auto-Select");
  ComboBox_InsertString (hWndMode, 1, L"Force Single-GPU");
  ComboBox_InsertString (hWndMode, 2, L"* Alternate Frame Render (1)");
  ComboBox_InsertString (hWndMode, 3, L"* Alternate Frame Render (2)");
  ComboBox_InsertString (hWndMode, 4, L"* Split Frame Rendering");
  ComboBox_InsertString (hWndMode, 5, L"* Alternating Split Frame Rendering");

  ComboBox_SetCurSel (hWndMode, mode.u32CurrentValue);
}

int
sli_settings_t::poll_mode (void)
{
  return ComboBox_GetCurSel (hWndMode);
}

void
vsync_settings_t::init (HWND hDlg)
{
  hWndVSYNC    = GetDlgItem (hDlg, IDC_VSYNC);
  hWndSmooth   = GetDlgItem (hDlg, IDC_SMOOTH_VSYNC);
  hWndAdaptive = GetDlgItem (hDlg, IDC_ADAPTIVE_VSYNC);

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
}

void setup_prerendered_frames (HWND hDlg)
{
  HWND prerender_ctl = GetDlgItem (hDlg, IDC_PRERENDERED_FRAMES);

  ComboBox_ResetContent (prerender_ctl);

  ComboBox_InsertString (prerender_ctl, 0, L"* Use Application Settings");
  ComboBox_InsertString (prerender_ctl, 1, L"1 Frame");
  ComboBox_InsertString (prerender_ctl, 2, L"* 2 Frames");
  ComboBox_InsertString (prerender_ctl, 3, L"* 3 Frames");
  ComboBox_InsertString (prerender_ctl, 4, L"* 4 Frames");
  ComboBox_InsertString (prerender_ctl, 5, L"* 5 Frames");
  ComboBox_InsertString (prerender_ctl, 6, L"* 6 Frames");

  if (prerender_limit.u32CurrentValue > 6 || prerender_limit.u32CurrentValue < 0)
    ComboBox_SetCurSel (prerender_ctl, 0);
  else
    ComboBox_SetCurSel (prerender_ctl, prerender_limit.u32CurrentValue);
}

int get_prerendered_frames (HWND hDlg)
{
  HWND prerender_ctl = GetDlgItem (hDlg, IDC_PRERENDERED_FRAMES);

  return ComboBox_GetCurSel (prerender_ctl);
}

void setup_power_policy (HWND hDlg)
{
  HWND pmode_ctl = GetDlgItem (hDlg, IDC_POWER_POLICY);

  ComboBox_ResetContent (pmode_ctl);

  ComboBox_InsertString (pmode_ctl, 0, L"Adaptive");
  ComboBox_InsertString (pmode_ctl, 1, L"Maximum Performance");
  ComboBox_InsertString (pmode_ctl, 2, L"Driver Controlled");
  ComboBox_InsertString (pmode_ctl, 3, L"Consistent Performance");
  ComboBox_InsertString (pmode_ctl, 4, L"Maximum Power Saving");

  ComboBox_SetCurSel (pmode_ctl, power_policy.u32CurrentValue);
}

int get_power_policy (HWND hDlg)
{
  HWND pmode_ctl = GetDlgItem (hDlg, IDC_POWER_POLICY);

  return ComboBox_GetCurSel (pmode_ctl);
}

int
vsync_settings_t::poll_adaptive (void)
{
  if (Button_GetCheck (hWndAdaptive))
    return 0x99941284;
  else
    return 0x96861077;
}

int
vsync_settings_t::poll_smooth (void)
{
  return Button_GetCheck (hWndSmooth);
}

int
vsync_settings_t::poll_mode (void)
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



void
lodbias_settings_t::init (HWND hDlg)
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
}

int
lodbias_settings_t::poll_adjust (void)
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
lodbias_settings_t::poll_allow_negative (void)
{
  if (Button_GetCheck (hWndAllowNegative))
    return false;
  else
    return true;
}

void setup_mfaa (HWND hDlg)
{
  HWND mfaa_ctl = GetDlgItem (hDlg, IDC_CHECK2);

  Button_Enable (mfaa_ctl, support_mfaa);

  if (support_mfaa && mfaa.u32CurrentValue)
    Button_SetCheck (mfaa_ctl, true);
  else
    Button_SetCheck (mfaa_ctl, false);
}

int get_mfaa (HWND hDlg)
{
  HWND mfaa_ctl = GetDlgItem (hDlg, IDC_CHECK2);

  return Button_GetCheck (mfaa_ctl);
}

void BMT_SaveNVDriverTweaks (HWND hDlg);

#include <commctrl.h>

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

      sli.init (hDlg);
      vsync.init (hDlg);
      lodbias.init (hDlg);

      setup_prerendered_frames (hDlg);
      setup_power_policy (hDlg);
      setup_mfaa (hDlg);

      Edit_SetText (GetDlgItem (hDlg, IDC_QUALITY_DESC),
        L"A negative LOD bias will sharpen textures throughout the game, but creates artifacts on glass and various other surfaces... it is intended to make signs easier to read.\r\n\r\n"
        L"MFAA requires a Maxwell \"B\"-class NVIDIA GPU (9xx series) and SLI must be disabled; enabling it here does not guarantee it will work.\n");
    }

    case WM_COMMAND:
    {
      if (LOWORD (wParam) == IDOK)
      {
        BMT_SaveNVDriverTweaks (hDlg);
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



void BMT_SaveNVDriverTweaks (HWND hDlg)
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
    prerender_limit.version = NVDRS_SETTING_VER;

    NVAPI_SET_DWORD (prerender_limit, setting_id, get_prerendered_frames (hDlg));
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &prerender_limit));

    NVAPI_SET_DWORD (power_policy, PREFERRED_PSTATE_ID, get_power_policy (hDlg));
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &power_policy));

    // Don't save this stuff if there's only 1 GPU...
    if (BMT_CountNVGPUs () > 1) {
      NVAPI_SET_DWORD (sli.mode, SLI_RENDERING_MODE_ID, sli.poll_mode ());
      NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &sli.mode));

      NVAPI_SET_DWORD (vsync.smooth, VSYNCSMOOTHAFR_ID, vsync.poll_smooth ());
      NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &vsync.smooth));
    }


    NVAPI_SET_DWORD (vsync.mode, VSYNCMODE_ID, vsync.poll_mode ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &vsync.mode));

    NVAPI_SET_DWORD (vsync.adaptive, VSYNCTEARCONTROL_ID, vsync.poll_adaptive ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &vsync.adaptive));


    NVDRS_SETTING lodbias_auto_adjust;
    NVAPI_SET_DWORD (lodbias_auto_adjust, AUTO_LODBIASADJUST_ID, 1);
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &lodbias_auto_adjust));

    NVAPI_SET_DWORD (lodbias.adjust, LODBIASADJUST_ID, lodbias.poll_adjust ());
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &lodbias.adjust));

    NVAPI_SET_DWORD (lodbias.allow_negative, PS_TEXFILTER_NO_NEG_LODBIAS_ID, lodbias.poll_allow_negative ())
    NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &lodbias.allow_negative));

    if (support_mfaa) {
      NVAPI_SET_DWORD (mfaa, MAXWELL_B_SAMPLE_INTERLEAVE_ID, get_mfaa (hDlg));
      NVAPI_CALL      (DRS_SetSetting (hSession, hProfile, &mfaa));
    }
  }

  NVAPI_CALL (DRS_SaveSettings   (hSession));
  NVAPI_CALL (DRS_DestroySession (hSession));
}

std::wstring
BMT_GetNVDriverVersion (void)
{
  NvU32             ver;
  NvAPI_ShortString ver_str;       // ANSI
  wchar_t           ver_wstr [64]; // Unicode

  NvAPI_SYS_GetDriverAndBranchVersion (&ver, ver_str);

  MultiByteToWideChar (CP_OEMCP, 0, ver_str, -1, ver_wstr, 64);

  return ver_wstr;
}