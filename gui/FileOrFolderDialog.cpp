/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "../stdafx.h"

#include "FileOrFolderDialog.h"
#include "../utilities/IconGetter.h"


BEGIN_EVENT_TABLE(FileOrFolderDialog, wxDialog)
  EVT_TREE_SEL_CHANGED(ID_FILEEXPLODLG_ExplorerControl, FileOrFolderDialog::OnTreeSelectionChanged) 
END_EVENT_TABLE()


FileOrFolderDialog::FileOrFolderDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxDialog(parent, id, title, pos, size, style | wxRESIZE_BORDER) {

  int gap = 8;

  topSizer = new wxBoxSizer(wxHORIZONTAL);
  verticalSizer = new wxFlexGridSizer(3, 1, gap, gap);
  buttonSizer = new wxFlexGridSizer(1, 3, 0, gap);

  explorerControl = new FileExplorerControl(this, ID_FILEEXPLODLG_ExplorerControl);
  explorerControl->SetRootPath(_T("MyComputer"));

  okButton = new wxButton(this, wxID_OK);
  cancelButton = new wxButton(this, wxID_CANCEL);


  buttonSizer->AddGrowableCol(0);

  buttonSizer->Add(new wxStaticText(this, wxID_ANY, _T(" ")), 0, wxEXPAND);
  buttonSizer->Add(okButton);
  buttonSizer->Add(cancelButton);




  verticalSizer->AddGrowableCol(0);
  verticalSizer->AddGrowableRow(0);
  
  verticalSizer->Add(explorerControl, 1, wxEXPAND);
  verticalSizer->Add(buttonSizer, 0, wxEXPAND);




  topSizer->Add(verticalSizer, 1, wxEXPAND | wxALL, gap);
  SetSizer(topSizer);


  okButton->Enable(explorerControl->GetSelection().IsOk());

  Localize();
}


wxString FileOrFolderDialog::GetSelectedPath() {
  return explorerControl->GetSelectedPath();
}


void FileOrFolderDialog::Localize() {
  okButton->SetLabel(_("OK"));
  cancelButton->SetLabel(_("Cancel"));
}



void FileOrFolderDialog::OnTreeSelectionChanged(wxTreeEvent& evt) {
  okButton->Enable(explorerControl->GetSelection().IsOk());
}


