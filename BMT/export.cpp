#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "resource.h"

#include "bmt.h"

INT_PTR
CALLBACK
ImportExport (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
  {
    HWND hWndDataSelect = GetDlgItem (hDlg, IDC_DATA);

    LONG style = GetWindowLong (hWndDataSelect, GWL_STYLE);
    SetWindowLong (hWndDataSelect, GWL_STYLE, style | TVS_CHECKBOXES | TVS_HASBUTTONS);

    wchar_t* default_sections [] { L"Engine.Engine",
      L"Engine.Client",
      L"Engine.GameEngine",
      L"TextureStreaming" };
    int num_def_secs = sizeof (default_sections) / sizeof (wchar_t *);

    BMT_INI_File* bm_engine_ini = engine.get_file ();

    int idx = 0;

    TVINSERTSTRUCT ins;
    ins.hParent = NULL;
    ins.hInsertAfter = TVI_ROOT;
    ins.item.pszText = L"Configuration Files";
    ins.item.mask = TVIF_TEXT;

    HTREEITEM hRoot = TreeView_InsertItem (hWndDataSelect, &ins);

    ins.hParent = hRoot;
    ins.hInsertAfter = TVI_ROOT;
    ins.item.pszText = L"BmEngine.ini";
    ins.item.mask = TVIF_TEXT;

    HTREEITEM hEngine = TreeView_InsertItem (hWndDataSelect, &ins);

    for (std::map <std::wstring, BMT_INI_File::Section>::const_iterator it = bm_engine_ini->get_sections ().begin ();
    it != bm_engine_ini->get_sections ().end ();
      ++it) {

#if 1
      ins.hParent = hEngine;
      ins.hInsertAfter = TVI_LAST;
      ins.item.pszText = (wchar_t *)(it->first.c_str ());
      ins.item.mask = TVIF_TEXT;

      HTREEITEM hItem = TreeView_InsertItem (hWndDataSelect, &ins);

      for (int i = 0; i < num_def_secs; i++) {
        if (lstrcmpW (ins.item.pszText, default_sections [i]) == 0) {
          TreeView_SetCheckState (hWndDataSelect, hItem, true);
          break;
        }
      }
#endif
    }

    TreeView_Expand (hWndDataSelect, hRoot, TVE_EXPAND);

    ins.hParent = hRoot;
    ins.hInsertAfter = TVI_ROOT;
    ins.item.pszText = L"BmSystemSettings.ini";
    ins.item.mask = TVIF_TEXT;

    HTREEITEM hSystem = TreeView_InsertItem (hWndDataSelect, &ins);

    BMT_INI_File* bm_system_ini = settings.get_file ();

    for (std::map <std::wstring, BMT_INI_File::Section>::const_iterator it = bm_system_ini->get_sections ().begin ();
    it != bm_system_ini->get_sections ().end ();
      ++it) {

#if 1
      ins.hParent = hSystem;
      ins.hInsertAfter = TVI_LAST;
      ins.item.pszText = (wchar_t *)(it->first.c_str ());
      ins.item.mask = TVIF_TEXT;

      HTREEITEM hItem = TreeView_InsertItem (hWndDataSelect, &ins);

      for (int i = 0; i < num_def_secs; i++) {
        if (lstrcmpW (ins.item.pszText, default_sections [i]) == 0) {
          TreeView_SetCheckState (hWndDataSelect, hItem, true);
          break;
        }
      }
#endif
    }

  } break;

  case WM_COMMAND:
  {
    switch (LOWORD (wParam))
    {
    case IDOK:
      EndDialog (hDlg, TRUE);
      break;
    }
  }
  }

  return (INT_PTR)FALSE;
}