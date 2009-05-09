/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "../stdafx.h"
#include "../PluginPreferences.h"

#include "PluginPreferencesDialog.h"


BEGIN_EVENT_TABLE(PluginPreferencesDialog, wxDialog)
  EVT_BUTTON(wxID_ANY, PluginPreferencesDialog::OnButtonClicked) 
END_EVENT_TABLE()


PluginPreferencesDialog::PluginPreferencesDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxDialog(parent, id, title, pos, size, style) {
  preferences_ = NULL;
}


PluginPreferencesDialog::~PluginPreferencesDialog() {
  for(int i = 0; i < controls_.size(); i++) {
    PluginPreferenceDialogControl* controlData = controls_.at(i);
    wxDELETE(controlData);
  }
  controls_.clear();
}


void PluginPreferencesDialog::LoadPreferences(PluginPreferences* preferences, bool flatView, const wxString& saveButtonLabel) {
  SetTitle(_("Preferences"));

  preferences_ = preferences;
  PluginPreferenceGroupVector preferenceGroups = preferences->GetPreferenceGroups();

  int border = 10;
  int gap = 8;
  int windowWidth = 400;

  wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
  wxNotebook* notebook = flatView ? NULL : new wxNotebook(this, wxID_ANY);
  wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);


  std::map<wxString, wxPanel*> panels;
  std::map<wxString, wxFlexGridSizer*> panelGridSizers;


  int groupIndex = -1;
  int groupCount = notebook ? preferenceGroups.size() : 0;

  while (groupIndex < groupCount) {
    wxString groupTitle;
    wxString groupName;
    int preferenceCount = 0;

    if (groupIndex == -1) {
      groupTitle = _("General");
      groupName = _T("general");
      preferenceCount = preferences->CountGroupPreferences(_T(""));
    } else {
      PluginPreferenceGroup* group = preferenceGroups.at(groupIndex);
      groupTitle = group->Title;
      groupName = group->Name;
      preferenceCount = preferences->CountGroupPreferences(group->Name);
    }

    wxPanel* panel = NULL;

    if (notebook) {
      panel = new wxPanel(notebook);
      notebook->AddPage(panel, groupTitle);
      panels[groupName] = panel;
    }

    wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
    if (notebook) {
      panel->SetSizer(boxSizer);
    } else {
      rootSizer->Add(boxSizer, 1, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 4);
    }

    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(preferenceCount, 2, 8, 8);
    boxSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 10);
    panelGridSizers[groupName] = gridSizer;
    gridSizer->AddGrowableCol(1);

    groupIndex++;
  }
  


  for (int i = 0; i < preferences->Count(); i++) {
    PluginPreference* preference = preferences->GetPreferenceAt(i);
    PluginPreferenceGroup* preferenceGroup = preference->GetGroup();

    wxWindow* groupPanel = NULL;
    wxFlexGridSizer* gridSizer = NULL;

    if (!notebook) {

      groupPanel = this;
      gridSizer = panelGridSizers[_T("general")];

    } else {

      if (preferenceGroup) {
        groupPanel = panels[preferenceGroup->Name];
        gridSizer = panelGridSizers[preferenceGroup->Name];
      } else {
        groupPanel = panels[_T("general")];
        gridSizer = panelGridSizers[_T("general")];
      }

    }

    wxStaticText* label = new wxStaticText(groupPanel, wxID_ANY, preference->GetTitle(), wxDefaultPosition, wxDefaultSize);
    gridSizer->Add(label, 0, wxEXPAND | wxTOP, 2);

    PluginPreferenceDialogControl* controlData = new PluginPreferenceDialogControl();

    wxWindow* control = NULL;
    bool controlAdded = false;
    bool showLabel = true;

    if (preference->GetType() == PluginPreferenceType::Text) {

      control = new wxTextCtrl(groupPanel, wxID_ANY);
      wxTextCtrl* textBox = dynamic_cast<wxTextCtrl*>(control);
      textBox->SetValue(preference->GetValue());

    } else if (preference->GetType() == PluginPreferenceType::TextArea) {

      control = new wxTextCtrl(groupPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
      wxTextCtrl* textBox = dynamic_cast<wxTextCtrl*>(control);
      textBox->SetValue(preference->GetValue());
      textBox->SetMinSize(wxSize(100, 100));
      textBox->SetSize(textBox->GetSize().GetWidth(), 100);

    } else if (preference->GetType() == PluginPreferenceType::CheckBox) {

      control = new wxCheckBox(groupPanel, wxID_ANY, wxEmptyString);
      wxCheckBox* checkBox = dynamic_cast<wxCheckBox*>(control);
      checkBox->SetValue(preference->GetValue() == _T("1") || preference->GetValue().Lower() == _T("true"));
      checkBox->SetLabel(preference->GetTitle());

      showLabel = false;

    } else if (preference->GetType() == PluginPreferenceType::File) {

      controlAdded = true;

      wxFlexGridSizer* sizer = new wxFlexGridSizer(1, 2, 4, 4);
      sizer->AddGrowableCol(0);

      control = new wxTextCtrl(groupPanel, wxID_ANY);
      wxTextCtrl* textBox = dynamic_cast<wxTextCtrl*>(control);
      textBox->SetValue(preference->GetValue());

      wxButton* browseButton = new wxButton(groupPanel, wxID_ANY);
      browseButton->SetLabel(_T("..."));
      browseButton->SetMinSize(wxSize(30, browseButton->GetMinHeight()));
      
      sizer->Add(textBox, 0, wxEXPAND, 0);
      sizer->Add(browseButton, 0, 0, 0);           

      gridSizer->Add(sizer, 0, wxEXPAND, 0);

    } else if (preference->GetType() == PluginPreferenceType::Popup) {

      control = new wxComboBox(groupPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_READONLY);
      wxComboBox* comboBox = dynamic_cast<wxComboBox*>(control);

      PluginPreferenceOptions options = preference->GetOptions();
      PluginPreferenceOptions::iterator it;
      int selectedIndex = 0;
      int currentIndex = 0;
      
      for(it = options.begin(); it != options.end(); ++it) {
        wxStringClientData* data = new wxStringClientData(it->first);
        comboBox->Append(it->second, data);
        if (it->first == preference->GetValue()) {
          selectedIndex = currentIndex;
        }
        currentIndex++;
      }

      comboBox->Select(selectedIndex);

    }

    if (!showLabel) label->SetLabel(_T(""));
    if (!controlAdded) gridSizer->Add(control, 0, wxEXPAND, 0);  

    controlData->control = control;
    controlData->label = label;
    controlData->preference = preference;

    controls_.push_back(controlData);

  }


  wxStaticText* bottomSizerFiller = new wxStaticText(this, wxID_ANY, _T(""));
  wxButton* saveButton = new wxButton(this, wxID_SAVE, saveButtonLabel);
  wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"));


  bottomSizer->Add(bottomSizerFiller, 1, wxEXPAND);
  bottomSizer->Add(saveButton, 0, wxRIGHT | wxALIGN_BOTTOM, border);
  bottomSizer->Add(cancelButton, 0, wxALIGN_BOTTOM, 0);

  if (notebook) rootSizer->Add(notebook, 1, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 4);
  rootSizer->Add(bottomSizer, 0, wxALL | wxEXPAND | wxALIGN_RIGHT, border);


  SetSizer(rootSizer);
  rootSizer->SetSizeHints(this);

  SetSize(windowWidth, GetSize().GetHeight());

}


void PluginPreferencesDialog::OnButtonClicked(wxCommandEvent& evt) {
  switch (evt.GetId()) {

    case wxID_SAVE:

      for (int i = 0; i < controls_.size(); i++) {
        PluginPreferenceDialogControl* controlData = controls_.at(i);
        PluginPreference* preference = controlData->preference;
        
        wxComboBox* comboBox = dynamic_cast<wxComboBox*>(controlData->control);
        if (comboBox) {
          wxStringClientData* clientData = (wxStringClientData*)(comboBox->GetClientObject(comboBox->GetSelection()));
          preference->SetValue(clientData->GetData());
          continue;
        }

        wxTextCtrl* textBox = dynamic_cast<wxTextCtrl*>(controlData->control);
        if (textBox) {
          preference->SetValue(textBox->GetValue());
          continue;
        }
        
      }

      preferences_->ScheduleSave();

      EndModal(wxID_CLOSE);
      
      break;

    default:

      evt.Skip();

  }
}