#ifndef __BMT__NVAPI_H__
#define __BMT__NVAPI_H__

#include "nvapi/nvapi.h"
#include <Windows.h>

#include <cstring>
#include <string>

struct DXGI_ADAPTER_DESC;

namespace bmt {
namespace NVAPI {

// r353_23
#define MINIMUM_DRIVER_VERSION 35330

  BOOL InitializeLibrary (void);
  BOOL UnloadLibrary     (void);

  int  CountPhysicalGPUs (void);
  DXGI_ADAPTER_DESC*
       EnumGPUs_DXGI     (void);

  std::wstring
       GetDriverVersion  (NvU32* pVer = NULL);

  // In typical NVIDIA fashion, you literally cannot
  //   run this game without updating your driver first.
  //
  //   The game doesn't ever check this, so we might as well do
  //     that too.
  bool
    CheckDriverVersion   (void);

  std::wstring
       ErrorMessage      (_NvAPI_Status err,
                          const char*   args,
                          UINT          line_no,
                          const char*   function_name,
                          const char*   file_name);

  // Guilty until proven innocent
  static bool nv_hardware = true;

}
}


#endif /* __BMT__NVAPI_H__ */
