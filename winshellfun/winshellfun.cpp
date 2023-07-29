//***************************************************************************
//*                                                                         *
//*  winshellfun.cpp                                                        *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Create: 2023.06.26 Create                                              *
//*                                                                         *
//***************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "winshellfun.h"
#include "mdichild.h"

static WCHAR *pszTitle       = L"WinShellFun";
static WCHAR *pszWindowClass = L"WinShellFun_WndClass";

static HINSTANCE hInst = NULL;

HWND hWndMain = NULL;
HWND hWndMDIClient = NULL;
static HWND hWndActiveMDIChild = NULL;
static HMENU hMainMenu = NULL;   // todo: DestoryMenu
static HMENU hMdiMenu = NULL;    // todo: DestoryMenu
static PWSTR g_pszIniFilePath = NULL;

#define PM_OPENMDICHILD (WM_APP+321)

HINSTANCE _GetResourceInstance()
{
	return hInst;
}

HWND _GetMainWnd()
{
	return hWndMain;
}

BOOL _GetIniFilePath(HMODULE hModule,PWSTR pszIniPath,int cchIniPath)
{
	GetModuleFileName(hModule,pszIniPath,cchIniPath);
	PathRemoveExtension(pszIniPath);
	PathAddExtension(pszIniPath,L".ini");

	if( !PathFileExists(pszIniPath) )
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////

#include "..\build.h"

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
		{
			_CenterWindow(hDlg,GetActiveWindow());
			WCHAR *psz = new WCHAR[64];
			StringCchPrintf(psz,128,L"%u.%u.%u.%u Preview",MEJOR_VERSION,MINOR_VERSION,BUILD_NUMBER,PATCH_NUMBER);
			SetDlgItemText(hDlg,IDC_TEXT,psz);
			delete[] psz;
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

//----------------------------------------------------------------------------
//
//  OpenMDIChild()
//
//  PURPOSE: Open MDI child window.
//
//----------------------------------------------------------------------------
HWND OpenMDIChild(HWND hWnd,PCWSTR pszPath,int ViewType,BOOL bMaximize)
{
	MDICREATEPARAM mcp;
	if( pszPath )
		mcp.pszInitialPath = pszPath;
	else
		mcp.pszInitialPath = NULL;

	HWND hwndMDIChild = CreateMDIChildFrame(hWndMDIClient,NULL,(LPARAM)&mcp,bMaximize);

	if( hwndMDIChild )
	{
		if( hMdiMenu == NULL )
			hMdiMenu = LoadMenu(_GetResourceInstance(),MAKEINTRESOURCE(IDR_MDICHILDFRAME));

		SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMdiMenu,0);
		DrawMenuBar(hWnd);

		MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
		{
			pd->hWndView = ShellFolderCreateWindow(hwndMDIChild);

			mcp.pszInitialPath = g_pszIniFilePath;

			ShellFolderInitData(pd->hWndView,mcp.pszInitialPath,NULL);

			RECT rc;
			GetClientRect(hwndMDIChild,&rc);
			SetWindowPos(pd->hWndView,NULL,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER);
		}
	}

	return hwndMDIChild;
}

//----------------------------------------------------------------------------
//
//  FUNCTION: RegisterMDIFrameClass()
//
//  PURPOSE: Register main frame window class.
//
//----------------------------------------------------------------------------
ATOM RegisterMDIFrameClass(HINSTANCE hInstance)
{
	extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	WNDCLASSEX wcex = {0};

	wcex.cbSize        = sizeof(wcex);
	wcex.style         = CS_HREDRAW|CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra	   = 0;
	wcex.cbWndExtra	   = 0;
	wcex.hInstance	   = hInstance;
	wcex.hIcon		   = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wcex.hCursor	   = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = pszWindowClass;
	wcex.hIconSm	   = (HICON)LoadImage(wcex.hInstance, MAKEINTRESOURCE(IDI_MAIN),IMAGE_ICON,16,16,LR_DEFAULTSIZE);

	return RegisterClassEx(&wcex);
}

//----------------------------------------------------------------------------
//
//  ParseCommandLine()
//
//  PURPOSE:
//
//----------------------------------------------------------------------------
VOID ParseCommandLine()
{
	extern int __argc;
	extern wchar_t **__wargv;

	if( __argc > 1 )
	{
		for(int i = 1; i < __argc; i++)
		{
			if( _wcsicmp(__wargv[i],L"-i") == 0 && ((i + 1) < __argc) )
			{
				g_pszIniFilePath = _MemAllocString(__wargv[++i]);
			}
		}
	}
}

//----------------------------------------------------------------------------
//
//  InitInstance()
//
//  PURPOSE: Initialize this instance.
//
//----------------------------------------------------------------------------
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance;

	ParseCommandLine();

	WCHAR szIniFilePath[MAX_PATH];
	if( g_pszIniFilePath == NULL && _GetIniFilePath(hInstance,szIniFilePath,MAX_PATH) )
	{
		g_pszIniFilePath = _MemAllocString(szIniFilePath);
	}

	if( g_pszIniFilePath == NULL || !PathFileExists(g_pszIniFilePath) )
	{
		MessageBox(NULL,L"Ini file not found.",pszTitle,MB_OK|MB_ICONSTOP);
		return NULL;
	}

	hWnd = CreateWindow(pszWindowClass, pszTitle, WS_OVERLAPPEDWINDOW,
				  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	// Load MainFrame menu
	hMainMenu = LoadMenu(_GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetMenu(hWnd,hMainMenu);

	// Show frame window
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//----------------------------------------------------------------------------
//
//  ExitInstance()
//
//  PURPOSE: Exit this instance.
//
//----------------------------------------------------------------------------
VOID ExitInstance()
{
	;
}

//----------------------------------------------------------------------------
//
//  QueryCmdState()
//
//  PURPOSE: Query command status.
//
//----------------------------------------------------------------------------
INT CALLBACK QueryCmdState(UINT CmdId,PVOID,LPARAM)
{
	HWND hwndMDIChild = MDIGetActiveChild(hWndMDIClient);
	if( hwndMDIChild )
	{
		MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
		UINT State = 0;
		if( SendMessage(pd->hWndView,WM_QUERY_CMDSTATE,MAKEWPARAM(CmdId,0),(LPARAM)&State) )
		{
			return State;
		}
	}

	switch( CmdId )
	{
		case ID_FILE_NEW:
		case ID_ABOUT:
		case ID_EXIT:
			return UPDUI_ENABLED;
		case ID_FILE_CLOSE:
			if( hwndMDIChild != NULL )
				return UPDUI_ENABLED;
			break;
	}

	return UPDUI_DISABLED;
}

//----------------------------------------------------------------------------
//
//  WndProc()
//
//  PURPOSE: Main frame window procedure.
//
//----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE: 
		{
			hWndMDIClient = CreateMDIClient(hWnd);
			break; 
		} 
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_SIZE:
		{
			// Resizes the MDI client window to fit in the new frame window's client area. 
			// If the frame window procedure sizes the MDI client window to a different size, 
			// it should not pass the message to the DefWindowProc function.
			int cx = GET_X_LPARAM(lParam);
			int cy = GET_Y_LPARAM(lParam);
			SetWindowPos(hWndMDIClient,NULL,0,0,cx,cy,SWP_NOZORDER);
			break;
		}
		case WM_COMMAND:
		{
			int wmId, wmEvent;
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch (wmId)
			{
				case ID_FILE_NEW:
					SendMessage(hWnd,PM_OPENMDICHILD,0,0);
					break;
                case ID_FILE_CLOSE:
					SendMessage(hWndMDIClient, WM_MDIDESTROY, (WPARAM)MDIGetActiveChild(hWndMDIClient), 0L); 
					break;
				case ID_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case ID_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
				{
					HWND hwndMDIChild = MDIGetActiveChild(hWndMDIClient);
					if( hwndMDIChild )
					{
						MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
						SendMessage(pd->hWndView,WM_COMMAND,wParam,lParam);
					}
					break;
				}
			}
			break;
		}
		case WM_MDIACTIVATE:
		{
			hWndActiveMDIChild = (HWND)lParam;
			if( hWndActiveMDIChild == NULL )
			{
				SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMainMenu,0);
				DrawMenuBar(hWnd);
			}
			else
			{
				SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMdiMenu,0);
				DrawMenuBar(hWnd);
			}
			break;
		}
		case WM_INITMENUPOPUP:
		{
			INT RelativePosition = (INT)LOWORD(lParam);
			INT WindowMenu = HIWORD(lParam);
			if( !WindowMenu )
				UpdateUI_MenuItem((HMENU)wParam,&QueryCmdState,0);
			break;
		}
		case PM_OPENMDICHILD:
		{
			HWND h = OpenMDIChild(hWnd,NULL,LOWORD(wParam),HIWORD(wParam));
			MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(h,GWLP_USERDATA);
			SetFocus(pd->hWndView);
			break;
		}
	}

	return DefFrameProc(hWnd, hWndMDIClient, message, wParam, lParam);
}

