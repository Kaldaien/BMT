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

#ifndef __BMT__WMI_H__
#define __BMT__WMI_H__

#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

namespace bmt {
namespace WMI {

  void Init    (void);
  void Cleanup (void);

  void Install (void);

  void StartMonitoring (void);
  void StopMonitoring  (void);

class EventSink : public IWbemObjectSink
{
  LONG m_lRef;
  bool bDone;

public:
   EventSink (void) { m_lRef = 0;    }
  ~EventSink (void) { bDone  = true; }

  virtual ULONG STDMETHODCALLTYPE AddRef  (void);
  virtual ULONG STDMETHODCALLTYPE Release (void);

  virtual HRESULT
    STDMETHODCALLTYPE QueryInterface (REFIID riid, void** ppv);

  virtual HRESULT STDMETHODCALLTYPE Indicate (
    LONG lObjectCount,
    IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray
    );

  // Really couldn't care less about this...
  virtual HRESULT STDMETHODCALLTYPE SetStatus (
    /* [in] */ LONG    lFlags,
    /* [in] */ HRESULT hResult,
    /* [in] */ BSTR    strParam,
    /* [in] */ IWbemClassObject __RPC_FAR *pObjParam
    );
};

}
}

#endif /* __BMT__WMI_H__ */