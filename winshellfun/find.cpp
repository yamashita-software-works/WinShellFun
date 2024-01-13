//
//  2023.12.14 Create
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "winshellfun.h"

static UINT g_uFindMessage = 0;
static HWND g_hwndFindDialog = NULL;
static FINDREPLACE g_fr = {0};
static int g_iFindAction = 0;
static WCHAR *g_pszFindText = NULL;

#define _MAX_FINDTEXT MAX_PATH

VOID InitFind()
{
	g_uFindMessage = RegisterWindowMessage(FINDMSGSTRING);
}

UINT_PTR
CALLBACK
FRHookProc(
	HWND hdlg,
	UINT uiMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if( uiMsg == WM_INITDIALOG )
	{
		_CenterWindow(hdlg,GetActiveWindow());
		return TRUE;
	}
	return 0;
}

void OnEditFind(HWND hWnd,UINT CmdId)
{
	HWND hwndView = _GetActiveView();

	if( ID_EDIT_FIND == CmdId )
	{
		SendMessage(hwndView,FSM_FINDITEM,FIND_QUERYOPENDIALOG,(LPARAM)0);

		if( g_pszFindText == NULL )
		{
			g_pszFindText = new WCHAR[_MAX_FINDTEXT];
			memset(g_pszFindText,0,sizeof(WCHAR[_MAX_FINDTEXT]));
		}

		memset(&g_fr,0,sizeof(g_fr));

		g_iFindAction = 0;

		g_fr.hInstance     = 0;
		g_fr.hwndOwner     = hWnd;
		g_fr.lStructSize   = sizeof(FINDREPLACE);
		g_fr.lpstrFindWhat = g_pszFindText;
		g_fr.wFindWhatLen  = _MAX_FINDTEXT;
		g_fr.lpfnHook      = &FRHookProc;
		g_fr.Flags         = FR_FINDNEXT|FR_DOWN|FR_HIDEWHOLEWORD|FR_HIDEMATCHCASE|FR_ENABLEHOOK;

		g_hwndFindDialog = FindText(&g_fr);
	}
	else if( ID_EDIT_FIND_NEXT == CmdId || ID_EDIT_FIND_PREVIOUS == CmdId )
	{
		if( g_pszFindText && *g_pszFindText != 0 )
		{
			if( g_iFindAction == 0 )
			{
				SendMessage(hwndView,FSM_FINDITEM,FIND_SEARCH,(LPARAM)&g_fr);
				g_iFindAction = 1;
			}
			else
			{
				g_fr.Flags &= ~FR_DOWN;
				g_fr.Flags |= (ID_EDIT_FIND_NEXT == CmdId) ? FR_DOWN : 0;
				SendMessage(hwndView,FSM_FINDITEM,FIND_SEARCH_NEXT,(LPARAM)&g_fr);
			}
		}
	}
}

void OnFindDialogEvent(LPFINDREPLACE lpfr)
{
	// If the FR_DIALOGTERM flag is set, 
	// invalidate the handle that identifies the dialog box. 
	if( lpfr->Flags & FR_DIALOGTERM )
	{ 
		HWND hwndView = _GetActiveView();
		SendMessage(hwndView,FSM_FINDITEM,FIND_CLOSEDIALOG,(LPARAM)lpfr);
		g_hwndFindDialog = NULL; 
		return ;
	} 

	// If the FR_FINDNEXT flag is set, 
	// call the application-defined search routine
	// to search for the requested string. 
	if( lpfr->Flags & FR_FINDNEXT )
	{
		HWND hwndView = _GetActiveView();
		if( g_iFindAction == 0 )
		{
			SendMessage(hwndView,FSM_FINDITEM,FIND_SEARCH,(LPARAM)&g_fr);
			g_iFindAction = 1;
		}
		else
		{
			SendMessage(hwndView,FSM_FINDITEM,FIND_SEARCH_NEXT,(LPARAM)&g_fr);
		}
	}
}

BOOL IsFindEventMessage(UINT message,LPARAM lParam)
{
	if( g_uFindMessage == message )
	{
		OnFindDialogEvent((LPFINDREPLACE)lParam);
		return TRUE;
	}
	return FALSE;
}

BOOL IsFindDialogMessage(MSG *pmsg)
{
	if( g_hwndFindDialog && IsDialogMessage(g_hwndFindDialog,pmsg) )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL HasFindText()
{
	return (g_pszFindText != NULL) && (*g_pszFindText != L'\0');
}
