﻿/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "stdafx.h"

#include "FolderItem.h"
#include "utilities/IconGetter.h"
#include "utilities/VersionInfo.h"
#include "utilities/SystemUtil.h"
#include "utilities/StringUtil.h"
#include "MessageBoxes.h"
#include "Enumerations.h"
#include "FilePaths.h"
#include "Styles.h"
#include "MiniLaunchBar.h"


int appFolderItem::uniqueID_ = 1000;

FolderItemIdHashMap appFolderItem::folderItemIdHashMap_;

std::map<std::pair<wxString, int>, wxIcon*> appFolderItem::defaultIcons_;


appFolderItem::appFolderItem(bool isGroup) {
  appFolderItem::uniqueID_++;
  id_ = appFolderItem::uniqueID_;

  resolvedPath_ = _T("*"); // It means that it hasn't been resolved yet

  isDisposed_ = false;
  parent_ = NULL;
  isGroup_ = isGroup;
  uuid_ = wxEmptyString;
  customIconIndex_ = 0;

  automaticallyAdded_ = false;
  belongsToMultiLaunchGroup_ = false;
}


appFolderItem* appFolderItem::CreateFolderItem(bool isGroup) {
  appFolderItem* f = new appFolderItem(isGroup);
  folderItemIdHashMap_[f->GetId()] = f;
  return f;
}


appFolderItem* appFolderItem::GetFolderItemById(int id) {
  appFolderItem* sp = folderItemIdHashMap_[id];
  
  // The folder item is not (or no longer) in the hash map
  if (!sp) return NULL;

  if (sp->IsDisposed()) {
    // The folder item has been disposed, so remove it
    // from the hash map now and return NULL
    wxDELETE(sp);
    folderItemIdHashMap_.erase(id);

    return NULL;
  }

  // Otherwise return the pointer
  return sp;
}


void appFolderItem::Dispose() {
  if (isDisposed_) return;
  
  appFolderItem* parent = GetParent();
  if (parent) parent->RemoveChild(this);
  
  isDisposed_ = true;
}


bool appFolderItem::IsDisposed() {
  return isDisposed_;
}


// DO NOT MANUALLY DELETE FOLDER ITEMS. Instead,
// tag them for deletion using Dispose(). The
// garbage collector in DestroyStaticData()
// will do the rest.
appFolderItem::~appFolderItem() {
  ClearCachedIcons();
}


void appFolderItem::DestroyStaticData() {
  int s = folderItemIdHashMap_.size();

  FolderItemIdHashMap::iterator i;
  for(i = folderItemIdHashMap_.begin(); i != folderItemIdHashMap_.end(); ++i) {
    wxDELETE(i->second);
  }
  folderItemIdHashMap_.clear();

  std::map<std::pair<wxString, int>, wxIcon*>::iterator i2;
  for (i2 = defaultIcons_.begin(); i2 != defaultIcons_.end(); ++i2) wxDELETE(i2->second);
  defaultIcons_.clear();
}


void appFolderItem::SetCustomIcon(const wxString& filePath, int index) {
  if (filePath == customIconPath_ && index == customIconIndex_) return;

  customIconPath_ = appFolderItem::ConvertToRelativePath(filePath);
  customIconIndex_ = index;
  ClearCachedIcons();
}


wxString appFolderItem::GetCustomIconPath() {
  return customIconPath_;
}


int appFolderItem::GetCustomIconIndex() {
  return customIconIndex_;
}


void appFolderItem::SetParameters(const wxString& parameters) {
  if (parameters_ == parameters) return;
  parameters_ = wxString(parameters);
  parameters_.Trim(true).Trim(false);
}


wxString appFolderItem::GetParameters() {
  return parameters_;
}


wxString appFolderItem::GetUUID() {
  if (uuid_ == wxEmptyString) uuid_ = wxGetApp().GetUtilities().CreateUUID();
  return uuid_;
}


void appFolderItem::InsertChildBefore(appFolderItem* toAdd, appFolderItem* previousFolderItem) {
  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* child = children_.at(i);
    if (child->GetId() == previousFolderItem->GetId()) {
      MoveChild(toAdd, i);
      break;
    }
  }
}


void appFolderItem::InsertChildAfter(appFolderItem* toAdd, appFolderItem* previousFolderItem) {  
  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* child = children_.at(i);
    if (child->GetId() == previousFolderItem->GetId()) {
      MoveChild(toAdd, i + 1);
      break;
    }
  }
}


void appFolderItem::PrependChild(appFolderItem* toAdd) {
  MoveChild(toAdd, 0);
}


bool appFolderItem::IsGroup() {
  return isGroup_;
}


appFolderItem* appFolderItem::GetChildByResolvedPath(const wxString& filePath) {
  int childrenCount = children_.size();

  for (int i = 0; i < childrenCount; i++) {
    appFolderItem* child = children_.at(i);
    if (child->GetResolvedPath() == filePath) {
      return child;
    } else {
      appFolderItem* found = child->GetChildByResolvedPath(filePath);
      if (found) return found;
    }
  }

  return NULL;
}


bool appFolderItem::DoMultiLaunch() {
  bool output = false;

  if (!IsGroup()) {
    if (BelongsToMultiLaunchGroup()) {
      Launch();
      output = true;
    }
  } else {
    for (int i = 0; i < children_.size(); i++) {
      appFolderItem* folderItem = children_.at(i);
      if (folderItem->DoMultiLaunch()) output = true;
    }
  }

  return output;
}