//----------------------------------------------------------------------------
//
//  WinMain()
//
//  PURPOSE: Main procedure.
//
//----------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	_wsetlocale(LC_ALL,L"");

	RegisterMDIFrameClass(hInstance);
	RegisterMDIChildFrameClass(hInstance);

	CoInitialize(NULL);

	if( (hWndMain = InitInstance (hInstance, nCmdShow)) == NULL )
	{
		CoUninitialize();
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));

	PostMessage(hWndMain,PM_OPENMDICHILD,MAKEWPARAM(0,TRUE),0);

	MSG msg;
	INT ret;
	while ((ret = GetMessage(&msg, (HWND) NULL, 0, 0)) != 0)
	{
		if( ret == -1 )
	    {
		    break;// handle the error and possibly exit
	    }
		else 
	    { 
			if( (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && (msg.wParam == VK_RETURN) )
			{
				// prevent ring beep when press enter key on tree-view.
				DispatchMessage(&msg);
				continue;
			}

		    if (!TranslateMDISysAccel(hWndMDIClient, &msg) && 
			    !TranslateAccelerator(hWndMain, hAccelTable, &msg))
	        { 
				if( hWndActiveMDIChild )
				{
					if( IsDialogMessage(hWndActiveMDIChild,&msg) )
						continue;
				}
		        TranslateMessage(&msg); 
			    DispatchMessage(&msg); 
			} 
		} 
	}

	ExitInstance();

	CoUninitialize();

	return (int) msg.wParam;
}
