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