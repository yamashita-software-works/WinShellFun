//****************************************************************************
//
//  recyclebinview.cpp
//
//  Implements the recycle bin folder viewer base window.
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
#include "shellfun.h"
#include "recyclebinview.h"
#include "pagewbdbase.h"
#include "page_recyclebinlist.h"

class CRecycleBinFolderView : 
	public CBaseWindow,
	public IViewBaseWindow
{
	CPageWndBase *m_pBase;
	CPageWndBase *m_pViewTable[VIEW_TYPE_MAX_COUNT];

public:
	CRecycleBinFolderView()
	{
		m_hWnd = NULL;
		m_pBase = NULL;
		memset(m_pViewTable,0,sizeof(m_pViewTable));
	}

	virtual ~CRecycleBinFolderView()
	{
	}

	VOID UpdateLayout(int cx=0,int cy=0)
	{
		if( cx == 0 && cy == 0 )
		{
			RECT rc;
			GetClientRect(m_hWnd,&rc);
			cx = rc.right - rc.left;
			cy = rc.bottom - rc.top;
		}

		if( m_pBase && m_pBase->GetHwnd() )
			SetWindowPos(m_pBase->GetHwnd(),NULL,0,0,cx,cy,SWP_NOZORDER|SWP_NOMOVE|SWP_FRAMECHANGED);
	}

	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM lParam)
	{
		SetWindowText(hWnd,L"CRecycleBinFolderView");
		return 0;
	}

	LRESULT OnDestroy(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		return 0;
	}

	LRESULT OnSize(HWND,UINT,WPARAM,LPARAM lParam)
	{
		UpdateLayout(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		return 0;
	}

	LRESULT OnSetFocus(HWND,UINT,WPARAM,LPARAM lParam)
	{
		if( m_pBase )
			SetFocus(m_pBase->GetHwnd());
		return 0;
	}

	LRESULT OnFindItem(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		if( m_pBase )
			return SendMessage(m_pBase->GetHwnd(),uMsg,wParam,lParam);
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
		pnmhdr->hwndFrom = m_hWnd;
		pnmhdr->idFrom = GetWindowLong(m_hWnd,GWL_ID);
		SendMessage(GetParent(m_hWnd),WM_NOTIFY,0,(LPARAM)pnmhdr);
		return 0;
	}

	template <class T>
	CPageWndBase *GetOrAllocWndObjct(int wndId)
	{
		CPageWndBase *pobj;
		if( m_pViewTable[ wndId ] == NULL )
		{
			pobj = (CPageWndBase*)new T ;
			m_pViewTable[ wndId ] = pobj;
			pobj->Create(m_hWnd,wndId,0,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
		}
		else
		{
			pobj = m_pViewTable[ wndId ];
		}
		return pobj;
	}

	INT SelectPage(int nView)
	{
		CPageWndBase* pNew = NULL;

		switch( nView )
		{
			case VIEW_RECYCLEBINFOLDER:
			{
				pNew = GetOrAllocWndObjct<CRecycleBinListViewPage>(VIEW_RECYCLEBINFOLDER);
				break;
			}
			default:
				return -1;
		}

		if( m_pBase == pNew )
		{
			return nView;
		}
	
		CPageWndBase* pPrev = m_pBase;

		m_pBase = pNew;
		UpdateLayout();

		EnableWindow(pNew->m_hWnd,TRUE);
		ShowWindow(pNew->m_hWnd,SW_SHOWNA);

		if( pPrev )
		{
			ShowWindow(pPrev->m_hWnd,SW_HIDE);
			EnableWindow(pPrev->m_hWnd,FALSE);
		}

		return nView;
	}

	HRESULT UpdateData(SELECT_ITEM *pFile)
	{
		ASSERT( m_pBase != NULL );

		return m_pBase->UpdateData(pFile);
	}

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
	    {
			case WM_NOTIFY:
				return OnNotify(hWnd,uMsg,wParam,lParam);
			case WM_SIZE:
				return OnSize(hWnd,uMsg,wParam,lParam);
			case WM_SETFOCUS:
				return OnSetFocus(hWnd,uMsg,wParam,lParam);
		    case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
			case WM_DESTROY:
				return OnDestroy(hWnd,uMsg,wParam,lParam);
			case WM_CONTROL_MESSAGE:
				return SendMessage(GetParent(m_hWnd),WM_CONTROL_MESSAGE,wParam,lParam);
			case FSM_FINDITEM:
				return OnFindItem(hWnd,uMsg,wParam,lParam);
		}
		return CBaseWindow::WndProc(hWnd,uMsg,wParam,lParam);
	}

	//
	// IFileInfoBaseWindow
	//
public:
	HWND GetHWND() const
	{
		return m_hWnd;
	}

	HRESULT Create(HWND hWnd,HWND *phWndFileList=NULL)
	{
		HWND hwnd = CBaseWindow::Create(hWnd,0,0,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
		if( phWndFileList )
			*phWndFileList = hwnd;
		return S_OK;
	}

	HRESULT Destroy() { return E_NOTIMPL; }
	HRESULT InitData() { return E_NOTIMPL; }
	HRESULT InitLayout(const RECT *prc) { return E_NOTIMPL; }

	HRESULT SelectView(SELECT_ITEM *SelItem)
	{
		switch( SelItem->ViewType )
		{
			case VIEW_RECYCLEBINFOLDER:
				SelectPage( VIEW_RECYCLEBINFOLDER );
				break;
			default:
				ASSERT(FALSE);
				return E_FAIL;
		}

		UpdateData(SelItem);

		UpdateLayout();

		return S_OK;
	}

	virtual HRESULT QueryCmdState(UINT CmdId,UINT *State)
	{
		ASSERT(m_pBase != NULL);
		return m_pBase->QueryCmdState(CmdId,State);
	}

	virtual HRESULT InvokeCommand(UINT CmdId)
	{
		ASSERT(m_pBase != NULL);
		return m_pBase->InvokeCommand(CmdId);
	}

	virtual HRESULT PreTranslateMessage(MSG *pMsg)
	{
		ASSERT(m_pBase != NULL);
		return m_pBase->PreTranslateMessage(pMsg);
	}

	virtual HRESULT InitData(PVOID,LPARAM) { return E_NOTIMPL;}
	virtual HRESULT GetState(int,ULONG *pul) { return E_NOTIMPL;}
	virtual HRESULT SetViewData(SELECT_ITEM *Sel) { return E_NOTIMPL;}
};

//////////////////////////////////////////////////////////////////////////////

//
//  C style functions
//

//----------------------------------------------------------------------------
//
//  CreateRecycleBinWindowObject()
//
//----------------------------------------------------------------------------
HRESULT CreateRecycleBinWindowObject(HINSTANCE hInstance,IViewBaseWindow **pObject)
{
	CRecycleBinFolderView *pWnd = new CRecycleBinFolderView;

	CRecycleBinFolderView::RegisterClass(hInstance);

	*pObject = static_cast<IViewBaseWindow *>(pWnd);

	return S_OK;
}
