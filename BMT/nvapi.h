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
  extern bool nv_hardware;

  // Was the tweak tool started while BMT was configured to run the game fullscreen?
  extern bool bmt_fullscreen;

}
}


#endif /* __BMT__NVAPI_H__ */
