#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include "parameter.h"

std::wstring
BMT_Parameter_Int::get_value_str (void)
{
  wchar_t str [32];
  _itow (value, str, 10);

  return std::wstring (str);
}

int
BMT_Parameter_Int::get_value (void)
{
  return value;
}

void
BMT_Parameter_Int::set_value (int val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
BMT_Parameter_Int::set_value_str (std::wstring str)
{
  value = _wtoi (str.c_str ());

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
BMT_Parameter_Int64::get_value_str (void)
{
  wchar_t str [32];
  _i64tow (value, str, 10);

  return std::wstring (str);
}

int64_t
BMT_Parameter_Int64::get_value (void)
{
  return value;
}

void
BMT_Parameter_Int64::set_value (int64_t val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
BMT_Parameter_Int64::set_value_str (std::wstring str)
{
  value = _wtol (str.c_str ());

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
BMT_Parameter_Bool::get_value_str (void)
{
  if (value == true)
    return L"true";

  return L"false";
}

bool
BMT_Parameter_Bool::get_value (void)
{
  return value;
}

void
BMT_Parameter_Bool::set_value (bool val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
BMT_Parameter_Bool::set_value_str (std::wstring str)
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
BMT_Parameter_Float::get_value_str (void)
{
  wchar_t val_str [16];
  swprintf (val_str, L"%f", value);

  return std::wstring (val_str);
}

float
BMT_Parameter_Float::get_value (void)
{
  return value;
}

void
BMT_Parameter_Float::set_value (float val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
BMT_Parameter_Float::set_value_str (std::wstring str)
{
  value = (float)wcstod (str.c_str (), NULL);

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}

#include <Windows.h>
#include <windowsx.h>

std::wstring
BMT_EditBox::get_value_str (void)
{
  wchar_t val_str [32];
  Edit_GetText (handle, val_str, 32);
  return std::wstring (val_str);
}

void
BMT_EditBox::set_value_str (std::wstring str)
{
  Edit_SetText (handle, str.c_str ());
}

std::wstring
BMT_CheckBox::get_value_str (void)
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
BMT_CheckBox::set_value_str (std::wstring str)
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