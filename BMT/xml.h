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

#ifndef __BMT__XML_H__
#define __BMT__XML_H__

#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"

using namespace rapidxml;

namespace bmt {
namespace XML {

  xml_node<wchar_t>*
    FindNode (xml_node<wchar_t>* parent_node, std::wstring name);

  xml_attribute<wchar_t>*
    FindAttrib (xml_node<wchar_t>* parent_node, std::wstring name);

  xml_node<wchar_t>*
    FindOption (xml_node<wchar_t>* parent_node, std::wstring name);

  extern std::wstring install_path;
  extern std::wstring executable;
  extern xml_document<wchar_t> bmak_xml;

  extern xml_node<wchar_t>* bmak_root;
  extern xml_node<wchar_t>* bmak_application;
  extern xml_node<wchar_t>* bmak_gamesettings;

  void SaveXML (void);
  bool LoadXML (void);

}
}

#endif /* __BMT__XML_H__ */
