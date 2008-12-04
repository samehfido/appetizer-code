/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "../stdafx.h"

#include "azOptionButton.h"
#include "azMenu.h"

#include "../ExtendedMenuItem.h"
#include "../MiniLaunchBar.h"
#include "LuaUtil.h"


//*****************************************************************
//
// LUNAR DATA
//
//*****************************************************************

const char azOptionButton::className[] = "OptionButton";

#define method(class, name) {#name, &class::name}

Lunar<azOptionButton>::RegType azOptionButton::methods[] = {
  method(azOptionButton, setToolTip),
  method(azOptionButton, addEventListener),
  method(azOptionButton, popupMenu),
  {0,0}
};


//*****************************************************************
//
// NON-EXPORTED MEMBERS
//
//*****************************************************************


OptionButton* azOptionButton::Get() const {
  OptionButton* b = wxGetApp().GetMainFrame()->GetOptionPanel()->GetButtonById(buttonId_);
  if (b) return b;
  
  // The button may be on the null panel if it has been created but hasn't been
  // added to the option panel yet.
  b = (OptionButton*)(wxGetApp().GetMainFrame()->GetNullPanelObjectById(buttonId_));
  if (b) return b;

  return NULL;
}


void azOptionButton::OnMenuClick(wxCommandEvent& evt) {
  ExtendedMenuItem* menuItem = GetClickedMenuItem(evt);
  if (!menuItem) return;

  bool handled = wxGetApp().GetPluginManager()->HandleMenuItemClick(menuItem);
  
  if (!handled) evt.Skip();
}


//*****************************************************************
//
// EXPORTED MEMBERS
//
//*****************************************************************


azIMPLEMENT_EVENT_LISTENER_FUNCTION(azOptionButton)


azOptionButton::azOptionButton(lua_State *L) {
  OptionButton* b = new OptionButton(wxGetApp().GetMainFrame()->GetNullPanel(), wxGetApp().GetUniqueInt());
  buttonId_ = b->GetId();
}


int azOptionButton::setToolTip(lua_State *L) {
  CheckWrappedObject(L, Get());

  wxString toolTip = LuaUtil::ToString(L, 1);
  Get()->SetToolTip(toolTip);
  return 0;
}


int azOptionButton::popupMenu(lua_State *L) {
  CheckWrappedObject(L, Get());

  const azMenu* menu = Lunar<azMenu>::check(L, 1);

  OptionButton* button = Get();

  int screenX = button->GetRect().GetX();
  int screenY = button->GetRect().GetY();

  if (!button->GetParent()) return 0; // shouldn't happen

  menu->Get()->Connect(
    wxID_ANY,
    wxEVT_COMMAND_MENU_SELECTED,
    wxCommandEventHandler(azOptionButton::OnMenuClick),
    NULL,
    this);

  wxWindow* parent = button->GetParent();
  if (!parent) return 0;

  parent->PopupMenu(menu->Get(), screenX, screenY + button->GetRect().GetHeight());

  return 0;
}