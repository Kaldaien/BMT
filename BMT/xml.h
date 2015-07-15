#ifndef __BMT__XML_H__
#define __BMT__XML_H__

#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"

using namespace rapidxml;

xml_node<wchar_t>*
BMT_XML_FindNode (xml_node<wchar_t>* parent_node, std::wstring name);

xml_attribute<wchar_t>*
BMT_XML_FindAttrib (xml_node<wchar_t>* parent_node, std::wstring name);

xml_node<wchar_t>*
BMT_XML_FindOption (xml_node<wchar_t>* parent_node, std::wstring name);

extern std::wstring install_path;
extern xml_document<wchar_t> bmak_xml;

extern xml_node<wchar_t>* bmak_root;
extern xml_node<wchar_t>* bmak_application;
extern xml_node<wchar_t>* bmak_gamesettings;

void BMT_SaveXML (void);
bool BMT_LoadXML (void);

#endif /* __BMT__XML_H__ */
