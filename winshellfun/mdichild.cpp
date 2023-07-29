//
//  2023.03.01 Create
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "resource.h"
#include "mdichild.h"

//
// Following are the messages the application must pass to DefMDIChildProc.
//
// WM_CHILDACTIVATE 
// Performs activation processing when MDI child windows are sized, moved, 
// or displayed. This message must be passed. 
//
// WM_GETMINMAXINFO
// Calculates the size of a maximized MDI child window, based on 
// the current size of the MDI client window. 
//
// WM_MENUCHAR
// Passes the message to the MDI frame window. 
//
// WM_MOVE
// Recalculates MDI client scroll bars, if they are present. 
//
// WM_SETFOCUS
// Activates the child window, if it is not the active MDI child window. 
//
// WM_SIZE 
// Performs operations necessary for changing the size of a window, 
// especially for maximizing or restoring an MDI child window.
// Failing to pass this message to the DefMDIChildProc function 
// produces highly undesirable results. 
//
// WM_SYSCOMMAND 
// Handles window (formerly known as system) menu commands: 
// SC_NEXTWINDOW, SC_PREVWINDOW, SC_MOVE, SC_SIZE, and SC_MAXIMIZE. 
//
LRESULT CALLBACK MDIChildWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch( msg )
	{
		case WM_CREATE:
		{
			CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
			MDICREATESTRUCT *pmdics = (MDICREATESTRUCT *)pcs->lpCreateParams;
			MDICREATEPARAM *pmcp = (MDICREATEPARAM *)pmdics->lParam;

			MDICLIENTWNDDATA *pd = new MDICLIENTWNDDATA;
			ZeroMemory(pd,sizeof(MDICLIENTWNDDATA));
			SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)pd);
			break;
		}
		case WM_NCDESTROY:
		{
			MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hWnd,GWLP_USERDATA);
			if( pd )
			{
				delete pd;
				SetWindowLongPtr(hWnd,GWLP_USERDATA,NULL);
			}
			break;
		}
		case WM_SIZE:
		{
			int cx = GET_X_LPARAM(lParam);
			int cy = GET_Y_LPARAM(lParam);

			MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hWnd,GWLP_USERDATA);
			SetWindowPos(pd->hWndView,NULL,0,0,cx,cy,SWP_NOZORDER);

			break;
		}
		case WM_MDIACTIVATE:
		{
			MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hWnd,GWLP_USERDATA);

			SendMessage(_GetMainWnd(),WM_MDIACTIVATE,wParam,lParam);

			if( (HWND)wParam == hWnd )
			{
				pd->hWndFocus = GetFocus();
			} 
			else if( (HWND)lParam == hWnd )
			{
				if( pd->hWndFocus )
					SetFocus(pd->hWndFocus);
			} 
			break;
		}
		case WM_SETFOCUS:
		{
			MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hWnd,GWLP_USERDATA);

			if( pd->hWndView )
				SetFocus(pd->hWndView);

			return 0;
		}
	}
	return DefMDIChildProc(hWnd,msg,wParam,lParam);
}

ATOM RegisterMDIChildFrameClass(HINSTANCE hInstance) 
{ 
    WNDCLASSEX wcex = {0};
 
	wcex.cbSize        = sizeof(wcex);
    wcex.style         = 0; 
    wcex.cbClsExtra    = 0; 
    wcex.cbWndExtra    = 0; 
    wcex.hInstance     = hInstance; 
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); 
    wcex.lpszMenuName  = NULL;
    wcex.lpfnWndProc   = (WNDPROC)&MDIChildWndProc; 
    wcex.hIconSm       = NULL;
    wcex.lpszMenuName  = (LPCTSTR) NULL; 
    wcex.cbWndExtra    = CBWNDEXTRA; 
    wcex.lpszClassName = MDICHILD_CLASSNAME;

    return RegisterClassEx(&wcex);
}

HWND CreateMDIClient(HWND hWnd)
{
	CLIENTCREATESTRUCT ccs; 

    // Retrieve the handle to the window menu and assign the 
	// first child window identifier. 
 	ccs.hWindowMenu = NULL; // no window menu
	ccs.idFirstChild = -1;

	// Create the MDI client window. 
	HWND hWndMDIClient;
 	hWndMDIClient = CreateWindow( L"mdiclient", NULL, 
									WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN , 
									0, 0, 0, 0, hWnd, (HMENU) 0xCAC, _GetResourceInstance(), (LPSTR) &ccs); 
 
	return hWndMDIClient;
}

HWND CreateMDIChildFrame(HWND hWndMDIClient,PCWSTR pszTitle,LPARAM lParam,BOOL bMaximize)
{ 
    HWND hwnd; 
    MDICREATESTRUCT mcs = {0};
 
	mcs.szTitle = pszTitle;
	mcs.szClass = MDICHILD_CLASSNAME;
	mcs.hOwner  = _GetResourceInstance();
    mcs.x = mcs.cx = CW_USEDEFAULT; 
    mcs.y = mcs.cy = CW_USEDEFAULT; 
    mcs.style = WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_CLIPCHILDREN; 
	mcs.lParam = lParam;

	if( !bMaximize )
	{
		SendMessage(hWndMDIClient,WM_MDIGETACTIVE,0,(LPARAM)&bMaximize);
	}

	if( bMaximize )
		mcs.style |= WS_MAXIMIZE;

#if 0
	else if( bFullSize )
	{
		RECT rc;
		GetClientRect(hWndMDIClient,&rc);
		mcs.x = mcs.y = CW_USEDEFAULT; 
		mcs.cx = rc.right - rc.left;
		mcs.cy = rc.bottom - rc.top;
	}
#endif

	hwnd = (HWND)SendMessage(hWndMDIClient,WM_MDICREATE,0,(LPARAM)&mcs); 
 
    return hwnd; 
} 
