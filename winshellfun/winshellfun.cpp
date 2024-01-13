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

#define PM_OPENMDICHILD (WM_APP+321)

static WCHAR *pszTitle       = L"WinShellFun";
static WCHAR *pszWindowClass = L"WinShellFun_WndClass";

static HINSTANCE hInst = NULL;

static HWND hWndMain = NULL;
static HWND hWndMDIClient = NULL;
static HWND hWndActiveMDIChild = NULL;
static HMENU hMainMenu = NULL;
static HMENU hMdiMenu = NULL;
static PWSTR g_pszIniFilePath = NULL;
static HWND g_hwndRecycleBin = NULL;
static BOOL g_bMaximizeStartup = FALSE;
static UINT g_nStartupView[2] = {0};
static int g_nStartupViewCount = 0;

HINSTANCE _GetResourceInstance()
{
	return hInst;
}

HWND _GetMainWnd()
{
	return hWndMain;
}

HWND _GetActiveView(HWND hwndMDIChildFrame)
{
	if( hwndMDIChildFrame == NULL )
		hwndMDIChildFrame = hWndActiveMDIChild;

	MDICHILDWNDDATA *pd = (MDICHILDWNDDATA *)GetWindowLongPtr(hwndMDIChildFrame,GWLP_USERDATA);
	if( pd )
	{
		return pd->hWndView;
	}
	return NULL;
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
	HWND hwndChildFrame = NULL;

	if( ViewType == VIEW_RECYCLEBINFOLDER )
	{
		hwndChildFrame = g_hwndRecycleBin;
	}

	if( hwndChildFrame != NULL )
	{
		BOOL bMaximized;
		MDIGetActive(hWndMDIClient,&bMaximized);

		if( bMaximized )
		{
			SetRedraw(hWndMDIClient,FALSE);
		}

		SendMessage(hWndMDIClient,WM_MDIACTIVATE,(WPARAM)hwndChildFrame,0);

		if( bMaximized )
		{
			SetRedraw(hWndMDIClient,TRUE);
			RedrawWindow(hwndChildFrame,0,0,RDW_UPDATENOW|RDW_INVALIDATE|RDW_ALLCHILDREN);
		}

		return hwndChildFrame;
	}

	PCWSTR pszTitle=NULL;
	MDICREATEPARAM mcp;
	if( pszPath )
		mcp.pszInitialPath = pszPath;
	else
		mcp.pszInitialPath = NULL;

	if( ViewType == VIEW_RECYCLEBINFOLDER )
	{
		SHSTOCKICONINFO shsi = {0};
		shsi.cbSize = sizeof(shsi);
		SHGetStockIconInfo(SIID_RECYCLER,SHGSI_ICON|SHGSI_SMALLICON|SHGSI_SHELLICONSIZE,&shsi);
		mcp.hIcon = shsi.hIcon;
		pszTitle = L"Recycle Bin Folder";
	}

	MDICHILDFRAMEINIT mcfi = {0};
	mcfi.pt.x = mcfi.pt.y = CW_USEDEFAULT;
	mcfi.size.cx = mcfi.size.cy = CW_USEDEFAULT;

	HWND hwndMDIChild = CreateMDIChildFrame(hWndMDIClient,pszTitle,&mcfi,(LPARAM)&mcp,bMaximize);

	if( hwndMDIChild )
	{
		if( hMdiMenu == NULL )
			hMdiMenu = LoadMenu(_GetResourceInstance(),MAKEINTRESOURCE(IDR_MDICHILDFRAME));

		SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMdiMenu,0);
		DrawMenuBar(hWnd);

		MDICHILDWNDDATA *pd = (MDICHILDWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
		{
			pd->wndId = ViewType;

			if( ViewType == VIEW_FOLDERCONTENTSBROWSER )
			{
				// Folder Contents Viewer
				pd->hWndView = CreateShellFunWindow(hwndMDIChild,ViewType);
				mcp.pszInitialPath = g_pszIniFilePath;
				SendMessage(pd->hWndView,WM_CONTROL_MESSAGE,UI_INIT_VIEW,(LPARAM)mcp.pszInitialPath);
			}
			else if( ViewType == VIEW_RECYCLEBINFOLDER )
			{
				// Recycle Bin Viewer
				pd->hWndView = CreateShellFunWindow(hwndMDIChild,ViewType);
				SendMessage(pd->hWndView,WM_CONTROL_MESSAGE,UI_INIT_VIEW,(LPARAM)0);
				g_hwndRecycleBin = hwndMDIChild; // save for singleton
			}

			// Initialize Layout
			RECT rc;
			GetClientRect(hwndMDIChild,&rc);
			SetWindowPos(pd->hWndView,NULL,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE|SWP_HIDEWINDOW);
			SendMessage(pd->hWndView,WM_CONTROL_MESSAGE,UI_INIT_LAYOUT,(LPARAM)&rc);

			// Show MDI child frame window
			ShowWindow(pd->hWndView,SW_SHOW);

			// Refresh MDI menu
			SendMessage(hWndMDIClient,WM_MDIREFRESHMENU,0,0);
			DrawMenuBar(hWnd);
		}
	}

	return hwndMDIChild;
}

