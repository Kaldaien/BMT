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

#ifndef __BMT__UTILITY_H__
#define __BMT__UTILITY_H__

#include <string>

std::wstring  BMT_GetDocumentsDir      (void);
bool          BMT_GetUserProfileDir    (wchar_t* buf, uint32_t* pdwLen);
bool          BMT_IsTrue               (const wchar_t* string);
bool          BMT_IsAdmin              (void);
int           BMT_MessageBox           (std::wstring caption, std::wstring title, uint32_t flags);
void          BMT_DeleteAllConfigFiles (void);

bool          BMT_HasBackupConfigFiles (void);
std::wstring  BMT_GetConfigFileTime    (void);
void          BMT_CreateBackupConfig   (void);
void          BMT_RestoreConfigFiles   (void);

std::wstring  BMT_GetBackupFileTime    (void);
std::wstring  BMT_GetConfigFileTime    (void);

inline size_t
BMT_NextPowerOfTwo (size_t x)
{
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

#endif /* __BMT__UTILITY_H__ */