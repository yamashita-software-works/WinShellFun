#pragma once

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#include "targetver.h"

#if _MSC_VER <= 1500
#define nullptr NULL
#endif

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shlguid.h>
#include <strsafe.h>
#include <stgprop.h>
#include <commoncontrols.h>
#include <uxtheme.h>
#include <commdlg.h>

#include "debug.h"
#include "mem.h"
#include "libmisc.h"
#include "builddefs.h"
#include "common_control_helper.h"

#include "dparray.h"
#include "common.h"
#include "..\inc\common_resid.h"
#include "..\inc\common_shellfun.h"
#include "..\inc\interface.h"
#include "..\inc\listhelp.h"
#include "appdef_resid.h"

#define  _ASSERT ASSERT
#define SetRedraw(h,f)	SendMessage(h,WM_SETREDRAW,(WPARAM)f,0)
#define GETINSTANCE(hWnd)   (HINSTANCE)GetWindowLongPtr(hWnd,GWLP_HINSTANCE)
#define GETCLASSBRUSH(hWnd) (HBRUSH)GetClassLongPtr(hWnd,GCLP_HBRBACKGROUND)
HINSTANCE _GetResourceInstance();

typedef struct _FILEITEMHEADER
{
	ULONG Reserved;
	PWSTR Path;
	PWSTR FileName;
} FILEITEMHEADER;

typedef struct _FILEITEM
{
	FILEITEMHEADER hdr;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    ULONG FileIndex;
    LARGE_INTEGER FileId;
    WCHAR ShortName[14];
} FILEITEM,*PFILEITEM;

class CFileItem : public FILEITEM
{
public:
	CFileItem()
	{
		memset(this,0,sizeof(FILEITEM));
	}

	CFileItem(PCWSTR pszDirPath,PCWSTR pszFile)
	{
		memset(this,0,sizeof(FILEITEM));
		if( pszDirPath )
			hdr.Path = _MemAllocString(pszDirPath);
		if( pszFile )
			hdr.FileName = _MemAllocString(pszFile);
	}

	~CFileItem()
	{
		_SafeMemFree(hdr.Path);
		_SafeMemFree(hdr.FileName);
	}
};

typedef struct _SHELLITEM
{
	LPITEMIDLIST pidl;
	int EnumOrderIndex;
} SHELLITEM,*PSHELLITEM;

class CShellItem : 
	public CFileItem, 
	public SHELLITEM
{
public:
	CShellItem()
	{
		pidl = NULL;
	}

	CShellItem(PCWSTR pszDirPath,PCWSTR pszFile) 
		: CFileItem(pszDirPath,pszFile)
	{
		pidl = NULL;
	}

	~CShellItem()
	{
		if( pidl )
			CoTaskMemFree(pidl);
	}
};

typedef struct _SHELLITEMLIST
{
	PVOID Reserved;
	ULONG cItemCount;
	CShellItem **pFI;
} SHELLITEMLIST,*PSHELLITEMLIST;

//
// Folder tree item ident
//
EXTERN_C
BOOL
WINAPI
GetIniFilePath(
	PWSTR pszIniPath,
	int cchIniPath
	);

