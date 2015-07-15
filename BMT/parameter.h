#ifndef __BMT__PARAMETER_H__
#define __BMT__PARAMETER_H__

#include "ini.h"
#include "xml.h"

#include <Windows.h>

class BMT_UI_Control {
public:
  BMT_UI_Control (void) {
    handle = NULL;
  }

  virtual std::wstring get_value_str (void) = 0;
  virtual void         set_value_str (std::wstring val_str) = 0;

  BOOL         exists        (void)                 { return IsWindow (handle); }

protected:
  HWND    handle;

private:
};

class BMT_EditBox : public BMT_UI_Control {
public:
  BMT_EditBox (HWND hWnd) {
    handle = hWnd;
  }

  virtual std::wstring get_value_str (void);
  virtual void         set_value_str (std::wstring val_str);
};

class BMT_CheckBox : public BMT_UI_Control {
public:
  BMT_CheckBox (HWND hWnd) {
    handle  = hWnd;
    numeric = false; // When true, input will be 1/0 and output will be 1/0
  }

  virtual std::wstring get_value_str (void);
  virtual void         set_value_str (std::wstring val_str);

private:
  bool numeric;
};

template <typename _T>
class BMT_Parameter {
public:
  BMT_Parameter (void) {
    xml_attrib = nullptr;
    ini        = nullptr;
    ui_control = nullptr;
  }

  virtual std::wstring get_value_str (void) = 0;
  virtual _T           get_value     (void) = 0;

  virtual void         set_value     (_T val)           = 0;
  virtual void         set_value_str (std::wstring str) = 0;

  // Read value from INI or XML
  bool load  (void)
  {
    if (xml_attrib != nullptr) {
      set_value_str (xml_attrib->value ());
      return true;
    }
    else if (ini != nullptr) {
      BMT_INI_File::Section& section = ini->get_section (ini_section);

      if (section.contains_key (ini_key)) {
        set_value_str (section.get_value (ini_key));
        return true;
      }
    }

    return false;
  }

  // Store value in INI and/or XML
  bool store (void)
  {
    bool ret = false;

    if (ui_control != nullptr)
      set_value_str (ui_control->get_value_str ());

    wcsncpy_s (backing_string, get_value_str ().c_str (), 64);

    if (xml_attrib != nullptr) {
      xml_attrib->value (backing_string);
      ret = true;
    }
    if (ini != nullptr) {
      BMT_INI_File::Section& section = ini->get_section (ini_section);

      if (section.contains_key (ini_key)) {
        section.get_value (ini_key) = backing_string;
        ret = true;
      }
    }

    return ret;
  }

  void register_to_ini (BMT_INI_File* file, std::wstring section, std::wstring key)
  {
    ini = file;
    ini_section = section;
    ini_key = key;
  }

  void register_to_xml (xml_node <wchar_t>* node, std::wstring attrib_name)
  {
    if (node != nullptr) {
      xml_attrib =
        BMT_XML_FindAttrib (node, attrib_name.c_str ());
    }
  }

  void bind_to_control (BMT_UI_Control* ui_ctl)
  {
    ui_control = ui_ctl;
  }

protected:
  _T                       value;
  BMT_UI_Control*          ui_control;
  wchar_t                  backing_string [64]; // Required by XML

private:
  xml_attribute <wchar_t>* xml_attrib;
  BMT_INI_File*            ini;
  std::wstring             ini_section;
  std::wstring             ini_key;
};

class BMT_Parameter_Int : public BMT_Parameter <int>
{
public:
  std::wstring get_value_str (void);
  int          get_value     (void);

  void         set_value     (int val);
  void         set_value_str (std::wstring str);

protected:
  int value;
};

class BMT_Parameter_Int64 : public BMT_Parameter <int64_t>
{
public:
  std::wstring get_value_str (void);
  int64_t      get_value (void);

  void         set_value (int64_t val);
  void         set_value_str (std::wstring str);

protected:
  int64_t value;
};

class BMT_Parameter_Bool : public BMT_Parameter <bool>
{
public:
  std::wstring get_value_str (void);
  bool         get_value     (void);

  void         set_value     (bool val);
  void         set_value_str (std::wstring str);

protected:
  bool value;
};

class BMT_Parameter_Float : public BMT_Parameter <float>
{
public:
  std::wstring get_value_str (void);
  float        get_value (void);

  void         set_value (float val);
  void         set_value_str (std::wstring str);

protected:
  float value;
};
#endif /* __BMT__PARAMETER_H__ */
