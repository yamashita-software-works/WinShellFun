#include "stdafx.h"
#include "shellfolderwindow.h"

static HINSTANCE hInstance = NULL;
static PWSTR g_pszIniFilePath = NULL;

HINSTANCE _GetResourceInstance()
{
	return hInstance;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hInstance = hModule;
			_MemInit();
			break;
		case DLL_PROCESS_DETACH:
			_SafeMemFree(g_pszIniFilePath);
			_MemEnd();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

EXTERN_C
HWND
WINAPI
ShellFolderCreateWindow(
	HWND hwnd
	)
{
	return ShellFolderWindow_CreateWindow(hwnd);
}

EXTERN_C
BOOL
WINAPI
ShellFolderInitData(
	HWND hwndSHFolder,
	PCWSTR pszReserved,
	RECT *prc
	)
{
	if( pszReserved && g_pszIniFilePath == NULL )
		g_pszIniFilePath = _MemAllocString(pszReserved);

	ShellFolderWindow_InitData(hwndSHFolder,NULL);
	ShellFolderWindow_InitLayout(hwndSHFolder,prc);
	return TRUE;
}

EXTERN_C
BOOL
WINAPI
GetIniFilePath(
	PWSTR pszIniPath,
	int cchIniPath
	)
{
	if( g_pszIniFilePath )
	{
		if( StringCchCopy(pszIniPath,cchIniPath,g_pszIniFilePath) == S_OK )
			return TRUE;
	}
	return FALSE;
#if 0
	// Try to get dll module name ini file.
	GetModuleFileName(hInstance,pszIniPath,cchIniPath);
	PathRemoveExtension(pszIniPath);
	PathAddExtension(pszIniPath,L".ini");

	if( !PathFileExists(pszIniPath) )
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
#endif
}
