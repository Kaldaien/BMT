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

#include "wmi.h"
#include "xml.h"
#include "BMT.h"

using namespace bmt;
using namespace bmt::XML;
using namespace bmt::WMI;

static HRESULT              hres;
static IWbemServices*       pSvc           = NULL;
static IWbemLocator*        pLoc           = NULL;
static IUnsecuredApartment* pUnsecApp      = NULL;
static IUnknown*            pStubUnk       = NULL;
static IWbemObjectSink*     pStubSinkOpen  = NULL;
static IWbemObjectSink*     pStubSinkClose = NULL;
static EventSink*           pSink          = NULL;

void WMI::Init (void)
{
  // This is a nasty hack, probably not even worth it.
  atexit (Cleanup);

  if (FAILED (CoInitializeEx (0, COINIT_MULTITHREADED)))
    return;

  hres = CoInitializeSecurity (
    NULL,
    -1,                          // COM negotiates service
    NULL,                        // Authentication services
    NULL,                        // Reserved
    RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
    NULL,                        // Authentication info
    EOAC_NONE,                   // Additional capabilities 
    NULL                         // Reserved
    );

  if (FAILED (hres))
  {
    CoUninitialize ();
  }
}

void WMI::Install (void)
{
  // Already setup the WMI event notification system, dummy!
  if (pSink != NULL)
    return;

  hres = CoCreateInstance (
    CLSID_WbemLocator,
    0,
    CLSCTX_INPROC_SERVER,
    IID_IWbemLocator, (LPVOID *)&pLoc);

  if (FAILED (hres))
  {
    CoUninitialize ();
    return;
  }

  hres = pLoc->ConnectServer (
    _bstr_t (L"ROOT\\CIMV2"),
    NULL,
    NULL,
    0,
    NULL,
    0,
    0,
    &pSvc
    );

  if (FAILED (hres))
  {
    pLoc->Release  ();
    CoUninitialize ();
  }

  hres = CoSetProxyBlanket (
    pSvc,                        // Indicates the proxy to set
    RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
    RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
    NULL,                        // Server principal name 
    RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
    RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
    NULL,                        // client identity
    EOAC_NONE                    // proxy capabilities 
    );

  if (FAILED (hres))
  {
    pSvc->Release  ();
    pLoc->Release  ();
    CoUninitialize ();
    return;
  }

  hres = CoCreateInstance (CLSID_UnsecuredApartment, NULL,
    CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
    (void**)&pUnsecApp);

  pSink = new EventSink;
  pSink->AddRef ();

  pUnsecApp->CreateObjectStub (pSink, &pStubUnk);

  pStubUnk->QueryInterface (IID_IWbemObjectSink, (void **)&pStubSinkOpen);
  pStubUnk->QueryInterface (IID_IWbemObjectSink, (void **)&pStubSinkClose);

  extern std::wstring executable;

  hres = pSvc->ExecNotificationQueryAsync (
    _bstr_t ("WQL"),
    _bstr_t ("SELECT * "
      "FROM __InstanceCreationEvent WITHIN 1 "
      "WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.Name = 'BatmanAK.exe'"
    ),
    WBEM_FLAG_SEND_STATUS,
    NULL,
    pStubSinkOpen);

  bool failure = FAILED (hres);

  if (! failure) {
    hres = pSvc->ExecNotificationQueryAsync (
      _bstr_t ("WQL"),
      _bstr_t ("SELECT * "
        "FROM __InstanceDeletionEvent WITHIN 1 "
        "WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.Name = 'BatmanAK.exe'"
      ),
      WBEM_FLAG_SEND_STATUS,
      NULL,
      pStubSinkClose);

    failure = FAILED (hres);
  }

  // We need to FULLY cleanup WMI now, it's a lost cause.
  if (failure) {
    Cleanup ();
  }

  // Success!
}

