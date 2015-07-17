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