void appFolderItem::MoveChild(appFolderItem* folderItemToMove, int insertionIndex) {
  // Create the new vector of folder items that
  // is going to replace the old one
  FolderItemVector newFolderItems;

  bool isPushed = false;

  if (insertionIndex < 0) {
    newFolderItems.push_back(folderItemToMove);
    isPushed = true;
  }

  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* folderItem = children_.at(i);

    // If the current folder item is the one
    // we want to move, skip it
    if (folderItem->GetId() == folderItemToMove->GetId()) continue;

    if (i == insertionIndex && !isPushed) {
      // If we are at the insertion index, insert the
      // the folder item
      newFolderItems.push_back(folderItemToMove);
      isPushed = true;
    }

    // Keep copying the other folder items
    newFolderItems.push_back(folderItem);
  }

  // If we didn't insert the folder item, do it now
  if (!isPushed) newFolderItems.push_back(folderItemToMove);

  // Swap the vectors
  children_ = newFolderItems;

  folderItemToMove->SetParent(this);

  // Notify everybody that we've changed the item collection
  wxGetApp().FolderItems_CollectionChange();
}


FolderItemVector appFolderItem::GetChildren(bool recursively) {
  if (!recursively) return children_;

  FolderItemVector output;

  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* child = children_.at(i);
    output.push_back(child);

    if (child->IsGroup()) {
      FolderItemVector temp = child->GetChildren(true);
      for (int j = 0; j < temp.size(); j++) output.push_back(temp.at(j));
    }
  }  

  return output;
}


bool appFolderItem::ContainsGroups() {
  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* child = children_.at(i);
    if (child->IsGroup()) return true;
  }  
  return false;
}


FolderItemVector appFolderItem::GetAllGroups(bool recursively) {
  FolderItemVector output;

  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* child = children_.at(i);
    if (child->IsGroup()) {
      output.push_back(child);    
      if (!recursively) continue;

      FolderItemVector temp = child->GetAllGroups(recursively);
      for (int j = 0; j < temp.size(); j++) output.push_back(temp.at(j));
    }
  }  

  return output;
}


void appFolderItem::AddChild(appFolderItem* folderItem) {
  if (folderItem->GetId() == GetId()) return;
  if (folderItem->IsGroup()) {
    appFolderItem* f = folderItem->GetChildById(GetId());
    if (f) return;
  }

  appFolderItem* p = folderItem->GetParent();
  if (p) p->RemoveChild(folderItem);

  folderItem->SetParent(this);
  children_.push_back(folderItem);
}


void appFolderItem::RemoveChild(appFolderItem* folderItem) {
  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* child = children_.at(i);

    if (child->GetId() == folderItem->GetId()) {

      if (folderItem->GetAutomaticallyAdded()) {
        wxGetApp().GetUser()->AddAutoAddExclusion(folderItem->GetResolvedPath());
      }
      
      child->SetParent(NULL);
      children_.erase(children_.begin() + i);

      wxGetApp().FolderItems_CollectionChange();
      return;
    }
  }

}


appFolderItem* appFolderItem::GetChildAt(int index) {
  return children_.at(index);
}


appFolderItem* appFolderItem::GetChildByUUID(const wxString& uuid, bool recurse) {
  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* folderItem = children_.at(i);

    if (folderItem->GetUUID() == uuid) return folderItem;

    if (recurse && folderItem->IsGroup()) {
      appFolderItem* temp = folderItem->GetChildByUUID(uuid, recurse);
      if (temp) return temp;
    }
  }

  return NULL;
}


appFolderItem* appFolderItem::GetChildById(int folderItemId, bool recurse) {
  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* folderItem = children_.at(i);

    if (folderItem->GetId() == folderItemId) return folderItem;

    if (recurse && folderItem->IsGroup()) {
      appFolderItem* temp = folderItem->GetChildById(folderItemId, recurse);
      if (temp) return temp;
    }
  }

  return NULL;
}


int appFolderItem::ChildrenCount() {
  return children_.size();
}


void appFolderItem::SetParent(appFolderItem* folderItem) {
  parent_ = folderItem;
}


appFolderItem* appFolderItem::GetParent() {
  return parent_;
}


int appFolderItem::GetId() const {
  return id_;
}


void appFolderItem::AddToMultiLaunchGroup() {
  belongsToMultiLaunchGroup_ = true;
}


void appFolderItem::RemoveFromMultiLaunchGroup() {
  belongsToMultiLaunchGroup_ = false;
}


bool appFolderItem::BelongsToMultiLaunchGroup() {
  return belongsToMultiLaunchGroup_;
}


void appFolderItem::OnMenuItemClick(wxCommandEvent& evt) {
  ExtendedMenuItem* menuItem = GetClickedMenuItem(evt);
  wxString name = menuItem->GetMetadata(_T("name"));

  if (name == _T("organizeShortcuts")) {

    int folderItemId = menuItem->GetMetadataInt(_T("folderItemId"));
    wxGetApp().GetUtilities().ShowTreeViewDialog(folderItemId);
  
  } else if (name == _T("folderItem")) {

    int folderItemId = menuItem->GetMetadataInt(_T("folderItemId"));
    appFolderItem* folderItem = wxGetApp().GetUser()->GetRootFolderItem()->GetChildById(folderItemId);
    if (!folderItem) {
      evt.Skip();
      return;
    }

    folderItem->Launch();
    wxGetApp().GetMainFrame()->DoAutoHide();

    LuaHostTable table;
    table[_T("dockItem")] = new LuaHostTableItem(folderItem, LHT_wxObject);
    wxGetApp().GetPluginManager()->DispatchEvent(&(wxGetApp()), _T("dockItemClick"), table);  

  } else {

    evt.Skip();

  }
}


