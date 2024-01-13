//****************************************************************************
//
//  recyclebinwindow.cpp
//
//  Implements the recycle bin folder viewer window.
//
//  Auther: YAMASHITA Katsuhiro
//
//  Create: 2024.01.05
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
#include "recyclebinwindow.h"
#include "recyclebinview.h"

class CRecycleBinFolderWindow : public CBaseWindow
{
public:
	IViewBaseWindow *m_pView;

	HWND m_hWndCtrlFocus;

	CRecycleBinFolderWindow()
	{
		m_hWnd = NULL;
		m_pView = NULL;
		m_hWndCtrlFocus = NULL;
	}

	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		m_hWnd = hWnd;

		CreateRecycleBinWindowObject(GETINSTANCE(m_hWnd),&m_pView);
		m_pView->Create(hWnd);

		m_hWndCtrlFocus = m_pView->GetHWND();

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
		if( m_hWndCtrlFocus == m_pView->GetHWND() )
			m_pView->InvokeCommand(LOWORD(wParam));
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
			case UI_INIT_VIEW:
			{
				SELECT_ITEM sel = {0};
				sel.ViewType = VIEW_RECYCLEBINFOLDER;
				m_pView->SelectView(&sel);
				break;
			}
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

		DeferWindowPos(hdwp,m_pView->GetHWND(),NULL,0,0,cx,cy,SWP_NOZORDER);

		EndDeferWindowPos(hdwp);
	}

	VOID OnUpdateView(SELECT_ITEM* pSelItem)
	{
		HWND hwndMDIChildFrame = GetParent(m_hWnd);

		m_pView->SelectView(pSelItem);
	}

	VOID InitData(PCWSTR pszDirectoryPath)
	{
	}

	VOID InitLayout(const RECT *prcDesktopWorkArea)
	{
		m_pView->InitLayout(NULL);
	}
};

//////////////////////////////////////////////////////////////////////////////

HWND CreateRecycleBinWindow(HWND hWndParent)
{
	CRecycleBinFolderWindow::RegisterClass(GETINSTANCE(hWndParent));

	CRecycleBinFolderWindow *pWnd = new CRecycleBinFolderWindow;

	return pWnd->Create(hWndParent,0,L"CRecycleBinFolderWindow",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
}
