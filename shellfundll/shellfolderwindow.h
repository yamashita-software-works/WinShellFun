#pragma once

#include "basewindow.h"

HWND ShellFolderWindow_CreateWindow(HWND hWnd);
VOID ShellFolderWindow_InitData(HWND hWndDirBrowse,PCWSTR pszDirectoryPath);
VOID ShellFolderWindow_InitLayout(HWND hWndDirBrowse,const RECT *prcDesktopWorkArea);