void WMI::Cleanup (void)
{
  if (pSvc == NULL)
    return;

  if (pStubSinkOpen != NULL)
    pSvc->CancelAsyncCall (pStubSinkOpen);
  if (pStubSinkClose != NULL)
    pSvc->CancelAsyncCall (pStubSinkOpen);

  pSvc->Release           ();
  pLoc->Release           ();
  pUnsecApp->Release      ();
  pStubUnk->Release       ();
  pSink->Release          ();

  pStubSinkOpen->Release  ();
  pStubSinkClose->Release ();

  CoUninitialize          ();

  pSvc      = NULL;
  pLoc      = NULL;
  pUnsecApp = NULL;
  pStubUnk  = NULL;
  pSink     = NULL;

  pStubSinkOpen  = NULL;
  pStubSinkClose = NULL;
}


bool monitor_bmak = false;

void WMI::StartMonitoring (void)
{
  monitor_bmak = true;
}

void WMI::StopMonitoring (void)
{
  monitor_bmak = false;
}



ULONG EventSink::AddRef ()
{
  return InterlockedIncrement (&m_lRef);
}

ULONG EventSink::Release ()
{
  LONG lRef = InterlockedDecrement (&m_lRef);
  if (lRef == 0)
    delete this;
  return lRef;
}

HRESULT
EventSink::QueryInterface (REFIID riid, void** ppv)
{
  if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
  {
    *ppv = (IWbemObjectSink *) this;
    AddRef ();
    return WBEM_S_NO_ERROR;
  }
  else return E_NOINTERFACE;
}

HRESULT
EventSink::Indicate (long               lObjectCount,
                     IWbemClassObject** apObjArray)
{
  HRESULT hres = S_OK;

  // Run through EVERY EVENT and only take action if there is an imbalance.
  signed int start_stop = 0;

  for (int i = 0; i < lObjectCount; i++)
  {
    // EVENT: Batman Arkham Knight Started
    if (apObjArray [i]->InheritsFrom (L"__InstanceCreationEvent") != WBEM_S_FALSE) {
      start_stop++;
    }

    // EVENT: Batman Arkham Knight Stopped
    else if (apObjArray [i]->InheritsFrom (L"__InstanceDeletionEvent") != WBEM_S_FALSE) {
      start_stop--;
    }

    // SANITY CHECK: No event can inherit both!

    //BSTR XXX = bstr_t ();
    //apObjArray [i]->GetObjectText (NULL, &XXX);
    //MessageBox (NULL, XXX, L"Event occurred\n", MB_OK);
  }

  // Imbalance: We definitely need to do something (if monitoring is set to true)...
  if (start_stop != 0 && monitor_bmak) {
    // We are starting BMAK, change the desktop resolution
    if (start_stop > 0) {
      DEVMODE dmDisplay;
      ZeroMemory (&dmDisplay, sizeof DEVMODE);
      dmDisplay.dmSize = sizeof DEVMODE;

      //EnumDisplaySettings (NULL, 0, &dmDisplay);

      dmDisplay.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;// | DM_DISPLAYFREQUENCY;

                                                        // TODO: Use the Parameter Factory / Registry instead of this stupid mess.
      dmDisplay.dmPelsWidth        = _wtoi (settings.lookup_value (L"SystemSettings", L"ResX").c_str ());
      dmDisplay.dmPelsHeight       = _wtoi (settings.lookup_value (L"SystemSettings", L"ResY").c_str ());
      //dmDisplay.dmDisplayFrequency = _wtoi (FindOption (FindNode (bmak_gamesettings, L"RESOLUTION"), L"RefreshRate")->value ());

      ChangeDisplaySettings (&dmDisplay, NULL);
    }

    // We are stopping BMAK, restore the desktop
    else {
      ChangeDisplaySettings (NULL, NULL);
    }
  }

  return WBEM_S_NO_ERROR;
}

HRESULT
EventSink::SetStatus (
  /* [in] */ LONG lFlags,
  /* [in] */ HRESULT hResult,
  /* [in] */ BSTR strParam,
  /* [in] */ IWbemClassObject __RPC_FAR *pObjParam
  )
{
  if      (lFlags == WBEM_STATUS_COMPLETE) { }
  else if (lFlags == WBEM_STATUS_PROGRESS) { }

  return WBEM_S_NO_ERROR;
}