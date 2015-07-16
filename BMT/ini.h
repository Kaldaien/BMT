#ifndef __BMT__INI_H__
#define __BMT__INI_H__

#include <string>
#include <map>

class BMT_INI_File
{
public:
           BMT_INI_File (wchar_t* filename);
  virtual ~BMT_INI_File (void);

  void parse  (void);
  void import (std::wstring import_data);
  void write  (std::wstring fname);

  class Section
  {
  public:
    Section (void) {
    }

    Section (std::wstring section_name) {
      name = section_name;
    }

    // Technically, this isn't 1:1 ... but as far as WE'RE concerned, all the
    //   keys we're interested in _are_.
    std::wstring& get_value     (std::wstring key);
    bool          contains_key  (std::wstring key);
    void          add_key_value (std::wstring key, std::wstring value);

  //protected:
  //private:
    std::wstring name;
    std::multimap <std::wstring, std::wstring> pairs;
  };

  const std::map <std::wstring, Section>& get_sections (void);

  Section& get_section      (std::wstring section);
  bool     contains_section (std::wstring section);

protected:
private:
  FILE*    fINI;
  wchar_t* wszName;
  wchar_t* wszData;
  std::map <std::wstring, Section> sections;
};

#endif
