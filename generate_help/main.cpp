/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/cmdline.h>
#include <wx/intl.h>
#include <wx/arrstr.h>
#include <wx/file.h>
#include <wx/regex.h>


// The application class. An instance is created and initialized
// below in IMPLEMENT_APP()
class GenerateHelp: public wxApp {
  virtual bool OnInit();
};


IMPLEMENT_APP(GenerateHelp) 


wxString gCurrentString;


void WriteHelp(const wxString& filePath, const wxString& htmlString) {
  wxFile file;
  file.Create(filePath, true);
  file.Open(filePath, wxFile::write);
  file.Write(htmlString, wxConvISO8859_1);
  file.Close();
}


void AddStringLn(const wxString& s) {
  gCurrentString += s + _T("\n");
}


void H1(const wxString& text) { gCurrentString += _T("\n<h1>") + text + _T("</h1>\n"); }
void H2(const wxString& text) { gCurrentString += _T("\n<h2>") + text + _T("</h2>\n"); }
void H3(const wxString& text) { gCurrentString += _T("\n<h3>") + text + _T("</h3>\n"); }
void P(const wxString& text) { gCurrentString += _T("<p>") + text + _T("</p>\n"); }
void BR() { gCurrentString += _T("<br/>\n"); }
void StartList() { gCurrentString += _T("<ul>\n"); } 
void EndList() { gCurrentString += _T("</ul>\n"); } 
void LI(const wxString& text) { gCurrentString += _T("<li>") + text + _T("</li>\n"); }
void A(const wxString& text, const wxString& href, const wxString& name) { gCurrentString += _T("<a href='") + href + _T("'") + _T(" name='") + name + _T("'>") + text + _T("</a>"); }