//----------------------------------------------------------------------------
//
//  RegisterMDIFrameClass()
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

	ZeroMemory(g_nStartupView,sizeof(g_nStartupView));
	g_nStartupViewCount = 0;

	if( __argc > 1 )
	{
		for(int i = 1; i < __argc; i++)
		{
			if( __wargv[i][0] == L'/' || __wargv[i][0] == L'-' )
			{
				if( _wcsicmp(&__wargv[i][1],L"i") == 0 && ((i + 1) < __argc) )
				{
					// Uses the first found option and else options ignored.
					if( g_pszIniFilePath == NULL )
						g_pszIniFilePath = _MemAllocString(__wargv[++i]);
				}
				else if( _wcsicmp(&__wargv[i][1],L"mx") == 0 )
				{
					g_bMaximizeStartup = TRUE;
				}
				else if( _wcsicmp(&__wargv[i][1],L"fb") == 0 )
				{
					if( g_nStartupViewCount < _countof(g_nStartupView) )
						g_nStartupView[g_nStartupViewCount++] = VIEW_FOLDERCONTENTSBROWSER;
				}
				else if( _wcsicmp(&__wargv[i][1],L"rb") == 0 )
				{
					if( g_nStartupViewCount < _countof(g_nStartupView) )
						g_nStartupView[g_nStartupViewCount++] = VIEW_RECYCLEBINFOLDER;
				}
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

	// Open MDI child frame when start up
	if( g_nStartupViewCount == 0 )
	{
		SendMessage(hWnd,PM_OPENMDICHILD,MAKEWPARAM(VIEW_FOLDERCONTENTSBROWSER,g_bMaximizeStartup),0);
	}
	else
	{
		for(int i = 0; i < g_nStartupViewCount; i++)
			SendMessage(hWnd,PM_OPENMDICHILD,MAKEWPARAM(g_nStartupView[i],g_bMaximizeStartup),0);
	}

	SetWindowPos(MDIGetActive(hWndMDIClient),NULL,0,0,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_DRAWFRAME);

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
	if( hMainMenu )
	{
		DestroyMenu(hMainMenu);
		hMainMenu = NULL;
	}
	if( hMdiMenu )
	{
		DestroyMenu(hMdiMenu);
		hMdiMenu = NULL;
	}

	_SafeMemFree(g_pszIniFilePath);
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
	switch( CmdId )
	{
		case ID_EDIT_FIND_NEXT:
		case ID_EDIT_FIND_PREVIOUS:
			if( !HasFindText() )
				return UPDUI_DISABLED;
	}

	HWND hwndMDIChild = MDIGetActive(hWndMDIClient);
	if( hwndMDIChild )
	{
		MDICHILDWNDDATA *pd = (MDICHILDWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
		UINT State = 0;
		if( SendMessage(pd->hWndView,WM_QUERY_CMDSTATE,MAKEWPARAM(CmdId,0),(LPARAM)&State) )
		{
			return State;
		}
	}

	switch( CmdId )
	{
		case ID_SHELLFOLDERCONTENTSBROWSER:
		case ID_SHELLFOLDERS:
		case ID_RECYCLEBINFOLDER:
		case ID_ABOUT:
		case ID_EXIT:
			return UPDUI_ENABLED;
		case ID_FILE_CLOSE:
		case ID_WINDOW_CASCADE:
		case ID_WINDOW_TILE_HORZ:
		case ID_WINDOW_TILE_VERT:
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
		case WM_NCCREATE:
		{
			hWndMain = hWnd;
			break;
		}
		case WM_CREATE: 
		{
			hWndMDIClient = CreateMDIClient(hWnd);
			InitFind();
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
				case ID_SHELLFOLDERCONTENTSBROWSER:
					SendMessage(hWnd,PM_OPENMDICHILD,VIEW_FOLDERCONTENTSBROWSER,0);
					break;
				case ID_RECYCLEBINFOLDER:
					SendMessage(hWnd,PM_OPENMDICHILD,VIEW_RECYCLEBINFOLDER,0);
					break;
				case ID_FILE_CLOSE:
					SendMessage(hWndMDIClient, WM_MDIDESTROY, (WPARAM)MDIGetActive(hWndMDIClient), 0L); 
					break;
				case ID_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case ID_EXIT:
					DestroyWindow(hWnd);
					break;
				case ID_EDIT_FIND:
				case ID_EDIT_FIND_NEXT:
				case ID_EDIT_FIND_PREVIOUS:
					OnEditFind(hWnd,wmId);
					break;
				case ID_WINDOW_CASCADE:
					SendMessage(hWndMDIClient,WM_MDICASCADE,MDITILE_SKIPDISABLED|MDITILE_ZORDER,0);
					break;
				case ID_WINDOW_TILE_HORZ:
					SendMessage(hWndMDIClient,WM_MDITILE,MDITILE_SKIPDISABLED|MDITILE_HORIZONTAL,0);
					break;
				case ID_WINDOW_TILE_VERT:
					SendMessage(hWndMDIClient,WM_MDITILE,MDITILE_SKIPDISABLED|MDITILE_VERTICAL,0);
					break;
				default:
				{
					HWND hwndMDIChild = MDIGetActive(hWndMDIClient);
					if( hwndMDIChild )
					{
						MDICHILDWNDDATA *pd = (MDICHILDWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
						SendMessage(pd->hWndView,WM_COMMAND,wParam,lParam);
					}
					break;
				}
			}
			break;
		}
		case WM_INITMENUPOPUP:
		{
			HMENU hMenu = (HMENU)wParam;
			INT RelativePosition = (INT)LOWORD(lParam);
			INT WindowMenu = HIWORD(lParam);

			// ignores system menu
			if( GetMenuPosFromID(hMenu,SC_SIZE) != -1 )
				break;

			if( !WindowMenu )
				UpdateUI_MenuItem((HMENU)wParam,&QueryCmdState,0);
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
		case WM_MDI_CHILDFRAME_CLOSE:
		{
			HWND hwndChildFrame = (HWND)wParam;
			if( hwndChildFrame )
			{
				MDICHILDWNDDATA *pd = (MDICHILDWNDDATA *)GetWindowLongPtr(hwndChildFrame,GWLP_USERDATA);
				if( pd->wndId == VIEW_RECYCLEBINFOLDER )
				{
					g_hwndRecycleBin = NULL;
				}
			}
			return 0;
		}
		case PM_OPENMDICHILD:
		{
			HWND hMDIChild = OpenMDIChild(hWnd,NULL,LOWORD(wParam),HIWORD(wParam));
			MDICHILDWNDDATA *pd = (MDICHILDWNDDATA *)GetWindowLongPtr(hMDIChild,GWLP_USERDATA);
			SetFocus(pd->hWndView);
			break;
		}
		default:
		{
			if( IsFindEventMessage(message,lParam) )
			{
				return 0;
			}
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

	if( InitInstance(hInstance, nCmdShow) == NULL )
	{
		CoUninitialize();
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));

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
			if( hWndActiveMDIChild && IsFindDialogMessage(&msg) )
			{
				continue;
			}

			if( (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && (msg.wParam == VK_RETURN) )
			{
				// prevent ring beep when press enter key on tree-view.
				DispatchMessage(&msg);
				continue;
			}

			if (!TranslateMDISysAccel(hWndMDIClient, &msg) && 
				!TranslateAccelerator(hWndMain, hAccelTable, &msg))
			{ 
				if( hWndActiveMDIChild != NULL )
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
