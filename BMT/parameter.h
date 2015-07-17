#ifndef __BMT__PARAMETER_H__
#define __BMT__PARAMETER_H__

#include "ini.h"
#include "xml.h"

#include <Windows.h>
#include <vector>

namespace bmt {
  namespace UI {

class Control {
  public:
    Control (void) {
      handle = NULL;
    }

    virtual std::wstring get_value_str (void) = 0;
    virtual void         set_value_str (std::wstring val_str) = 0;

    BOOL         exists (void) { return IsWindow (handle); }

  protected:
    HWND    handle;

  private:
};

class EditBox : public Control {
public:
  EditBox (HWND hWnd) {
    handle = hWnd;
  }

  virtual std::wstring get_value_str (void);
  virtual void         set_value_str (std::wstring val_str);
};

class CheckBox : public Control {
public:
  CheckBox (HWND hWnd) {
    handle  = hWnd;
    numeric = false; // When true, input will be 1/0 and output will be 1/0
  }

  virtual std::wstring get_value_str (void);
  virtual void         set_value_str (std::wstring val_str);

private:
  bool numeric;
};

};

class iParameter {
public:
  iParameter (void) {
    xml_attrib = nullptr;
    ini        = nullptr;
    ui_control = nullptr;
  }

  virtual std::wstring get_value_str (void) = 0;
  virtual void         set_value_str (std::wstring str) = 0;

  // Read value from INI or XML
  bool load (void)
  {
    if (xml_attrib != nullptr) {
      set_value_str (xml_attrib->value ());
      return true;
    }
    else if (ini != nullptr) {
      INI::File::Section& section = ini->get_section (ini_section);

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
      INI::File::Section& section = ini->get_section (ini_section);

      if (section.contains_key (ini_key)) {
        section.get_value (ini_key) = backing_string;
        ret = true;
      }
    }

    return ret;
  }

  void register_to_ini (INI::File* file, std::wstring section, std::wstring key)
  {
    ini         = file;
    ini_section = section;
    ini_key     = key;
  }

  void register_to_xml (xml_node <wchar_t>* node, std::wstring attrib_name)
  {
    if (node != nullptr) {
      xml_attrib =
        XML::FindAttrib (node, attrib_name.c_str ());
    }
  }

  void bind_to_control (UI::Control* ui_ctl)
  {
    ui_control = ui_ctl;
  }

protected:
  UI::Control*             ui_control;
  wchar_t                  backing_string [64]; // Required by XML

private:
  xml_attribute <wchar_t>* xml_attrib;
  INI::File*               ini;
  std::wstring             ini_section;
  std::wstring             ini_key;
};

template <typename _T>
class Parameter : public iParameter {
public:
  virtual std::wstring get_value_str (void) = 0;
  virtual _T           get_value     (void) = 0;

  virtual void         set_value     (_T val)           = 0;
  virtual void         set_value_str (std::wstring str) = 0;

protected:
  _T                       value;
};

class ParameterInt : public Parameter <int>
{
public:
  std::wstring get_value_str (void);
  int          get_value     (void);

  void         set_value     (int val);
  void         set_value_str (std::wstring str);

protected:
  int value;
};

class ParameterInt64 : public Parameter <int64_t>
{
public:
  std::wstring get_value_str (void);
  int64_t      get_value (void);

  void         set_value (int64_t val);
  void         set_value_str (std::wstring str);

protected:
  int64_t value;
};

class ParameterBool : public Parameter <bool>
{
public:
  std::wstring get_value_str (void);
  bool         get_value     (void);

  void         set_value     (bool val);
  void         set_value_str (std::wstring str);

protected:
  bool value;
};

class ParameterFloat : public Parameter <float>
{
public:
  std::wstring get_value_str (void);
  float        get_value (void);

  void         set_value (float val);
  void         set_value_str (std::wstring str);

protected:
  float value;
};

class ParameterFactory {
public:
  template <typename _T> iParameter* create_parameter  (const wchar_t* name);
protected:
private:
  std::vector <iParameter *> params;
} static g_ParameterFactory;

}


static bmt::ParameterInt*  refresh_rate;
static bmt::ParameterInt* res_x;
static bmt::ParameterInt* res_y;
static bmt::ParameterInt* max_fps;

static bmt::ParameterBool* use_vsync;
static bmt::ParameterBool* smooth_framerate;

static bmt::ParameterInt*  smoothed_min;
static bmt::ParameterInt*  smoothed_max;


static bmt::ParameterBool*  hardware_physx;
static bmt::ParameterInt*   physx_level;
static bmt::ParameterInt64* physx_heap_size;
static bmt::ParameterInt64* physx_mesh_cache;

static bmt::ParameterInt*  blur_samples;

static bmt::ParameterInt*  anisotropy;
static bmt::ParameterInt*  texture_res;

static bmt::ParameterBool* enable_dx10;
static bmt::ParameterBool* enable_dx11;
static bmt::ParameterBool* enable_crossfire;

static bmt::ParameterInt*  level_of_detail;
static bmt::ParameterInt*  level_of_detail2; // Temp hack, need a way to store parameters in multiple INI keys
static bmt::ParameterInt*  shadow_quality;
static bmt::ParameterInt*  antialiasing;

static bmt::ParameterBool*  interactive_debris;
static bmt::ParameterBool*  interactive_smoke;
static bmt::ParameterBool*  enhanced_rain;
static bmt::ParameterBool*  enhanced_lightshafts;

static bmt::ParameterFloat* mip_fadein0;
static bmt::ParameterFloat* mip_fadein1;

static bmt::ParameterFloat* mip_fadeout0;
static bmt::ParameterFloat* mip_fadeout1;

static bmt::ParameterFloat* shadow_scale;

static bmt::ParameterInt*   framerate_limiting;
static bmt::ParameterFloat* max_delta_time;
static bmt::ParameterInt*   visibility_frames;

#endif /* __BMT__PARAMETER_H__ */