wxString GenerateHTMLString() {
  gCurrentString = _T("");

  AddStringLn(_T("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0 Transitional//EN'>"));
  AddStringLn(_T("<html>"));
  AddStringLn(_T("<head>"));
  AddStringLn(_T("<meta http-equiv='content-type' content='text/html; charset=Western-1252'>"));
  AddStringLn(_T("<title>Appetizer</title>"));
  //AddStringLn(_T("	<STYLE TYPE='text/css'>"));
  //AddStringLn(_T("	<!--"));
  //AddStringLn(_T("		body { padding: 0px }"));
  //AddStringLn(_T("		P { margin-bottom: 0px }"));
  //AddStringLn(_T("		H1 { margin-bottom: 0px }"));
  //AddStringLn(_T("		H2 { margin-bottom: 0px }"));
  //AddStringLn(_T("		H3 { margin-bottom: 0px }"));
  //AddStringLn(_T("	-->"));
  //AddStringLn(_T("	</STYLE>"));
  AddStringLn(_T("</head>"));
  AddStringLn(_T("<body>"));



  H1(_("Appetizer Help Topics"));

  // *******************************************************************************
  // INTRODUCTION
  // *******************************************************************************

  H2(_("Introduction"));
  H3(_("About Appetizer"));
  P(_("Appetizer is a dock bar that allows you to organize, display and launch your applications and other shortcuts. Currently it is possible to add executables, documents and folders to the dock. Additionally, if you use Appetizer on a USB key in the [http://portableapps.com PortableApps] format, the application will detect it and automatically add all your applications along with the 'Documents', 'Videos', 'Music' and 'Pictures' folders."));
  
  // -------------------------------------------------
  // Features
  // -------------------------------------------------
  H3(_("Features"));

  StartList();
  LI(_("Resizable dock, which allows displaying the icons both horizontally or vertically."));
  LI(_("Two size of icons supported: Small and large"));
  LI(_("Move the bar by clicking and dragging its border"));
  LI(_("Organize the icons by drag & dropping them"));
  LI(_("'Multi-launch' functionality"));
  LI(_("Support for multiple languages"));
  LI(_("Minimize to tray icon functionality"));
  EndList();


  // *******************************************************************************
  // USING APPETIZER
  // *******************************************************************************

  H2(_("Using Appetizer"));
  P(_("Appetizer is made to be simple and intuitive. To get more information about an icon or a button, simply move the mouse over it. Additionally, try right-clicking the dock in different places - certain parts of the application such as the icons or the icon panel provide various contextual actions."));
  
  // -------------------------------------------------
  // Adding, removing and changing a shortcut
  // -------------------------------------------------
  H3(_("Adding, removing and changing a shortcut"));
  StartList();
  LI(_("To add a shortcut, right-click anywhere on the icon panel and select 'New shortcut...';"));
  LI(_("To remove a shortcut, right-click on it and select 'Delete';"));
  LI(_("To change the properties of an icon, such as its name or location, right-click on it and select 'Properties';"));
  LI(_("To move an icon to a different location, click it and drag it."));
  EndList();

  // -------------------------------------------------
  // Minimizing / Maximizing the dock
  // -------------------------------------------------
  H3(_("Minimizing / Maximizing the dock"));
  P(_("The dock can be minimized to the tray icon by clicking on the 'X' button. To bring it back to front, simply click on the icon."));
  P(_("Right-click on the tray icon for additional options."));

  // -------------------------------------------------
  // The option panel
  // -------------------------------------------------
  A(_T(""), _T(""), _T("OptionPanel"));
  H3(_("The option panel"));
  P(_("The option panel pops up when you click on the big arrow button. If you move the mouse over any of its buttons, a tooltip will show up giving you more information."));

  // -------------------------------------------------
  // Launching several application simultaneously
  // -------------------------------------------------
  H3(_("Launching several application simultaneously"));
  P(_("Appetizer provides a functionality to launch several applications at the click of a button. This is equivalent to the Startup menu on Windows, except that it launches the app on your portable drive. To use this 'Multi-launch' group, follow these steps:"));
  StartList();
  LI(_("Right-click on any icon and select 'Multi-launch group' from the context menu"));
  LI(_("You can add as many shortcuts as you wish"));
  LI(_("To trigger the 'Multi-launch' group, open the [#OptionPanel option panel] and click on the 'Multi-launch' button"));
  EndList();

  // *******************************************************************************
  // CONFIGURING APPETIZER
  // *******************************************************************************
  H2(_("Configuring Appetizer"));
  H3(_("The configuration dialogue"));
  P(_("The configuration dialogue is accessible by clicking on the 'Configuration' button in the [#OptionPanel option panel]."));


  // *******************************************************************************
  // OTHER TIPS
  // *******************************************************************************
  H2(_("Other tips"));
  StartList();
  LI(_("You can resize the dock by clicking and dragging its bottom right corner"));
  EndList();

  AddStringLn(_T("</body>"));
  AddStringLn(_T("</html>"));

  return gCurrentString;
}


bool GenerateHelp::OnInit() {
  wxArrayString localeCodes;
  localeCodes.Add(_T("en"));
  localeCodes.Add(_T("fr"));  

  for (int i = 0; i < localeCodes.Count(); i++) {
    wxString localeCode = localeCodes[i];
    const wxLanguageInfo* info = wxLocale::FindLanguageInfo(localeCode);
    if (!info) {
      wxLogDebug(_T("CANNOT GET LANGUAGE INFO"));
      continue;
    }

    wxLocale locale;
    locale.Init(info->Language);
    locale.AddCatalogLookupPathPrefix(_T("Data/Help"));
    locale.AddCatalog(_T("appetizer_help"));

    wxString htmlString = GenerateHTMLString();

    wxRegEx urlRegEx(_T("\\[((ftp|http|https)://[^\\s]+)\\s([^\\]]+)\\]"), wxRE_ADVANCED);    
    wxRegEx strongRegEx(_T("\\[b\\](.*)\\[\\/b\\]"), wxRE_ADVANCED);
    wxRegEx internalUrlRegEx(_T("\\[(#[^\\s]+)\\s([^\\]]+)\\]"), wxRE_ADVANCED);

    urlRegEx.ReplaceAll(&htmlString, _T("<a href='\\1'>\\3</a>"));
    strongRegEx.ReplaceAll(&htmlString, _T("<b>\\1</b>"));
    internalUrlRegEx.ReplaceAll(&htmlString, _T("<a href='\\1'>\\2</a>"));

    WriteHelp(_T("Data/Help/") + localeCode + _T("/Appetizer.html"), htmlString); 
  }

  return false;
} 