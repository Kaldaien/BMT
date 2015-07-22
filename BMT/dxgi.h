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

#ifndef __BMT__DXGI_H__
#define __BMT__DXGI_H__

#include <string>
#include <dxgi.h>

namespace bmt   {
namespace DXGI {
  enum GPUMemoryPool {
    Dedicated,
    GART
  };

  // TODO: Allow querying this per-adapter, as one would logically expect...
  size_t GetAdapterPool (GPUMemoryPool pool);
  size_t GetGART        (void);
  size_t GetVRAM        (void);

  std::wstring
         GetGPUInfo     (void);

}
}

#endif /* __BMT__DXGI_H__ */