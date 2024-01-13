//****************************************************************************
//
//  dllmain.cpp
//
//  Dll entry procedure
//
//  Auther: YAMASHITA Katsuhiro
//
//  Create: 2023.04.18
//
//****************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "shellfun.h"
#include "shellfolderwindow.h"
#include "recyclebinwindow.h"

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
CreateShellFunWindow(
	HWND hwnd,
	UINT ViewType
	)
{
	if( ViewType == VIEW_FOLDERCONTENTSBROWSER )
		return ShellFolderWindow_CreateWindow(hwnd);
	else if( ViewType == VIEW_RECYCLEBINFOLDER )
		return CreateRecycleBinWindow(hwnd);
	return NULL;
}

EXTERN_C
BOOL
WINAPI
SetIniFilePath(
	PCWSTR pszIniPath
	)
{
	_SafeMemFree( g_pszIniFilePath );
	if( pszIniPath )
	{
		g_pszIniFilePath = _MemAllocString(pszIniPath);
	}
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
}