void appFolderItem::AppendAsMenuItem(wxMenu* parentMenu, int iconSize, const wxString& menuItemName) {
  if (IsGroup()) {
    wxMenu* menu = ToMenu(iconSize, menuItemName);
    parentMenu->AppendSubMenu(menu, GetName(true));
  } else {
    ExtendedMenuItem* menuItem = ToMenuItem(parentMenu, iconSize, menuItemName);
    parentMenu->Append(menuItem);
  }

  parentMenu->Connect(
    wxID_ANY,
    wxEVT_COMMAND_MENU_SELECTED,
    wxCommandEventHandler(appFolderItem::OnMenuItemClick),
    NULL,
    this);
}


wxMenu* appFolderItem::ToMenu(int iconSize, const wxString& menuItemName) {
  if (!IsGroup()) {
    ELOG(_T("A non-group folder item cannot be converted to a menu. Call ToMenuItem() instead"));
    return NULL;
  }

  wxMenu* menu = new wxMenu();
  ExtendedMenuItem* menuItem = NULL;

  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* child = children_.at(i);
    if (!child) continue;
    child->AppendAsMenuItem(menu, iconSize, menuItemName);
  } 

  if (menu->GetMenuItemCount() > 0) menu->AppendSeparator();

  menuItem = new ExtendedMenuItem(menu, wxGetApp().GetUniqueInt(), _("Organize shortcuts"));
  menuItem->SetMetadata(_T("name"), _T("organizeShortcuts"));
  menuItem->SetMetadata(_T("folderItemId"), GetId());

  menu->Append(menuItem);

  menu->Connect(
    wxID_ANY,
    wxEVT_COMMAND_MENU_SELECTED,
    wxCommandEventHandler(appFolderItem::OnMenuItemClick),
    NULL,
    this);

  return menu;
}


ExtendedMenuItem* appFolderItem::ToMenuItem(wxMenu* parentMenu, int iconSize, const wxString& menuItemName) {
  ExtendedMenuItem* menuItem = new ExtendedMenuItem(parentMenu, wxGetApp().GetUniqueInt(), GetName(true));
  menuItem->SetMetadata(_T("name"), menuItemName);
  menuItem->SetMetadata(_T("folderItemId"), GetId());

  wxIcon* icon = GetIcon(iconSize);
  if (!icon->IsOk()) {
    ELOG(wxString::Format(_T("Icon is not ok for: %s"), GetName()));
  } else {
    wxBitmap iconBitmap(*icon);
    menuItem->SetBitmap(iconBitmap);
  }

  return menuItem;
}


/**
 * Search for the first folder item having a filename that matches the given string. The match mode
 * can take any of these values:
 * 1: A folder item's name must start with the specified string to be a match.
 * 2: A folder item's name can contain the string anywhere inside it to be a match. 
 * 3: A folder item's name must exactly match the string to be a match.
 * @param filename The filename to search for.
 * @param matchMode The matching behavior
 */
appFolderItem* appFolderItem::SearchChildByFilename(const wxString& filename, int matchMode) { 
  for (int i = 0; i < children_.size(); i++) {
    appFolderItem* folderItem = children_.at(i);
    if (!folderItem) continue;
    
    if (folderItem->IsGroup()) {
      appFolderItem* foundFolderItem = folderItem->SearchChildByFilename(filename, matchMode);
      if (foundFolderItem) return foundFolderItem;
    } else {
      wxString folderItemFilename = folderItem->GetFileName().Lower();
      if (folderItemFilename.Find(filename.Lower()) != wxNOT_FOUND) return folderItem;
    }
  }    

  return NULL;
}


