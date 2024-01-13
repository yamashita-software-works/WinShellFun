#pragma once

enum {
	VIEW_NULL=0,
	VIEW_FOLDERCONTENTSBROWSER,
	VIEW_RECYCLEBINFOLDER,
	VIEW_TYPE_MAX_COUNT,
};

EXTERN_C
HWND
WINAPI
CreateShellFunWindow(
	HWND hwnd,
	UINT ViewType
	);

EXTERN_C
BOOL
WINAPI
SetIniFilePath(
	PCWSTR pszIniPath
	);

EXTERN_C
BOOL
WINAPI
GetIniFilePath(
	PWSTR pszIniPath,
	int cchIniPath
	);
