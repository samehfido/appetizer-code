// -*- C++ -*- generated by wxGlade 0.6.3

#include "GroupEditorDialogBase.h"

// begin wxGlade: ::extracode

// end wxGlade



GroupEditorDialogBase::GroupEditorDialogBase(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
    wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
{
    // begin wxGlade: GroupEditorDialogBase::GroupEditorDialogBase
    nameLabel = new wxStaticText(this, wxID_ANY, wxT("Name:"));
    nameTextBox = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    iconLabel = new wxStaticText(this, wxID_ANY, wxT("Icon:"));
    iconBitmap = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    changeIconButton = new wxButton(this, ID_GROUPEDIT_DLG_BUTTON_ChangeIcon, wxT("Select a different icon"));
    saveButton = new wxButton(this, ID_GROUPEDIT_DLG_BUTTON_Save, wxT("Save"));
    cancelButton = new wxButton(this, ID_GROUPEDIT_DLG_BUTTON_Cancel, wxT("Cancel"));

    set_properties();
    do_layout();
    // end wxGlade
}


void GroupEditorDialogBase::set_properties()
{
    // begin wxGlade: GroupEditorDialogBase::set_properties
    SetTitle(wxT("Edit group"));
    SetSize(wxSize(336, 158));
    iconBitmap->SetMinSize(wxSize(32, 32));
    // end wxGlade
}


void GroupEditorDialogBase::do_layout()
{
    // begin wxGlade: GroupEditorDialogBase::do_layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* saveCancelSizer = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* inputSizers = new wxFlexGridSizer(4, 2, 10, 8);
    wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
    inputSizers->Add(nameLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
    inputSizers->Add(nameTextBox, 0, wxEXPAND, 0);
    inputSizers->Add(iconLabel, 0, 0, 0);
    sizer_4->Add(iconBitmap, 0, 0, 0);
    sizer_4->Add(changeIconButton, 0, wxLEFT, 6);
    inputSizers->Add(sizer_4, 1, wxEXPAND, 0);
    inputSizers->AddGrowableCol(1);
    mainSizer->Add(inputSizers, 0, wxALL|wxEXPAND, 10);
    saveCancelSizer->Add(saveButton, 0, wxRIGHT|wxALIGN_BOTTOM, 8);
    saveCancelSizer->Add(cancelButton, 0, wxALIGN_BOTTOM, 0);
    mainSizer->Add(saveCancelSizer, 1, wxRIGHT|wxBOTTOM|wxALIGN_RIGHT, 10);
    SetSizer(mainSizer);
    Layout();
    // end wxGlade
}

