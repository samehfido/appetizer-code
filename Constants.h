﻿/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "stdafx.h"

#ifndef __Constants_H
#define __Constants_H

const wxString APPLICATION_NAME = _T("Appetizer");
const wxColor MASK_COLOR = wxColor(255, 0, 255);
const wxString DATA_FOLDER_NAME = _T("Data");
const wxString SETTING_FILE_NAME = _T("Settings.xml");
const wxString FOLDER_ITEMS_FILE_NAME = _T("FolderItems.xml");
const wxString SETTING_FOLDER_NAME = _T("Settings");
const wxString SKIN_FOLDER_NAME = _T("Skin");
const wxString LOCALES_FOLDER_NAME = _T("Locales");
const wxString ICONS_FOLDER_NAME = _T("Icons");
const wxString HELP_FOLDER_NAME = _T("Help");
const wxString WINDOW_FILE_NAME = _T("Window.xml");
const wxString HELP_FILE_NAME = _T("Appetizer.chm");
const wxString SKIN_FILE_NAME = _T("Skin.xml");
const wxString DEFAULT_SKIN = _T("Default");
const wxString CHECK_VERSION_URL = _T("http://appetizer.sourceforge.net/VersionInfo.xml");

const int SHELL32_ICON_INDEX_MY_COMPUTER = 16;
const int SHELL32_ICON_INDEX_MY_NETWORK = 18;
const int SHELL32_ICON_INDEX_CONTROL_PANEL = 22;
const int SHELL32_ICON_INDEX_RECYCLE_BIN = 32;
const int SHELL32_ICON_INDEX_DESKTOP = 35;
const int SHELL32_ICON_INDEX_EXPLORER = 46;
const int SHELL32_ICON_INDEX_SEARCH = 56;
const int SHELL32_ICON_INDEX_MY_DOCUMENTS = 127;
const int SHELL32_ICON_INDEX_MY_PICTURES = 128;
const int SHELL32_ICON_INDEX_MY_MUSIC = 129;
const int SHELL32_ICON_INDEX_MY_VIDEOS = 130;

const int HOT_KEY_ID = 0xBAFF;
const int CHECK_VERSION_DAY_INTERVAL = 2;
const int WINDOW_VISIBILITY_BORDER = 20;
const int MAIN_FRAME_DEFAULT_WIDTH = 300;
const int MAIN_FRAME_DEFAULT_HEIGHT = 80;
const int SMALL_ICON_SIZE = 16;
const int LARGE_ICON_SIZE = 32;
const int EXTRA_LARGE_ICON_SIZE = 48;
const int JUMBO_ICON_SIZE = 256;

#endif // __Constants_H