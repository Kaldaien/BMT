#include "utility.h"

#include <UserEnv.h>
#pragma comment (lib, "userenv.lib")

#include <Shlobj.h>
#pragma comment (lib, "shell32.lib")

int
BMT_MessageBox (std::wstring caption, std::wstring title, DWORD flags)
{
  extern HWND hWndApp;

  HWND parent = IsWindow (hWndApp) ? hWndApp : NULL;

  return MessageBox (parent, caption.c_str (), title.c_str (), flags | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND);
}

std::wstring
BMT_GetDocumentsDir (void)
{
  HANDLE hToken;

  if (!OpenProcessToken (GetCurrentProcess (), TOKEN_READ, &hToken))
    return NULL;

  wchar_t* str;
  SHGetKnownFolderPath (FOLDERID_Documents, 0, hToken, &str);
  std::wstring ret = str;
  CoTaskMemFree (str);
  return ret;
}

BOOL
BMT_GetUserProfileDir (wchar_t* buf, DWORD* pdwLen)
{
  HANDLE hToken;

  if (!OpenProcessToken (GetCurrentProcess (), TOKEN_READ, &hToken))
    return FALSE;

  if (!GetUserProfileDirectory (hToken, buf, pdwLen))
    return FALSE;

  CloseHandle (hToken);
  return TRUE;
}

#include <string>

bool
BMT_IsTrue (const wchar_t* string)
{
  if (std::wstring (string).length () == 1 &&
    string [0] == L'1')
    return true;

  if (std::wstring (string).length () != 4)
    return false;

  if (towlower (string [0]) != L't')
    return false;
  if (towlower (string [1]) != L'r')
    return false;
  if (towlower (string [2]) != L'u')
    return false;
  if (towlower (string [3]) != L'e')
    return false;

  return true;
}

BOOL
BMT_IsAdmin (void)
{
  BOOL   bRet = FALSE;
  HANDLE hToken = NULL;

  if (OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &hToken)) {
    TOKEN_ELEVATION Elevation;
    DWORD cbSize = sizeof (TOKEN_ELEVATION);

    if (GetTokenInformation (hToken, TokenElevation, &Elevation, sizeof (Elevation), &cbSize)) {
      bRet = Elevation.TokenIsElevated;
    }
  }

  if (hToken)
    CloseHandle (hToken);

  return bRet;
}