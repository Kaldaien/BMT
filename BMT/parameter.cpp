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

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include "parameter.h"

std::wstring
bmt::ParameterInt::get_value_str (void)
{
  wchar_t str [32];
  _itow (value, str, 10);

  return std::wstring (str);
}

int
bmt::ParameterInt::get_value (void)
{
  return value;
}

void
bmt::ParameterInt::set_value (int val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
bmt::ParameterInt::set_value_str (std::wstring str)
{
  value = _wtoi (str.c_str ());

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
bmt::ParameterInt64::get_value_str (void)
{
  wchar_t str [32];
  _i64tow (value, str, 10);

  return std::wstring (str);
}

int64_t
bmt::ParameterInt64::get_value (void)
{
  return value;
}

void
bmt::ParameterInt64::set_value (int64_t val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
bmt::ParameterInt64::set_value_str (std::wstring str)
{
  value = _wtol (str.c_str ());

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
bmt::ParameterBool::get_value_str (void)
{
  if (value == true)
    return L"true";

  return L"false";
}

bool
bmt::ParameterBool::get_value (void)
{
  return value;
}

void
bmt::ParameterBool::set_value (bool val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
bmt::ParameterBool::set_value_str (std::wstring str)
{
  if (str.length () == 1 &&
      str [0] == L'1')
    value = true;

  else if (str.length () == 4 &&
      towlower (str [0]) == L't' &&
      towlower (str [1]) == L'r' &&
      towlower (str [2]) == L'u' &&
      towlower (str [3]) == L'e')
    value = true;

  else
    value = false;

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
bmt::ParameterFloat::get_value_str (void)
{
  wchar_t val_str [16];
  swprintf (val_str, L"%f", value);

  return std::wstring (val_str);
}

float
bmt::ParameterFloat::get_value (void)
{
  return value;
}

void
bmt::ParameterFloat::set_value (float val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
bmt::ParameterFloat::set_value_str (std::wstring str)
{
  value = (float)wcstod (str.c_str (), NULL);

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}

#include <Windows.h>
#include <windowsx.h>

std::wstring
bmt::UI::EditBox::get_value_str (void)
{
  wchar_t val_str [32];
  Edit_GetText (handle, val_str, 32);
  return std::wstring (val_str);
}

void
bmt::UI::EditBox::set_value_str (std::wstring str)
{
  Edit_SetText (handle, str.c_str ());
}

std::wstring
bmt::UI::CheckBox::get_value_str (void)
{
  if (Button_GetCheck (handle)) {
    if (numeric)
      return L"1";

    return L"true";
  }
  else {
    if (numeric)
      return L"0";

    return L"false";
  }
}

void
bmt::UI::CheckBox::set_value_str (std::wstring str)
{
  if (str.length () == 1 &&
      str [0] == L'1') {
    Button_SetCheck (handle, true);
    numeric = true;
  }
  else if (str.length () == 1 &&
           str [0] == L'0') {
    Button_SetCheck (handle, false);
    numeric = true;
  }

  else if (str.length () == 4 &&
      towlower (str [0]) == L't' &&
      towlower (str [1]) == L'r' &&
      towlower (str [2]) == L'u' &&
      towlower (str [3]) == L'e') {
    Button_SetCheck (handle, true);
    numeric = false;
  }
  else {
    Button_SetCheck (handle, false);
    numeric = false;
  }
}



template <>
bmt::iParameter*
bmt::ParameterFactory::create_parameter <int> (const wchar_t* name)
{
  iParameter* param = new ParameterInt ();
  params.push_back (param);

  return param;
}

template <>
bmt::iParameter*
bmt::ParameterFactory::create_parameter <int64_t> (const wchar_t* name)
{
  iParameter* param = new ParameterInt64 ();
  params.push_back (param);

  return param;
}

template <>
bmt::iParameter*
bmt::ParameterFactory::create_parameter <bool> (const wchar_t* name)
{
  iParameter* param = new ParameterBool ();
  params.push_back (param);

  return param;
}

template <>
bmt::iParameter*
bmt::ParameterFactory::create_parameter <float> (const wchar_t* name)
{
  iParameter* param = new ParameterFloat ();
  params.push_back (param);

  return param;
}




bmt::ParameterFactory bmt::g_ParameterFactory;


bmt::ParameterInt*  refresh_rate;
bmt::ParameterInt* res_x;
bmt::ParameterInt* res_y;
bmt::ParameterInt* max_fps;

bmt::ParameterBool* use_vsync;
bmt::ParameterBool* smooth_framerate;

bmt::ParameterInt*  smoothed_min;
bmt::ParameterInt*  smoothed_max;


bmt::ParameterBool*  hardware_physx;
bmt::ParameterInt*   physx_level;
bmt::ParameterInt64* physx_heap_size;
bmt::ParameterInt64* physx_mesh_cache;

bmt::ParameterInt*  blur_samples;

bmt::ParameterInt*  anisotropy;
bmt::ParameterInt*  texture_res;

bmt::ParameterBool* enable_dx10;
bmt::ParameterBool* enable_dx11;
bmt::ParameterBool* enable_crossfire;

bmt::ParameterInt*  level_of_detail;
bmt::ParameterInt*  level_of_detail2; // Temp hack, need a way to store parameters in multiple INI keys
bmt::ParameterInt*  shadow_quality;
bmt::ParameterInt*  antialiasing;

bmt::ParameterBool*  interactive_debris;
bmt::ParameterBool*  interactive_smoke;
bmt::ParameterBool*  enhanced_rain;
bmt::ParameterBool*  enhanced_lightshafts;

bmt::ParameterFloat* mip_fadein0;
bmt::ParameterFloat* mip_fadein1;

bmt::ParameterFloat* mip_fadeout0;
bmt::ParameterFloat* mip_fadeout1;

bmt::ParameterFloat* shadow_scale;

bmt::ParameterInt*   framerate_limiting;
bmt::ParameterFloat* max_delta_time;
bmt::ParameterInt*   visibility_frames;

bmt::ParameterInt*   streaming_profile;
bmt::ParameterInt*   texgroup_profile;

bmt::ParameterBool*  decline_backup;