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
  // Strip Read-Only
  BMT_SetNormalFileAttribs (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.ini"));
  BMT_SetNormalFileAttribs (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.ini"));
  BMT_SetNormalFileAttribs (std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.xml"));

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
  // Strip Read-Only
  BMT_SetNormalFileAttribs (to);
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

  // Strip Read-Only
  BMT_SetNormalFileAttribs (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmSystemSettings.bmt"));
  BMT_SetNormalFileAttribs (std::wstring (bmt::XML::install_path + L"..\\..\\BMGame\\Config\\BmEngine.bmt"));
  BMT_SetNormalFileAttribs (std::wstring (BMT_GetDocumentsDir () + L"\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.bmt"));

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

#pragma comment (lib, "advapi32.lib")

#include <windows.h>
#include <stdio.h>
#include <accctrl.h>
#include <aclapi.h>

BOOL TakeOwnership (LPTSTR lpszOwnFile);

void
BMT_SetNormalFileAttribs (std::wstring file)
{
  SetFileAttributes (file.c_str (), FILE_ATTRIBUTE_NORMAL);
  TakeOwnership     ((LPWSTR)file.c_str ());
}




//Forward declaration of SetPrivilege
BOOL SetPrivilege (
  HANDLE hToken,          // access token handle
  LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
  BOOL bEnablePrivilege   // to enable or disable privilege
  );


BOOL TakeOwnership (LPTSTR lpszOwnFile)
{

  BOOL bRetval = FALSE;

  HANDLE hToken = NULL;
  PSID pSIDAdmin = NULL;
  PSID pSIDEveryone = NULL;
  PACL pACL = NULL;
  SID_IDENTIFIER_AUTHORITY SIDAuthWorld =
    SECURITY_WORLD_SID_AUTHORITY;
  SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
  const int NUM_ACES = 2;
  EXPLICIT_ACCESS ea [NUM_ACES];
  DWORD dwRes;

  // Specify the DACL to use.
  // Create a SID for the Everyone group.
  if (!AllocateAndInitializeSid (&SIDAuthWorld, 1,
    SECURITY_WORLD_RID,
    0,
    0, 0, 0, 0, 0, 0,
    &pSIDEveryone))
  {
    //printf ("AllocateAndInitializeSid (Everyone) error %u\n",
      //GetLastError ());
    goto Cleanup;
  }

  // Create a SID for the BUILTIN\Administrators group.
  if (!AllocateAndInitializeSid (&SIDAuthNT, 2,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_ADMINS,
    0, 0, 0, 0, 0, 0,
    &pSIDAdmin))
  {
    //printf ("AllocateAndInitializeSid (Admin) error %u\n",
      //GetLastError ());
    goto Cleanup;
  }

  ZeroMemory (&ea, NUM_ACES * sizeof (EXPLICIT_ACCESS));

  // Set full control for Everyone.
  ea [0].grfAccessPermissions = GENERIC_ALL;// GENERIC_READ;
  ea [0].grfAccessMode = SET_ACCESS;
  ea [0].grfInheritance = NO_INHERITANCE;
  ea [0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea [0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
  ea [0].Trustee.ptstrName = (LPTSTR)pSIDEveryone;

  // Set full control for Administrators.
  ea [1].grfAccessPermissions = GENERIC_ALL;
  ea [1].grfAccessMode = SET_ACCESS;
  ea [1].grfInheritance = NO_INHERITANCE;
  ea [1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea [1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea [1].Trustee.ptstrName = (LPTSTR)pSIDAdmin;

  if (ERROR_SUCCESS != SetEntriesInAcl (NUM_ACES,
    ea,
    NULL,
    &pACL))
  {
    //printf ("Failed SetEntriesInAcl\n");
    goto Cleanup;
  }

  // Try to modify the object's DACL.
  dwRes = SetNamedSecurityInfo (
    lpszOwnFile,                 // name of the object
    SE_FILE_OBJECT,              // type of object
    DACL_SECURITY_INFORMATION,   // change only the object's DACL
    NULL, NULL,                  // do not change owner or group
    pACL,                        // DACL specified
    NULL);                       // do not change SACL

  if (ERROR_SUCCESS == dwRes)
  {
    //printf ("Successfully changed DACL\n");
    bRetval = TRUE;
    // No more processing needed.
    goto Cleanup;
  }
  if (dwRes != ERROR_ACCESS_DENIED)
  {
    //printf ("First SetNamedSecurityInfo call failed: %u\n",
      //dwRes);
    goto Cleanup;
  }

  // If the preceding call failed because access was denied, 
  // enable the SE_TAKE_OWNERSHIP_NAME privilege, create a SID for 
  // the Administrators group, take ownership of the object, and 
  // disable the privilege. Then try again to set the object's DACL.

  // Open a handle to the access token for the calling process.
  if (!OpenProcessToken (GetCurrentProcess (),
    TOKEN_ADJUST_PRIVILEGES,
    &hToken))
  {
    //printf ("OpenProcessToken failed: %u\n", GetLastError ());
    goto Cleanup;
  }

  // Enable the SE_TAKE_OWNERSHIP_NAME privilege.
  if (!SetPrivilege (hToken, SE_TAKE_OWNERSHIP_NAME, TRUE))
  {
    //printf ("You must be logged on as Administrator.\n");
    goto Cleanup;
  }

  // Set the owner in the object's security descriptor.
  dwRes = SetNamedSecurityInfo (
    lpszOwnFile,                 // name of the object
    SE_FILE_OBJECT,              // type of object
    OWNER_SECURITY_INFORMATION,  // change only the object's owner
    pSIDAdmin,                   // SID of Administrator group
    NULL,
    NULL,
    NULL);

  if (dwRes != ERROR_SUCCESS)
  {
    //printf ("Could not set owner. Error: %u\n", dwRes);
    goto Cleanup;
  }

  // Disable the SE_TAKE_OWNERSHIP_NAME privilege.
  if (!SetPrivilege (hToken, SE_TAKE_OWNERSHIP_NAME, FALSE))
  {
    //printf ("Failed SetPrivilege call unexpectedly.\n");
    goto Cleanup;
  }

  // Try again to modify the object's DACL,
  // now that we are the owner.
  dwRes = SetNamedSecurityInfo (
    lpszOwnFile,                 // name of the object
    SE_FILE_OBJECT,              // type of object
    DACL_SECURITY_INFORMATION,   // change only the object's DACL
    NULL, NULL,                  // do not change owner or group
    pACL,                        // DACL specified
    NULL);                       // do not change SACL

  if (dwRes == ERROR_SUCCESS)
  {
    //printf ("Successfully changed DACL\n");
    bRetval = TRUE;
  }
  else
  {
    //printf ("Second SetNamedSecurityInfo call failed: %u\n",
      //dwRes);
  }

Cleanup:

  if (pSIDAdmin)
    FreeSid (pSIDAdmin);

  if (pSIDEveryone)
    FreeSid (pSIDEveryone);

  if (pACL)
    LocalFree (pACL);

  if (hToken)
    CloseHandle (hToken);

  return bRetval;
}

BOOL SetPrivilege (
  HANDLE hToken,          // access token handle
  LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
  BOOL bEnablePrivilege   // to enable or disable privilege
  )
{
  TOKEN_PRIVILEGES tp;
  LUID luid;

  if (!LookupPrivilegeValue (
    NULL,            // lookup privilege on local system
    lpszPrivilege,   // privilege to lookup 
    &luid))        // receives LUID of privilege
  {
    //printf ("LookupPrivilegeValue error: %u\n", GetLastError ());
    return FALSE;
  }

  tp.PrivilegeCount = 1;
  tp.Privileges [0].Luid = luid;
  if (bEnablePrivilege)
    tp.Privileges [0].Attributes = SE_PRIVILEGE_ENABLED;
  else
    tp.Privileges [0].Attributes = 0;

  // Enable the privilege or disable all privileges.

  if (!AdjustTokenPrivileges (
    hToken,
    FALSE,
    &tp,
    sizeof (TOKEN_PRIVILEGES),
    (PTOKEN_PRIVILEGES)NULL,
    (PDWORD)NULL))
  {
    //printf ("AdjustTokenPrivileges error: %u\n", GetLastError ());
    return FALSE;
  }

  if (GetLastError () == ERROR_NOT_ALL_ASSIGNED)

  {
    //printf ("The token does not have the specified privilege. \n");
    return FALSE;
  }

  return TRUE;
}