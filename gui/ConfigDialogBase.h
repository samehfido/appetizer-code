// -*- C++ -*- generated by wxGlade 0.6.3

#include <wx/wx.h>
#include <wx/image.h>
// begin wxGlade: ::dependencies
#include <wx/notebook.h>
// end wxGlade


#ifndef CONFIGDIALOGBASE_H
#define CONFIGDIALOGBASE_H


// begin wxGlade: ::extracode
enum {
ID_CDLG_BUTTON_Save,
ID_CDLG_BUTTON_Cancel,
ID_CDLG_BUTTON_CheckForUpdate
};
// end wxGlade


class ConfigDialogBase: public wxDialog {
public:
    // begin wxGlade: ConfigDialogBase::ids
    // end wxGlade

    ConfigDialogBase(wxWindow* parent, int id, const wxString& title, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE);

private:
    // begin wxGlade: ConfigDialogBase::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: ConfigDialogBase::attributes
    wxStaticText* languageLabel;
    wxComboBox* languageComboBox;
    wxStaticText* label_1;
    wxButton* checkForUpdateButton;
    wxPanel* notebook_1_pane_1;
    wxStaticText* iconSizeLabel;
    wxComboBox* iconSizeComboBox;
    wxStaticText* orientationLabel;
    wxComboBox* orientationComboBox;
    wxStaticText* skinLabel;
    wxComboBox* skinComboBox;
    wxPanel* notebook_pane_2;
    wxNotebook* notebook;
    wxButton* saveButton;
    wxButton* cancelButton;
    // end wxGlade
}; // wxGlade: end class


#endif // CONFIGDIALOGBASE_H
