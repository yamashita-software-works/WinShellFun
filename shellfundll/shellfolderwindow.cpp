//****************************************************************************
//
//  shellfolderwindow.cpp
//
//  Implements the shell folder window.
//
//  Auther: YAMASHITA Katsuhiro
//
//  Create: 2023.06.26
//
//****************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "resource.h"
#include "miscshell.h"
#include "shellfun.h"
#include "shellfolderwindow.h"
#include "shellfoldertree.h"
#include "shellfolderview.h"

class CShellFolderWindow : public CBaseWindow
{
public:
	HWND m_hWndTreeBase;

	IViewBaseWindow *m_pView;

	int m_cxSplitPos;

	HWND m_hWndCtrlFocus;

	CShellFolderWindow()
	{
		m_hWnd = NULL;
		m_hWndTreeBase = NULL;
		m_pView = NULL;
		m_cxSplitPos = 280;
		m_hWndCtrlFocus = NULL;
	}

	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		m_hWnd = hWnd;

		// Create item tree view window
		ShellFolderTree_CreateWindow(hWnd,&m_hWndTreeBase);

		// Create information view host frame window
		CreateShellFolderViewObject(GETINSTANCE(m_hWnd),&m_pView);
		m_pView->Create(hWnd);

		m_hWndCtrlFocus = m_hWndTreeBase;

		return 0;
	}

	LRESULT OnDestroy(HWND,UINT,WPARAM,LPARAM)
	{
		m_pView->Destroy();
		return 0;
	}

	LRESULT OnSize(HWND,UINT,WPARAM,LPARAM lParam)
	{
		int cx = GET_X_LPARAM(lParam);
		int cy = GET_Y_LPARAM(lParam);
		UpdateLayout(cx,cy);
		return 0;
	}

	LRESULT OnSetFocus(HWND,UINT,WPARAM,LPARAM lParam)
	{
		SetFocus(m_hWndCtrlFocus);
		return 0;
	}

	LRESULT OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		NMHDR *pnmhdr = (NMHDR *)lParam;
		switch( pnmhdr->code )
		{
			case NM_SETFOCUS:
				return OnNmSetFocus(pnmhdr);
		}
		return 0;
	}

	LRESULT OnNmSetFocus(NMHDR *pnmhdr)
	{
		m_hWndCtrlFocus = pnmhdr->hwndFrom;
		return 0;
	}

	LRESULT OnCommand(HWND,UINT,WPARAM wParam,LPARAM)
	{
		switch( LOWORD(wParam) )
		{
			case ID_UP_DIR:
				ShellFolderTree_SelectFolder(m_hWndTreeBase,L"..",0);
				break;
			default:
				if( m_hWndCtrlFocus == m_pView->GetHWND() )
					m_pView->InvokeCommand(LOWORD(wParam));
				break;
		}
		return 0;
	}

	LRESULT OnQueryCmdState(HWND,UINT,WPARAM wParam,LPARAM lParam)
	{
		if( m_hWndCtrlFocus == m_pView->GetHWND() )
		{
			ASSERT( lParam != NULL );
			if( lParam )
			{
				if( m_pView->QueryCmdState((UINT)LOWORD(wParam),(UINT*)lParam) == S_OK )
					return TRUE;
			}
		}
		return 0;
	}

	LRESULT OnControlMessage(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch( LOWORD(wParam) )
		{
			case UI_SELECT_FOLDER:
				OnUpdateView( (SELECT_ITEM*)lParam );
				break;
			case UI_INIT_VIEW:
				OnInitView( (PCWSTR)lParam );
				break;
		}
		return 0;
	}

	LRESULT OnNotifyMessage(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	}

	LRESULT OnFindItem(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if( m_pView )
		{
			SendMessage(m_pView->GetHWND(),uMsg,wParam,lParam);
		}
		return 0;
	}

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
	    {
			case WM_NOTIFY:
				return OnNotify(hWnd,uMsg,wParam,lParam);
		    case WM_SIZE:
				return OnSize(hWnd,uMsg,wParam,lParam);
			case WM_COMMAND:
				return OnCommand(hWnd,uMsg,wParam,lParam);
			case WM_SETFOCUS:
				return OnSetFocus(hWnd,uMsg,wParam,lParam);
			case WM_DESTROY:
				return OnDestroy(hWnd,uMsg,wParam,lParam);
		    case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
			case WM_QUERY_CMDSTATE:
				return OnQueryCmdState(hWnd,uMsg,wParam,lParam);
			case WM_CONTROL_MESSAGE:
				return OnControlMessage(hWnd,uMsg,wParam,lParam);
			case WM_NOTIFY_MESSAGE:
				return OnNotifyMessage(hWnd,uMsg,wParam,lParam);
			case FSM_FINDITEM:
				return OnFindItem(hWnd,uMsg,wParam,lParam);
		}
		return CBaseWindow::WndProc(hWnd,uMsg,wParam,lParam);
	}

	VOID UpdateLayout(int cx,int cy)
	{
		HDWP hdwp = BeginDeferWindowPos(2);

		if( m_hWndTreeBase )
			DeferWindowPos(hdwp,m_hWndTreeBase,NULL,0,0,m_cxSplitPos,cy,SWP_NOZORDER);

		if( m_pView )
			DeferWindowPos(hdwp,m_pView->GetHWND(),NULL,m_cxSplitPos,0,cx-m_cxSplitPos,cy,SWP_NOZORDER);

		EndDeferWindowPos(hdwp);
	}

	VOID OnUpdateView(SELECT_ITEM* pSelItem)
	{
		HWND hwndMDIChildFrame = GetParent(m_hWnd);

		// Update MDI child icon
		HICON hIcon = (HICON)SendMessage(hwndMDIChildFrame,WM_GETICON,ICON_SMALL,0);
		if( hIcon )
			DestroyIcon(hIcon);
		SendMessage(hwndMDIChildFrame,WM_SETICON,ICON_SMALL,(LPARAM)pSelItem->hIcon);
		DrawMenuBar(GetParent(GetParent(hwndMDIChildFrame)));

		// Update title
		SetWindowText(hwndMDIChildFrame,pSelItem->pszName);

		m_pView->SelectView(pSelItem);
	}

	VOID OnInitView(PCWSTR pszIniFilePath)
	{
		SetIniFilePath(pszIniFilePath);
		InitData( L"" );
	}

	VOID InitData(PCWSTR pszDirectoryPath)
	{
		ShellFolderTree_InitData(m_hWndTreeBase,m_hWnd);
		FillTraverseItems(pszDirectoryPath);
	}

	VOID InitLayout(const RECT *prcDesktopWorkArea)
	{
		ShellFolderTree_InitLayout(m_hWndTreeBase,NULL);
		m_pView->InitLayout(NULL);
	}

	VOID FillTraverseItems(PCWSTR pszDirectoryPath)
	{
		ShellFolderTree_FillItems(m_hWndTreeBase,pszDirectoryPath);
	}
};

//////////////////////////////////////////////////////////////////////////////

HWND ShellFolderWindow_CreateWindow(HWND hWndParent)
{
	CShellFolderWindow::RegisterClass(GETINSTANCE(hWndParent));

	CShellFolderWindow *pWnd = new CShellFolderWindow;

	return pWnd->Create(hWndParent,0,L"CShellFolderWindow",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
}