void appFolderItem::Launch(const wxString& filePath, const wxString& arguments, bool notifyPlugins) {

  if (notifyPlugins) {
    wxString* filePathP = new wxString(filePath);
    wxString* argumentsP = new wxString(arguments);
    wxString* cancelP = new wxString(_T("0"));

    LuaHostTable eventTable;
    eventTable[_T("cancel")] = new LuaHostTableItem((wxObject*)cancelP, LHT_boolean);
    eventTable[_T("filePath")] = new LuaHostTableItem((wxObject*)filePathP, LHT_string);
    eventTable[_T("arguments")] = new LuaHostTableItem((wxObject*)argumentsP, LHT_string);
    wxGetApp().GetPluginManager()->DispatchEvent(&(wxGetApp()), _T("shorcutLaunching"), eventTable);

    wxString* sCancelled = (wxString*)(eventTable[_T("cancel")]->value);
    bool cancelled = sCancelled->Mid(0,1) == _T("1");
    
    wxDELETE(filePathP);
    wxDELETE(argumentsP);
    wxDELETE(cancelP);

    if (cancelled) {
      ILOG(_T("Launch has been cancelled by plugin"));
      return;
    }
  }
  
  //***************************************************************************
  // Launch web link
  //***************************************************************************

  if (StringUtil::IsWebLink(filePath)) {
    ::wxLaunchDefaultBrowser(filePath, wxBROWSER_NEW_WINDOW);
    return;
  }  

  wxFileName filename(filePath);

  if (filename.FileExists() || filename.IsRelative() && !SystemUtil::IsPathADrive(filePath)) {

    //***************************************************************************
    // Launch executable
    //***************************************************************************   

    if (filename.GetExt().Upper() == _T("EXE")) {
      // Set the current directory to the app directory
      wxString saveCurrentDirectory = wxGetCwd();
      wxSetWorkingDirectory(filename.GetPath());

      if (arguments == wxEmptyString) {

        // ---------------------------
        // Without arguments
        // ---------------------------
        wxExecute(filePath, wxEXEC_ASYNC);

      } else {

        // ---------------------------
        // With arguments
        // ---------------------------

        wxString tArguments(arguments); 
        // If the argument is a file path, then check that it has double quotes        
        if (wxFileName::FileExists(tArguments)) {
          tArguments = appFolderItem::ResolvePath(tArguments);
          if (tArguments[0] != _T('"') && tArguments[arguments.Len() - 1] != _T('"')) {
            tArguments = _T('"') + tArguments + _T('"');
          }          
        }
        wxExecute(wxString::Format(_T("%s %s"), filePath, tArguments));

      }

      // Restore the current directory
      wxSetWorkingDirectory(saveCurrentDirectory);

      return;
    }

    //***************************************************************************
    // Launch document
    //***************************************************************************

    wxString fileExtension = filename.GetExt();
    wxFileType* fileType = NULL;

    if (fileExtension != wxEmptyString) {
      fileType = wxTheMimeTypesManager->GetFileTypeFromExtension(fileExtension);
      if (!fileType) {
        MessageBoxes::ShowError(wxString::Format(_("This file doesn't exist or has been deleted (Error %s)"), _T("UnknownMimeType")));
        return;
      }
    } else {
      wxString filenameFullPath = filename.GetFullPath();

      SHELLEXECUTEINFO sei;
      memset(&sei,0,sizeof(SHELLEXECUTEINFO));
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.lpVerb = _T("openas");
      sei.lpFile = filenameFullPath.c_str();
      sei.nShow = SW_NORMAL;
      bool success = ShellExecuteEx(&sei);

      // Don't need to display an error if !success since Windows will do it

      return;
    }

    wxString command; 
    bool ok = fileType && fileType->GetOpenCommand(&command, wxFileType::MessageParameters(filename.GetFullPath(), wxEmptyString)); 
    if (!ok) {
      MessageBoxes::ShowError(wxString::Format(_("This file doesn't exist or has been deleted (Error %s)"), _T("CannotBuildCommand")));
    } else {
      wxExecute(command);
    }

    wxDELETE(fileType);    

  } else if (wxFileName::DirExists(filePath)) {
    // Strangely enough filename.DirExists() returns true
    // even if the directory doesn't exist, so we need to
    // use the static method wxFileName::DirExists() instead

    //***************************************************************************
    // Open folder
    //***************************************************************************

    #ifdef __WINDOWS__
    wxString command = _T("explorer ") + filename.GetFullPath();
    wxExecute(command);
    #else
    ELOG("TO BE IMPLEMENTED");
    #endif
  } else {
    MessageBoxes::ShowError(wxString::Format(_("This file doesn't exist or has been deleted (Error %s)"), _T("DoesNotExist")));
  }
}


