#pragma once

HRESULT ShellFolderTree_CreateWindow(HWND hWnd,HWND *phWnd);
HRESULT ShellFolderTree_InitData(HWND hWnd,HWND hWndNotify);
HRESULT ShellFolderTree_InitLayout(HWND hWnd,RECT *prc);
HRESULT ShellFolderTree_SelectFolder(HWND hWnd,PCWSTR Reserved1,UINT Reserved2);
HRESULT ShellFolderTree_FillItems(HWND hWnd,PCWSTR Reserved);
HRESULT ShellFolderTree_SetNotifyWnd(HWND hWnd,HWND hWndNotify);
HRESULT ShellFolderTree_GetNotifyWnd(HWND hWnd,HWND *phWnd);
