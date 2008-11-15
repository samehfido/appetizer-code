/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "../stdafx.h"

#ifndef __FileOrFolderDialog_H
#define __FileOrFolderDialog_H


#include "FileExplorerControl.h"


enum {
  ID_FILEEXPLODLG_ExplorerControl
};


class FileOrFolderDialog: public wxDialog {

public:

  FileOrFolderDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Select a file or a folder"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(350, 400), long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = _T("dialogBox"));
  void Localize();
  wxString GetSelectedPath();

  void OnTreeSelectionChanged(wxTreeEvent& evt);

protected:

  wxBoxSizer* topSizer;
  wxFlexGridSizer* verticalSizer;
  wxFlexGridSizer* buttonSizer;
  FileExplorerControl* explorerControl;
  wxButton* okButton;
  wxButton* cancelButton;  

  DECLARE_EVENT_TABLE()

};


#endif // __AboutDialog_H