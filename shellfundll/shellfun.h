#pragma once

EXTERN_C
HWND
WINAPI
ShellFolderCreateWindow(
	HWND hwnd
	);

EXTERN_C
BOOL
WINAPI
ShellFolderInitData(
	HWND hwndSHFolder,
	PCWSTR pszReserved,
	RECT *prc
	);
