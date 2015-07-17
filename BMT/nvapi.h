#ifndef __BMT__NVAPI_H__
#define __BMT__NVAPI_H__

#include "nvapi/nvapi.h"
#include <Windows.h>

#include <cstring>
#include <string>

struct DXGI_ADAPTER_DESC;

namespace bmt {
namespace NVAPI {

  BOOL InitializeLibrary (void);
  BOOL UnloadLibrary     (void);

  int  CountPhysicalGPUs (void);
  DXGI_ADAPTER_DESC*
       EnumGPUs_DXGI     (void);

  std::wstring
       GetDriverVersion  (void);

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