wxString appFolderItem::GetSpecialItemFilePath(const wxString& specialItem) {

  wxString filePath;

  if (specialItem.Index(_T("$(")) != wxNOT_FOUND) {

    #ifdef __WINDOWS__

    OSVERSIONINFO osInfo = wxGetApp().GetOsInfo();

    if (specialItem == _T("$(ControlPanel)")) {

      filePath = FilePaths::GetSystem32Directory() + _T("\\control.exe");


    } else if (specialItem == _T("$(MyComputer)")) {

      filePath = FilePaths::GetWindowsDirectory() + _T("\\explorer.exe");


    } else if (specialItem == _T("$(MyNetwork)")) {

      filePath = FilePaths::GetWindowsDirectory() + _T("\\explorer.exe");


    } else if (specialItem == _T("$(Prompt)")) {

      filePath = FilePaths::GetSystem32Directory() + _T("\\cmd.exe");


    } else if (specialItem == _T("$(Calculator)")) {

      filePath = FilePaths::GetSystem32Directory() + _T("\\calc.exe");


    } else if (specialItem == _T("$(Write)")) {

      filePath = appFolderItem::ResolvePath(_T("%ProgramFiles%\\Windows NT\\Accessories\\wordpad.exe")); // Windows 7 (and maybe Vista)
      if (!wxFileName::FileExists(filePath)) filePath = FilePaths::GetSystem32Directory() + _T("\\write.exe");


    } else if (specialItem == _T("$(Paint)")) {

      filePath = FilePaths::GetSystem32Directory() + _T("\\pbrush.exe");
      if (!wxFileName::FileExists(filePath)) filePath = FilePaths::GetWindowsDirectory() + _T("\\pbrush.exe");
      if (!wxFileName::FileExists(filePath)) filePath = FilePaths::GetSystem32Directory() + _T("\\mspaint.exe");


    } else if (specialItem == _T("$(Notepad)")) {

      filePath = FilePaths::GetWindowsDirectory() + _T("\\notepad.exe");
      if (!wxFileName::FileExists(filePath)) filePath = FilePaths::GetSystem32Directory() + _T("\\notepad.exe");


    } else if (specialItem == _T("$(RecycleBin)")) {

      filePath = FilePaths::GetWindowsDirectory() + _T("\\explorer.exe");


    } else if (specialItem == _T("$(Printers)")) {

      filePath = FilePaths::GetWindowsDirectory() + _T("\\explorer.exe");


    } else if (specialItem == _T("$(NetworkConnections)")) {

      filePath = FilePaths::GetSystem32Directory() + _T("\\rundll32.exe");


    } else if (specialItem == _T("$(Explorer)")) {

      filePath = FilePaths::GetWindowsDirectory() + _T("\\explorer.exe");


    } else if (specialItem == _T("$(MyDocuments)")) {

      wxLogNull logNo;

      wxRegKey regKey(_T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"));
      if (regKey.Exists()) {
        regKey.QueryValue(_T("Personal"), filePath);
      } else {
        ELOG(_T("Couldn't get My Documents path"));
      }


    } else if (specialItem == _T("$(MyPictures)")) {

      wxLogNull logNo;

      wxRegKey regKey(_T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"));
      if (regKey.Exists()) {
        regKey.QueryValue(_T("My Pictures"), filePath);
      } else {
        ELOG(_T("Couldn't get My Pictures path"));
      }


    } else if (specialItem == _T("$(MyMusic)")) {

      wxLogNull logNo;                          
      wxRegKey regKey(_T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"));
      if (regKey.Exists()) {
        regKey.QueryValue(_T("My Music"), filePath);
      } else {
        ELOG(_T("Couldn't get My Music path"));
      }


    } else if (specialItem == _T("$(MyVideo)")) {

      wxLogNull logNo;

      wxRegKey regKey(_T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"));
      if (regKey.Exists()) {
        regKey.QueryValue(_T("My Video"), filePath);
      } else {
        ELOG(_T("Couldn't get My Video path"));
      }


    }

    #endif // __WINDOWS__


  }


  return filePath;
}


void appFolderItem::Launch() {

  wxString parameters;
  wxString filePath;



  if (filePath_.Index(_T("$(")) != wxNOT_FOUND) {

    #ifdef __WINDOWS__

    OSVERSIONINFO osInfo = wxGetApp().GetOsInfo();

    filePath = appFolderItem::GetSpecialItemFilePath(filePath_);
    
    if (filePath_ == _T("$(MyComputer)")) {

      parameters = _T(",::{20D04FE0-3AEA-1069-A2D8-08002B30309D}");


    } else if (filePath_ == _T("$(MyNetwork)")) {

      parameters = _T(",::{208D2C60-3AEA-1069-A2D7-08002B30309D}"); // XP and below
      if (osInfo.dwMajorVersion >= 6) parameters = _T("shell:NetworkPlacesFolder"); // Vista and above


    } else if (filePath_ == _T("$(RecycleBin)")) {

      parameters = _T(",::{645FF040-5081-101B-9F08-00AA002F954E}"); // XP and below  
      if (osInfo.dwMajorVersion >= 6) parameters = _T("shell:RecycleBinFolder"); // Vista and above


    } else if (filePath_ == _T("$(Printers)")) {

      parameters = _T(",::{2227A280-3AEA-1069-A2DE-08002B30309D}"); // XP and below  
      if (osInfo.dwMajorVersion >= 6) parameters = _T("shell:PrintersFolder"); // Vista and above


    } else if (filePath_ == _T("$(NetworkConnections)")) {

      parameters = _T("shell32.dll,Control_RunDLL ") + FilePaths::GetSystem32Directory() + _T("\\ncpa.cpl");


    } else if (filePath_ == _T("$(ShowDesktop)")) {

      wxString fileContent;
      fileContent += _T("Set shell = wscript.CreateObject(\"Shell.Application\")\n");
      fileContent += _T("Shell.MinimizeAll\n");

      wxGetApp().GetUtilities().CreateAndRunVBScript(
        wxString::Format(_T("%s/%s"), FilePaths::GetSettingsDirectory(), _T("ShowDesktop.vbs")),
        fileContent);

      return;


    } else if (filePath_ == _T("$(Search)")) {

      wxString fileContent = _T("CreateObject(\"Shell.Application\").FindFiles");

      wxGetApp().GetUtilities().CreateAndRunVBScript(
        wxString::Format(_T("%s/%s"), FilePaths::GetSettingsDirectory(), _T("FindFile.vbs")),
        fileContent);

      return;

    }

    #endif // __WINDOWS__


  }


  if (filePath == wxEmptyString) {
    parameters = parameters_;
    filePath = appFolderItem::ResolvePath(filePath_, false);
  }

    
  if (parameters != wxEmptyString) {
    appFolderItem::Launch(filePath, parameters);
  } else {
    appFolderItem::Launch(filePath);
  }
}


void appFolderItem::LaunchWithArguments(const wxString& arguments) {
  appFolderItem::Launch(appFolderItem::ResolvePath(filePath_, false), arguments);
}


TiXmlElement* appFolderItem::ToXml() {
  TiXmlElement* xml = new TiXmlElement("appFolderItem");

  XmlUtil::AppendTextElement(xml, "FilePath", GetFilePath());
  XmlUtil::AppendTextElement(xml, "Name", GetName());
  XmlUtil::AppendTextElement(xml, "AutomaticallyAdded", GetAutomaticallyAdded());
  XmlUtil::AppendTextElement(xml, "MultiLaunchGroup", BelongsToMultiLaunchGroup());
  XmlUtil::AppendTextElement(xml, "IsGroup", IsGroup());
  if (uuid_ != wxEmptyString) XmlUtil::AppendTextElement(xml, "UUID", uuid_);
  if (parameters_ != wxEmptyString) XmlUtil::AppendTextElement(xml, "Parameters", parameters_);  
  if (customIconPath_ != wxEmptyString) XmlUtil::AppendTextElement(xml, "CustomIcon", wxString::Format(_T("%s,%d"), customIconPath_, customIconIndex_));

  if (IsGroup()) {
    TiXmlElement* childrenXml = new TiXmlElement("Children");
    xml->LinkEndChild(childrenXml);

    for (int i = 0; i < children_.size(); i++) {
      appFolderItem* folderItem = children_.at(i);
      if (!folderItem) continue;
      childrenXml->LinkEndChild(folderItem->ToXml());
    }    
  }

  return xml;
}


void appFolderItem::FromXml(TiXmlElement* xml) {
  TiXmlHandle handle(xml);

  SetName(XmlUtil::ReadElementText(handle, "Name"));
  SetFilePath(XmlUtil::ReadElementText(handle, "FilePath"));
  SetAutomaticallyAdded(XmlUtil::ReadElementTextAsBool(handle, "AutomaticallyAdded"));
  belongsToMultiLaunchGroup_ = XmlUtil::ReadElementTextAsBool(handle, "MultiLaunchGroup");
  isGroup_ = XmlUtil::ReadElementTextAsBool(handle, "IsGroup");
  uuid_ = XmlUtil::ReadElementText(handle, "UUID");
  parameters_ = XmlUtil::ReadElementText(handle, "Parameters");

  wxArrayString customIconData;
  XmlUtil::ReadElementTextAsArrayString(handle, "CustomIcon", customIconData);
  if (customIconData.Count() >= 2) {
    customIconPath_ = customIconData[0];
    customIconIndex_ = 0;
    long t;
    if (customIconData[1].ToLong(&t)) customIconIndex_ = (int)t;
  }

  for (int i = 0; i < children_.size(); i++) children_[i]->Dispose();
  children_.clear();
  
  TiXmlElement* childrenXml = handle.Child("Children", 0).ToElement();
  if (childrenXml) {
    for (TiXmlElement* element = childrenXml->FirstChildElement(); element; element = element->NextSiblingElement()) {
      wxString elementName = wxString(element->Value(), wxConvUTF8);
      if (elementName != _T("appFolderItem")) continue;
      
      appFolderItem* folderItem = appFolderItem::CreateFolderItem();
      folderItem->FromXml(element);
      AddChild(folderItem);
    }
  }



  ConvertOldVariablesToNew(filePath_);
  ConvertOldVariablesToNew(parameters_);
  ConvertOldVariablesToNew(customIconPath_);
}


void appFolderItem::ConvertOldVariablesToNew(wxString& s) {
  if (s.Index(_T("%")) == wxNOT_FOUND) return;

  s.Replace(_T("%AZ_CONTROL_PANEL%"), _T("$(ControlPanel)"));
  s.Replace(_T("%AZ_MY_COMPUTER%"), _T("$(MyComputer)"));
  s.Replace(_T("%AZ_MY_NETWORK%"), _T("$(MyNetwork)"));
  s.Replace(_T("%AZ_RECYCLE_BIN%"), _T("$(RecycleBin)"));
  s.Replace(_T("%AZ_SHOW_DESKTOP%"), _T("$(ShowDesktop)"));
  s.Replace(_T("%AZ_EXPLORER%"), _T("$(Explorer)"));
  s.Replace(_T("%AZ_SEARCH%"), _T("$(Search)"));
  s.Replace(_T("%AZ_MY_DOCUMENTS%"), _T("$(MyDocuments)"));
  s.Replace(_T("%AZ_MY_PICTURES%"), _T("$(MyPictures)"));
  s.Replace(_T("%AZ_MY_MUSIC%"), _T("$(MyMusic)"));
  s.Replace(_T("%AZ_MY_VIDEO%"), _T("$(MyVideo)"));
  s.Replace(_T("%DRIVE%"), _T("$(Drive)"));
}


bool appFolderItem::GetAutomaticallyAdded() {
  return automaticallyAdded_;
}


void appFolderItem::SetAutomaticallyAdded(bool automaticallyAdded) {
  automaticallyAdded_ = automaticallyAdded;
}




wxString appFolderItem::GetName(bool returnUnnamedIfEmpty) {
  if (returnUnnamedIfEmpty) {
    if (name_ == wxEmptyString) {
      if (isGroup_) return _("Group");
      return _("Unnamed");
    }
  }
  return name_;
}


void appFolderItem::SetName(const wxString& name) {
  name_ = wxString(name);
  name_.Trim(true).Trim(false);
}


void appFolderItem::ClearCachedIcons() {
  for (IconHashMap::iterator i = icons_.begin(); i != icons_.end(); ++i) wxDELETE(i->second);
  icons_.clear();
}


#ifdef __MLB_USE_ICON_DISK_CACHE__

void appFolderItem::CacheIconToDisk(const wxString& hash, wxIcon* icon, int iconSize) {
  wxASSERT_MSG(icon, _T("Couldn't cache icon: null pointer"));
  wxASSERT_MSG(icon->IsOk(), _T("Couldn't cache icon: is not ok"));
  wxASSERT_MSG(hash != wxEmptyString, _T("Couldn't cache icon: empty hash"));

  wxBitmap bitmap(*icon);
  wxString cacheDirectory = FilePaths::GetIconCacheDirectory();
  if (!wxFileName::DirExists(cacheDirectory)) wxFileName::Mkdir(cacheDirectory);
  bitmap.SaveFile(wxString::Format(_T("%s/%s-%d.png"), cacheDirectory, hash, iconSize), wxBITMAP_TYPE_PNG);
}


wxString appFolderItem::GetIconDiskCacheHash() {
  if (IsGroup()) return wxEmptyString;
  if (iconCacheHash_ != wxEmptyString) return iconCacheHash_;

  wxFileName filename(GetResolvedPath());
  if (!filename.IsOk()) return _T("");

  wxString toHash;

  if (!wxFileName::DirExists(filename.GetFullPath())) {
    wxString modString(_T(""));
    wxString createString(_T(""));
    wxDateTime dtMod;
    wxDateTime dtCreate;
    filename.GetTimes(NULL, &dtMod, &dtCreate);  
    if (dtMod.IsValid()) modString = dtMod.Format();
    if (dtCreate.IsValid()) createString = dtCreate.Format();

    toHash = wxString::Format(_T("%s:%d:%s:%s"), filename.GetName(), filename.GetSize(), modString, createString);
  } else {
    toHash = wxString::Format(_T("%s"), GetFilePath());
  }
  
  wxMD5 md5(toHash);
  iconCacheHash_ = md5.GetDigest();

  return iconCacheHash_;
}


wxIcon* appFolderItem::GetIconFromDiskCache(const wxString& hash, int iconSize) {
  wxString cacheDirectory = FilePaths::GetIconCacheDirectory();
  wxIcon* output;

  if (!wxFileName::DirExists(cacheDirectory)) return output;

  if (hash != wxEmptyString) {
    // Look for it in the cache first
    wxString filePath = wxString::Format(_T("%s/%s-%d.png"), cacheDirectory, hash, iconSize);
    if (wxFileName::FileExists(filePath)) {
      output = new wxIcon(filePath, wxBITMAP_TYPE_PNG);
    }
  }

  return output;
}

#endif // __MLB_USE_ICON_DISK_CACHE__


wxIcon* appFolderItem::CreateDefaultGroupIcon(int iconSize) {
  std::pair<wxString, int> p(_T("group"), iconSize);

  if (defaultIcons_.find(p) != defaultIcons_.end()) {
    wxIcon* output = new wxIcon(*defaultIcons_[p]);
    return output;
  }

  wxIcon* output = new wxIcon(FilePaths::GetSkinFile(wxString::Format(_T("FolderIcon%d.png"), iconSize)), wxBITMAP_TYPE_PNG);
  defaultIcons_[p] = new wxIcon(*output);

  return output;
}


bool appFolderItem::IsWebLink() {
  return StringUtil::IsWebLink(GetFilePath());
}


wxIcon* appFolderItem::GetIcon(int iconSize) {

  iconSize = wxGetApp().GetOSValidIconSize(iconSize);

  // If the icon has already been generated, return it
  if (icons_.find(iconSize) != icons_.end()) return icons_[iconSize];

  wxIcon* output = NULL;  

  if (customIconPath_ != wxEmptyString) {
    
    wxFileName fn(customIconPath_);

    if (fn.GetExt().Lower() == _T("png")) {

      output = Imaging::CreateIconFromPng(appFolderItem::ResolvePath(customIconPath_), iconSize);

    } else {

      if (customIconIndex_ > 0) {
        output = IconGetter::GetExecutableIcon(appFolderItem::ResolvePath(customIconPath_), iconSize, customIconIndex_);
      } else {
        output = IconGetter::GetFolderItemIcon(appFolderItem::ResolvePath(customIconPath_), iconSize, true);
      }

    }
  }

  if (!output && IsWebLink()) {
    output = IconGetter::GetDefaultWebLinkIcon(iconSize);
  }  

  #ifdef __MLB_USE_ICON_DISK_CACHE__
  wxString cacheHash;
  cacheHash = GetIconDiskCacheHash();
  wxIcon* cachedIcon = appFolderItem::GetIconFromDiskCache(cacheHash, iconSize);
  if (cachedIcon) {
    icons_[iconSize] = cachedIcon;
    return cachedIcon;
  }
  #endif // __MLB_USE_ICON_DISK_CACHE__

  if (!output) {
    if (IsGroup()) {
      output = appFolderItem::CreateDefaultGroupIcon(iconSize);
    } else {
      output = appFolderItem::CreateSpecialItemIcon(GetFilePath(), iconSize);
    }
  }

  if (!output) {
    output = IconGetter::GetFolderItemIcon(GetResolvedPath(), iconSize);
    #ifdef __MLB_USE_ICON_DISK_CACHE__
    if (output) appFolderItem::CacheIconToDisk(cacheHash, output, iconSize);
    #endif // __MLB_USE_ICON_DISK_CACHE__
  }

  if (!output) {
    output = new wxIcon(FilePaths::GetSkinFile(wxString::Format(_T("DefaultIcon%d.png"), iconSize)), wxBITMAP_TYPE_PNG);
  }

  // Cache the icon
  icons_[iconSize] = output;

  return output;
}


wxIcon* appFolderItem::CreateSpecialItemIcon(const wxString& path, int iconSize) {
  wxIcon* output = NULL;

  if (path.Index(_T("$(")) == wxNOT_FOUND) return output;
  if (path.Index(_T("$(Drive)")) != wxNOT_FOUND) return output;

  std::pair<wxString, int> nameSizePair(path, iconSize);

  if (defaultIcons_.find(nameSizePair) != defaultIcons_.end()) {
    wxIcon* output = new wxIcon(*defaultIcons_[nameSizePair]);
    return output;
  }

  wxString dllPath = FilePaths::GetSystem32Directory() + _T("\\shell32.dll");

  // 48x48 icons are always at the normal index + 1
  int inc = iconSize >= 48 ? 1 : 0;

  if (path == _T("$(ControlPanel)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_CONTROL_PANEL + inc);
  } else if (path == _T("$(MyComputer)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_MY_COMPUTER + inc);
  } else if (path == _T("$(MyNetwork)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_MY_NETWORK + inc);
  } else if (path == _T("$(RecycleBin)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_RECYCLE_BIN + inc);
  } else if (path == _T("$(ShowDesktop)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_DESKTOP + inc);
  } else if (path == _T("$(Explorer)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_EXPLORER + inc);
  } else if (path == _T("$(Search)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_SEARCH + inc);
  } else if (path == _T("$(MyDocuments)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_MY_DOCUMENTS + inc);
  } else if (path == _T("$(MyPictures)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_MY_PICTURES + inc);
  } else if (path == _T("$(MyMusic)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_MY_MUSIC + inc);
  } else if (path == _T("$(MyVideo)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_MY_VIDEOS + inc);
  } else if (path == _T("$(Printers)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_PRINTERS + inc);
  } else if (path == _T("$(NetworkConnections)")) {
    output = IconGetter::GetExecutableIcon(dllPath, iconSize, SHELL32_ICON_INDEX_NETWORK_CONNECTIONS + inc);
  } else {
    output = IconGetter::GetExecutableIcon(appFolderItem::GetSpecialItemFilePath(path), iconSize);
  }

  if (output) {
    defaultIcons_[nameSizePair] = new wxIcon(*output);
  } else {
    defaultIcons_[nameSizePair] = NULL;
  }

  return output;
}


wxString appFolderItem::GetDisplayName(const wxString& unresolvedFilePath) {
  wxString theName;

  if (unresolvedFilePath.Index(_T("$(")) != wxNOT_FOUND) {
    if (unresolvedFilePath == _T("$(ControlPanel)")) {
      theName = _("Control Panel");
    } else if (unresolvedFilePath == _T("$(MyComputer)")) {
      theName = _("My Computer");
    } else if (unresolvedFilePath == _T("$(MyNetwork)")) {
      theName = _("My Network Places");
    } else if (unresolvedFilePath == _T("$(RecycleBin)")) {
      theName = _("Recycle Bin");
    } else if (unresolvedFilePath == _T("$(ShowDesktop)")) {
      theName = _("Show Desktop");
    } else if (unresolvedFilePath == _T("$(Explorer)")) {
      theName = _("Windows Explorer");
    } else if (unresolvedFilePath == _T("$(Search)")) {
      theName = _("Search");
    } else if (unresolvedFilePath == _T("$(Drive)")) {
      theName = _("Current Drive");
    } else if (unresolvedFilePath == _T("$(Printers)")) {
      theName = _("Printers and Faxes");
    } else if (unresolvedFilePath == _T("$(NetworkConnections)")) {
      theName = _("Network Connections");
    } else if (unresolvedFilePath == _T("$(Calculator)")) {
      theName = _("Calculator");
    } else if (unresolvedFilePath == _T("$(Paint)")) {
      theName = _("Paint Brush");
    } else if (unresolvedFilePath == _T("$(Notepad)")) {
      theName = _("Notepad");
    } else if (unresolvedFilePath == _T("$(Write)")) {
      theName = _("WordPad");
    } else if (unresolvedFilePath == _T("$(Prompt)")) {
      theName = _("Command Prompt");
    }
  }

  if (theName != wxEmptyString) {
    return theName;    
  } else {
    wxString s = VersionInfo::GetFileDescription(appFolderItem::ResolvePath(unresolvedFilePath));
    if (s != wxEmptyString) return s;
  }

  return unresolvedFilePath;
}


void appFolderItem::AutoSetName() {
  SetName(appFolderItem::GetDisplayName(filePath_));
}



void appFolderItem::SetFilePath(const wxString& filePath) {
  if (filePath == filePath_) return;

  resolvedPath_ = _T("*");
  ClearCachedIcons();
  filePath_ = wxString(filePath);
  filePath_.Trim();
}


wxString appFolderItem::GetFileName(bool includeExtension) {
  wxFileName f(GetResolvedPath());
  if (includeExtension) return f.GetFullName();
  return f.GetName();
}


wxString appFolderItem::GetFilePath() {
  return filePath_;
}


wxString appFolderItem::GetResolvedPath() {
  if (resolvedPath_ != _T("*")) return resolvedPath_;
  resolvedPath_ = appFolderItem::ResolvePath(filePath_);
  return resolvedPath_;
}


wxString appFolderItem::ResolvePath(const wxString& filePath, bool normalizeToo) {
  wxString result(filePath);

  if (result.Index(_T("$(")) != wxNOT_FOUND) {
    result.Replace(_T("$(Drive)"), FilePaths::GetApplicationDrive());
    result.Replace(_T("$(System32)"), FilePaths::GetSystem32Directory());
    result.Replace(_T("$(Windows)"), FilePaths::GetWindowsDirectory());
    result.Replace(_T("$(ControlPanel)"), FilePaths::GetSystem32Directory() + _T("\\control.exe"));
    result.Replace(_T("$(MyComputer)"), FilePaths::GetWindowsDirectory() + _T("\\explorer.exe"));
  }

  if (!normalizeToo) return result;

  wxFileName f(result);
  
  if (result.Len() <= 2) {
    if (SystemUtil::IsPathADrive(result)) {
      f = wxFileName(f.GetPathSeparator());
    }
  }
  
  f.Normalize();

  wxString n = f.GetName();

  return f.GetFullPath();
}


wxString appFolderItem::ConvertToRelativePath(const wxString& filePath) {
  wxString result(filePath);

  wxString test1 = result.Mid(0, 2).Upper();
  wxString test2 = FilePaths::GetApplicationDrive().Upper();
  
  if (result.Mid(0, 2).Upper() == FilePaths::GetApplicationDrive().Upper()) {
    result = _T("$(Drive)") + result.Mid(2, result.Len());
  }

  return result;
}