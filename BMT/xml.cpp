#include "xml.h"
#include "utility.h"

#include <Windows.h>
#include <cstring>

xml_node<wchar_t>*
BMT_XML_FindNode (xml_node<wchar_t>* parent_node, std::wstring name)
{
  if (parent_node == NULL)
    return NULL;

  xml_node<wchar_t>* node = parent_node->first_node ();
  while (node != NULL) {
    if (std::wstring (node->name ()) == name)
      return node;
    node = node->next_sibling ();
  }

  return NULL;
}

xml_attribute<wchar_t>*
BMT_XML_FindAttrib (xml_node<wchar_t>* parent_node, std::wstring name)
{
  if (parent_node == NULL)
    return NULL;

  xml_attribute<wchar_t>* attrib = parent_node->first_attribute ();
  while (attrib != NULL) {
    if (std::wstring (attrib->name ()) == name)
      return attrib;
    attrib = attrib->next_attribute ();
  }

  return NULL;
}

xml_node<wchar_t>*
BMT_XML_FindOption (xml_node<wchar_t>* parent_node, std::wstring name)
{
  if (parent_node == NULL)
    return NULL;

  xml_node<wchar_t>* node = parent_node->first_node ();
  while (node != NULL) {
    if (std::wstring (node->name ()) == L"OPTION") {
      xml_attribute<wchar_t>* attrib = node->first_attribute ();
      while (attrib != NULL) {
        if (std::wstring (attrib->name ()) == L"Name") {
          if (std::wstring (attrib->value ()) == name) {
            return attrib->parent ();
          }
        }
        attrib = attrib->next_attribute ();
      }
    }
    node = node->next_sibling ();
  }

  return NULL;
}

std::wstring executable;
std::wstring install_path;
xml_document<wchar_t> bmak_xml;

xml_node<wchar_t>* bmak_root;
xml_node<wchar_t>* bmak_application;
xml_node<wchar_t>* bmak_gamesettings;

void
BMT_SaveXML (void)
{
  std::wstring documents_dir = BMT_GetDocumentsDir ();

  wchar_t wszXML [1024];

  wsprintf (wszXML, L"%s\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.xml", documents_dir.c_str ());

  wchar_t* wszOut = new wchar_t [8192];
  wchar_t* wszEnd = print (wszOut, bmak_xml, 0);

  int last_brace = 0;

  // XML parser doesn't like the TM symbol, so get it the hell out of there!
  for (int i = 0; i < 8192; i++) {
    if (wszOut [i] == L'™')
      wszOut [i] = L' ';
    if (wszOut [i] == L'>')
      last_brace = i;
  }

  wszOut [last_brace + 1] = L'\0';

  FILE* fXML;
  errno_t ret = _wfopen_s (&fXML, wszXML, L"w,ccs=UTF-16LE");

  if (ret != 0 || fXML == 0) {
    delete [] wszOut;
    BMT_MessageBox (L"Could not open GFXSettings.BatmanArkhamKnight.xml for writing!\n", L"Unable to save XML settings", MB_OK | MB_ICONSTOP);
    return;
  }

  fputws (L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"no\"?>\n", fXML);
  fputws (wszOut, fXML);

  delete [] wszOut;

  fflush (fXML);
  fclose (fXML);
}

bool
BMT_LoadXML (void)
{
  wchar_t wszXML [1024];

  std::wstring documents_dir = BMT_GetDocumentsDir ();

  wsprintf (wszXML, L"%s\\WB Games\\Batman Arkham Knight\\GFXSettings.BatmanArkhamKnight.xml", documents_dir.c_str ());

  FILE* fXML;
  errno_t ret = _wfopen_s (&fXML, wszXML, L"r,ccs=UTF-16LE");

  if (ret != 0) {
    BMT_MessageBox (L"Unable to locate GFXSettings.BatmanArkhamKnight.XML.\n\n  "
      L"This file is created when the game starts, try running the "
      L"game once.\n", L"Missing XML File", MB_OK | MB_ICONERROR);

    return false;
  }

  fseek (fXML, 0, SEEK_END);
  DWORD dwLen = ftell (fXML);
  rewind (fXML);

  wchar_t* xml = new wchar_t [dwLen + 1];
  fread (xml, 1, dwLen, fXML);

  int last_brace = 0;

  // XML parser doesn't like the TM symbol, so get it the hell out of there!
  for (unsigned int i = 0; i < dwLen; i++) {
    if (xml [i] == L'™')
      xml [i] = L' ';
    if (xml [i] == L'>')
      last_brace = i;
  }

  xml [last_brace + 1] = 0;

  bmak_xml.parse <0> (xml);

  fclose (fXML);

  bmak_root         = bmak_xml.first_node ();
  if (bmak_root == NULL) {
    BMT_MessageBox (L"GFXSettings.BatmanArkhamKnight.xml appears to be corrupt, please delete it and re-load the game\n", L"Corrupt XML File", MB_OK | MB_ICONHAND);
    return false;
  }

  bmak_application  = bmak_root->first_node ();
  bmak_gamesettings = BMT_XML_FindNode (bmak_root, L"GAMESETTINGS");

  xml_attribute <wchar_t>* install_path_attrib =
    BMT_XML_FindAttrib (BMT_XML_FindNode (bmak_application, L"INSTALLPATH"), L"Value");

  if (install_path_attrib != NULL) {
    install_path = install_path_attrib->value ();
  }

  xml_attribute <wchar_t>* exec_cmd_attrib =
    BMT_XML_FindAttrib (BMT_XML_FindNode (bmak_application, L"EXECCMD"), L"Value");

  if (exec_cmd_attrib != NULL && install_path_attrib != NULL) {
    executable = install_path_attrib->value () + std::wstring (exec_cmd_attrib->value ()) + std::wstring (L".exe");
  }

  return true;
}