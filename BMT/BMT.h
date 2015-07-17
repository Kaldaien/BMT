#pragma once

#include "resource.h"

#include <string>
#include "ini.h"

class BM_Engine {
public:
  BM_Engine (void) { ini = nullptr; };
  virtual ~BM_Engine (void)
  {
    if (ini != nullptr) {
      delete ini;
      ini = nullptr;
    }
  }

  void load (std::wstring path) {
    ini = new bmt::INI::File ((wchar_t *)std::wstring (path + std::wstring (L"..\\..\\BMGame\\Config\\BmEngine.ini")).c_str ());
  }

  void save (std::wstring path) {
    ini->write (std::wstring (path + std::wstring (L"..\\..\\BMGame\\Config\\BmEngine.ini")).c_str ());
  }

  std::wstring
    lookup_value (std::wstring section_name, std::wstring key_name) {
    bmt::INI::File::Section& section = ini->get_section (section_name);
    return section.get_value (key_name);
  }

  void
    set_value (std::wstring section_name, std::wstring key_name, std::wstring value) {
    bmt::INI::File::Section& section = ini->get_section (section_name);
    section.get_value (key_name) = value;
  }

  void import (std::wstring imp_data) {
    ini->import (imp_data);
  }

  bmt::INI::File* get_file (void) { return ini; }

private:
  bmt::INI::File* ini;
} static engine;

class BM_SystemSettings {
public:
  BM_SystemSettings (void) { ini = nullptr; };
  virtual ~BM_SystemSettings (void)
  {
    if (ini != nullptr) {
      delete ini;
      ini = nullptr;
    }
  }

  void load (std::wstring path) {
    ini = new bmt::INI::File ((wchar_t *)std::wstring (path + std::wstring (L"..\\..\\BMGame\\Config\\BmSystemSettings.ini")).c_str ());
  }

  void save (std::wstring path) {
    ini->write (std::wstring (path + std::wstring (L"..\\..\\BMGame\\Config\\BmSystemSettings.ini")).c_str ());
  }

  std::wstring
    lookup_value (std::wstring section_name, std::wstring key_name) {
    bmt::INI::File::Section& section = ini->get_section (section_name);
    return section.get_value (key_name);
  }

  void
    set_value (std::wstring section_name, std::wstring key_name, std::wstring value) {
    bmt::INI::File::Section& section = ini->get_section (section_name);
    section.get_value (key_name) = value;
  }

  void import (std::wstring imp_data) {
    ini->import (imp_data);
  }

  bmt::INI::File* get_file (void) { return ini; }

private:
  bmt::INI::File* ini;
} static settings;