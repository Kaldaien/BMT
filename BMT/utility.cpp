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

#include "utility.h"

#include <UserEnv.h>
#pragma comment (lib, "userenv.lib")

#include <Shlobj.h>
#pragma comment (lib, "shell32.lib")

int
BMT_MessageBox (std::wstring caption, std::wstring title, uint32_t flags)
{
  extern bool messagebox_active;
  extern HWND hWndApp;

  HWND parent = IsWindow (hWndApp) ? hWndApp : NULL;

  messagebox_active = true;

  int ret = MessageBox (hWndApp, caption.c_str (), title.c_str (), flags | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND);

  messagebox_active = false;

  return ret;
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

bool
BMT_IsAdmin (void)
{
  bool   bRet   = false;
  HANDLE hToken = 0;

  if (OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &hToken)) {
    TOKEN_ELEVATION Elevation;
    DWORD cbSize = sizeof (TOKEN_ELEVATION);

    if (GetTokenInformation (hToken, TokenElevation, &Elevation, sizeof (Elevation), &cbSize)) {
      bRet = Elevation.TokenIsElevated != 0;
    }
  }

  if (hToken)
    CloseHandle (hToken);

  return bRet;
}

#include "xml.h"

void
BMT_DeleteAllConfigFiles (void)
{
  DeleteFile (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.ini").c_str ());
  DeleteFile (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.ini").c_str ());
  DeleteFile (std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.xml").c_str ());
}

bool
BMT_HasBackupConfigFiles (void)
{
  WIN32_FIND_DATA FindFileData;

  if (FindFirstFile (std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.bmt").c_str (), &FindFileData) != INVALID_HANDLE_VALUE)
    return true;

  return false;
}

// Copies a file preserving file times
void
BMT_FullCopy (std::wstring from, std::wstring to)
{
  DeleteFile (to.c_str ());
  CopyFile   (from.c_str (), to.c_str (), FALSE);

  WIN32_FIND_DATA FromFileData;
  HANDLE hFrom = FindFirstFile (from.c_str (), &FromFileData);

  OFSTRUCT ofTo;
  ofTo.cBytes = sizeof (OFSTRUCT);

  char     szFileTo [MAX_PATH];

  WideCharToMultiByte (CP_OEMCP, 0, to.c_str (), -1, szFileTo, MAX_PATH, NULL, NULL);
  HFILE hfTo = OpenFile (szFileTo, &ofTo, NULL);

  CloseHandle ((HANDLE)hfTo);

  // Here's where the magic happens, apply the attributes from the original file to the new one!
  SetFileTime ((HANDLE)hfTo, &FromFileData.ftCreationTime, &FromFileData.ftLastAccessTime, &FromFileData.ftLastWriteTime);

  FindClose   (hFrom);
}

void
BMT_CreateBackupConfig (void)
{
  BMT_FullCopy ( std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.xml"),
                 std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.bmt") );

  BMT_FullCopy ( std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.ini"),
                 std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.bmt") );

  BMT_FullCopy ( std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.ini"),
                 std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.bmt") );
}

void
BMT_RestoreConfigFiles (void)
{
  BMT_FullCopy ( std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.bmt"),
                 std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.xml") );

  BMT_FullCopy ( std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.bmt"),
                 std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.ini") );

  BMT_FullCopy ( std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.bmt"),
                 std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.ini") );

  DeleteFile (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.bmt").c_str ());
  DeleteFile (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.bmt").c_str ());
  DeleteFile (std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.bmt").c_str ());
}

// Gets the timestamp on the current backup file
std::wstring
BMT_GetBackupFileTime (void)
{
  WIN32_FIND_DATA FindFileData;

  HANDLE hFileBackup = FindFirstFile (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.bmt").c_str (), &FindFileData);

  FindClose (hFileBackup);

  FILETIME   ftModified;
  FileTimeToLocalFileTime (&FindFileData.ftLastWriteTime, &ftModified);

  SYSTEMTIME stModified;
  FileTimeToSystemTime (&ftModified, &stModified);

  wchar_t wszFileTime [512];

  GetDateFormat (LOCALE_CUSTOM_UI_DEFAULT, DATE_AUTOLAYOUT, &stModified, NULL, wszFileTime, 512);

  std::wstring date_time = wszFileTime;

  GetTimeFormat (LOCALE_CUSTOM_UI_DEFAULT, TIME_NOSECONDS, &stModified, NULL, wszFileTime, 512);

  date_time += L" ";
  date_time += wszFileTime;

  return date_time;
}

// Gets the timestamp on the current config file
std::wstring
BMT_GetConfigFileTime (void)
{
  //
  // XXX: It's possible that one file is newer than another, but for now let's
  //        assume SystemSettings.ini is always the newest and then see how
  //          much trouble that assumption gets us into down the line ;)
  //
  WIN32_FIND_DATA FindFileData;

  HANDLE hFileBackup = FindFirstFile (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.ini").c_str (), &FindFileData);

  FILETIME   ftModified;
  FileTimeToLocalFileTime (&FindFileData.ftLastWriteTime, &ftModified);

  SYSTEMTIME stModified;
  FileTimeToSystemTime (&ftModified, &stModified);

  FindClose (hFileBackup);

  wchar_t wszFileTime [512];

  GetDateFormat (LOCALE_CUSTOM_UI_DEFAULT, DATE_AUTOLAYOUT, &stModified, NULL, wszFileTime, 512);

  std::wstring date_time = wszFileTime;

  GetTimeFormat (LOCALE_CUSTOM_UI_DEFAULT, TIME_NOSECONDS, &stModified, NULL, wszFileTime, 512);

  date_time += L" ";
  date_time += wszFileTime;

  return date_time;
}