-- Copyright (C) 2008 Laurent Cozic. All right reserved.
-- Use of this source code is governed by a GNU/GPL license that can be
-- found in the LICENSE file.

-- This script open the folder containing the given shortcut
--
-- An "Open containing folder" item is added to each icon popup menu.


function openContainingFolder_click(event) 
	shortcut = appetizer:getShortcutById(event.menuItem:getTag())
	
	system:runCommand("explorer.exe /select,"..shortcut:getResolvedPath(), true)
end


-- This function is called when an icon is right-clicked,
-- before the popup menu is displayed
function appetizer_iconMenuOpening(event)
	-- Get the icon
	icon = event.icon
	
	-- Get the icon popup menu and associated shortcut object
	menu = icon:getPopupMenu()	
	shortcut = icon:getShortcut()
	
	menuItem = MenuItem:new("Open containing folder")
	menuItem:setOnSelected("openContainingFolder_click")
	menuItem:setTag(shortcut:getId()) -- keep a reference to the shortcut
	
	-- Add a separator and append the sub menu to the icon popup menu
	menu:appendSeparator()
	menu:append(menuItem)
end


-- Register a global event handler. "application_iconMenuOpening()" is going
-- to be called whenever an icon popup menu is about to be displayed
appetizer:addEventListener("iconMenuOpening", "appetizer_iconMenuOpening")