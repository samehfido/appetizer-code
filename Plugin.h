/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "stdafx.h"

#ifndef __Plugin_H
#define __Plugin_H


#include "TypeDefinitions.h"
#include "ExtendedMenuItem.h"
#include "utilities/LuaUtil.h"


class Plugin {

public:

  Plugin();
  ~Plugin();

  void LoadFile(const wxString& luaFilePath);
  void AddEventListener(void* object, int eventId, const wxString& functionName);
  void AddEventListener(void* object, const wxString& eventName, const wxString& functionName);
  void DispatchEvent(wxObject* senderOrGlobalHook, int eventId, LuaHostTable arguments, wxObject* sender = NULL);
  void DispatchEvent(wxObject* senderOrGlobalHook, const wxString& eventName, LuaHostTable arguments, wxObject* sender = NULL);
  lua_State* GetLuaState();
  bool HandleMenuItemClick(ExtendedMenuItem* menuItem);

private:

  lua_State* L;

  std::map<std::pair<void*, int>, wxArrayString*> eventRegister_;

};


typedef boost::shared_ptr<Plugin> PluginSP;


#endif // __Plugin_H