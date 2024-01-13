#pragma once
//***************************************************************************
//*                                                                         *
//*  page_recyclebinlist.h                                                  *
//*                                                                         *
//*  Recycle bin folder viewer list page.                                   *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Create: 2024-01-05                                                     *
//*                                                                         *
//***************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "common.h"
#include "dparray.h"
#include "column.h"
#include "miscshell.h"
#include "recyclebinhelp.h"
#include "simplevalarray.h"

typedef struct _LVRECYCLEBINITEM
{
	UINT Type;
	RECYCLEBIN_FILE_ITEM *pFI;
} LVRECYCLEBINITEM, *PLVRECYCLEBINITEM;

struct CRecycleBinItem : public LVRECYCLEBINITEM
{
	CRecycleBinItem()
	{
		memset(this,0,sizeof(LVRECYCLEBINITEM));
	}
};

//////////////////////////////////////////////////////////////////////////////

class CRecycleBinListViewPage : public CPageWndBase
{
	HWND m_hWndList;

	RECYCLEBINITEMLIST *m_pRecycleBinItem;

	struct {
		int CurrentSubItem;
		int Direction;
	} m_Sort;

	COLUMN_HANDLER_DEF<CRecycleBinListViewPage> *m_disp_proc;
	COMPARE_HANDLER_PROC_DEF<CRecycleBinListViewPage,CRecycleBinItem> *m_comp_proc;

	CColumnList m_columns;

public:
	CRecycleBinListViewPage()
	{
		m_hWndList = NULL;
		m_pRecycleBinItem = NULL;
		m_Sort.CurrentSubItem = 0;
		m_Sort.Direction = 1;
		m_disp_proc = NULL;
		m_comp_proc = NULL;
	}

	~CRecycleBinListViewPage()
	{
		delete[] m_disp_proc;
		delete[] m_comp_proc;

		if( m_pRecycleBinItem )
			FreeRecycleBinItems(m_pRecycleBinItem);
	}

	LRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_hWndList = CreateWindow(WC_LISTVIEW, 
                              L"", 
                              WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
                              0,0,0,0,
                              hWnd,
                              (HMENU)0,
                              GetModuleHandle(NULL), 
                              NULL);

		WCHAR szIniPath[MAX_PATH];
		GetIniFilePath(szIniPath,MAX_PATH);
		m_columns.SetIniFilePath(szIniPath);

		InitList(m_hWndList);

		return 0;
	}

	LRESULT OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		int cx = GET_X_LPARAM(lParam);
		int cy = GET_Y_LPARAM(lParam);
		UpdateLayout(cx,cy);
		return 0;
	}

	LRESULT OnSetFocus(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		SetFocus(m_hWndList);
		return 0;
	}

	LRESULT OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		NMHDR *pnmhdr = (NMHDR *)lParam;

		switch( pnmhdr->code )
		{
			case LVN_GETDISPINFO:
				return OnGetDispInfo(pnmhdr);
			case LVN_DELETEITEM:
				return OnDeleteItem(pnmhdr);
			case LVN_COLUMNCLICK:
				return OnColumnClick(pnmhdr);
			case LVN_GETEMPTYMARKUP:
				return OnGetEmptyMarkup(pnmhdr);
			case NM_SETFOCUS:
				return OnNmSetFocus(pnmhdr);
		}
		return 0;
	}

	LRESULT OnDeleteItem(NMHDR *pnmhdr)
	{
		NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmhdr;

		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlv->lParam;
		_SafeMemFree( pItem->pFI->ManagementPath );
		_SafeMemFree( pItem->pFI->OriginalPath );
		_SafeMemFree( pItem->pFI->Name );
		delete pItem;
		return 0;
	}

	LRESULT OnNmSetFocus(NMHDR *pnmhdr)
	{
		pnmhdr->hwndFrom = m_hWnd;
		pnmhdr->idFrom = GetWindowLong(m_hWnd,GWL_ID);
		SendMessage(GetParent(m_hWnd),WM_NOTIFY,0,(LPARAM)pnmhdr);
		return 0;
	}

	LRESULT OnGetEmptyMarkup(NMHDR *pnmhdr)
	{
		NMLVEMPTYMARKUP *pnmemptymkup = (NMLVEMPTYMARKUP *)pnmhdr;
		pnmemptymkup->dwFlags = EMF_CENTERED;
		StringCchCopy(pnmemptymkup->szMarkup,_countof(pnmemptymkup->szMarkup),L"Recycle Bin is Empty");
		RedrawWindow(m_hWndList,NULL,NULL,RDW_UPDATENOW|RDW_INVALIDATE);
		return TRUE;
	}

	LRESULT OnColumnClick(NMHDR *pnmhdr)
	{
		NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmhdr;
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlv->lParam;

		DoSort(pnmlv->iSubItem,-1);

		return 0;
	}

	void DoSort(int iSubItem=-1,BOOL bDirection=FALSE)
	{
		HWND hwndLV = m_hWndList;
	
		int id = (int)ListViewEx_GetHeaderItemData(hwndLV,iSubItem);

		if( m_Sort.CurrentSubItem != -1 )
			ListViewEx_SetHeaderArrow(hwndLV,m_Sort.CurrentSubItem,0);

		if( bDirection )
		{
			if( m_Sort.CurrentSubItem != iSubItem )
				m_Sort.Direction = -1;
			m_Sort.Direction *= -1;
		}

		SortItems(id);

		ListViewEx_SetHeaderArrow(hwndLV,iSubItem,m_Sort.Direction);

		m_Sort.CurrentSubItem = iSubItem;
	}

	LRESULT OnDisp_Name(UINT,NMLVDISPINFO *pnmlvdi)
	{
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlvdi->item.lParam;
		pnmlvdi->item.pszText = pItem->pFI->Name;
		return 0;
	}

	LRESULT OnDisp_TrushFileName(UINT,NMLVDISPINFO *pnmlvdi)
	{
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlvdi->item.lParam;
		pnmlvdi->item.pszText = PathFindFileName(pItem->pFI->ManagementPath);
		return 0;
	}

	LRESULT OnDisp_OriginalPath(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlvdi->item.lParam;
		pnmlvdi->item.pszText = pItem->pFI->OriginalPath;
		return 0;
	}

	LRESULT OnDisp_ManagementPath(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlvdi->item.lParam;
		pnmlvdi->item.pszText = pItem->pFI->ManagementPath;
		return 0;
	}

	LRESULT OnDisp_OriginalFileDate(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlvdi->item.lParam;
		ULONG64 dt = pItem->pFI->WriteTime;
		_GetDateTimeStringEx2(dt,pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,NULL,NULL,0,0);
		return 0;
	}

	LRESULT OnDisp_DateDeleted(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlvdi->item.lParam;
		ULONG64 dt = pItem->pFI->TrushFileInfo.ChangeTime.QuadPart;
		_GetDateTimeStringEx2(dt,pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,NULL,NULL,0,0);
		return 0;
	}

	LRESULT OnDisp_Size(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		CRecycleBinItem *pItem = (CRecycleBinItem *)pnmlvdi->item.lParam;

		ULONGLONG cb = pItem->pFI->Size;

		if( 1 )
		{
			StrFormatByteSizeW(cb,pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax);
		}
		else
		{
			_CommaFormatString(cb,pnmlvdi->item.pszText);
		}
		return 0;
	}

	void InitColumnTable()
	{
		static COLUMN_HANDLER_DEF<CRecycleBinListViewPage> ch[] =
		{
			COL_HANDLER_MAP_DEF(COLUMN_Name,                  &CRecycleBinListViewPage::OnDisp_Name),
			COL_HANDLER_MAP_DEF(COLUMN_TrushFileName,         &CRecycleBinListViewPage::OnDisp_TrushFileName),
			COL_HANDLER_MAP_DEF(COLUMN_Size,                  &CRecycleBinListViewPage::OnDisp_Size),
			COL_HANDLER_MAP_DEF(COLUMN_OriginalFilePath,      &CRecycleBinListViewPage::OnDisp_OriginalPath),
			COL_HANDLER_MAP_DEF(COLUMN_TrushFilePath,         &CRecycleBinListViewPage::OnDisp_ManagementPath),
			COL_HANDLER_MAP_DEF(COLUMN_OriginalLastWriteDate, &CRecycleBinListViewPage::OnDisp_OriginalFileDate),
			COL_HANDLER_MAP_DEF(COLUMN_DeletedTime,           &CRecycleBinListViewPage::OnDisp_DateDeleted),
		};

		m_disp_proc = new COLUMN_HANDLER_DEF<CRecycleBinListViewPage>[COLUMN_MaxItem];

		ZeroMemory(m_disp_proc,sizeof(COLUMN_HANDLER_DEF<CRecycleBinListViewPage>) * COLUMN_MaxItem);

		for(int i = 0; i < _countof(ch); i++)
		{
			m_disp_proc[ ch[i].colid ].colid = ch[i].colid;
			m_disp_proc[ ch[i].colid ].pfn   = ch[i].pfn;
		}
	}

	LRESULT OnGetDispInfo(NMHDR *pnmhdr)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmhdr;
		CRecycleBinItem *pItem = (CRecycleBinItem *)pdi->item.lParam;

		if( pdi->item.mask & LVIF_IMAGE )
		{
			pdi->item.iImage = GetShellIconIndexIL(pItem->pFI->pidl,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OVERLAYINDEX|SHGFI_ICON);

			if( HIBYTE(HIWORD(pdi->item.iImage)) != 0 )
			{
				pdi->item.mask |= LVIF_STATE;
				pdi->item.state = INDEXTOOVERLAYMASK( HIBYTE(HIWORD(pdi->item.iImage)) );
				pdi->item.stateMask = LVIS_OVERLAYMASK;
				pdi->item.iImage &= ~0xff000000;
			}
			pdi->item.mask |= LVIF_DI_SETITEM;
		}

		if( pdi->item.mask & LVIF_TEXT )
		{
			int id = (int)ListViewEx_GetHeaderItemData(pnmhdr->hwndFrom,pdi->item.iSubItem);

			if( m_disp_proc == NULL )
			{
				// initialize once only
				InitColumnTable();
			}

			ASSERT( id < COLUMN_MaxCount );

			if( pdi->item.mask & LVIF_TEXT )
			{
				if( (id != -1) && (id < COLUMN_MaxCount) && m_disp_proc[ id ].pfn )
					return (this->*m_disp_proc[ id ].pfn)(id,pdi);

				pdi->item.pszText = L"(No Text)";
			}
		}

		return 0;	
	}

	LRESULT OnContextMenu(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HWND hwndClick = (HWND)wParam;
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		int iSelItem;
		if( pt.x == -1 && pt.y == -1 )
		{
			// context menu key
			iSelItem = ListView_GetNextItem(m_hWndList,-1,LVNI_SELECTED|LVNI_FOCUSED);
		}
		else
		{
			// mouse button click
			MapWindowPoints(NULL,m_hWndList,&pt,1);
			UINT f = 0;
			iSelItem = ListViewEx_HitTest(m_hWndList,pt,&f);
			if( iSelItem != -1 )
			{
				if( 0x200 & f ) // undocumented flag? Hit test result on Header Controol.
				{
					iSelItem = -1;
				}
			}
		}

		if( iSelItem != -1 )
		{
			RECT rcItem;
			if( ListView_GetItemRect(m_hWndList,iSelItem,&rcItem,LVIR_BOUNDS) )
			{
				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu,MF_STRING,ID_EDIT_COPY,L"&Copy");
				AppendMenu(hMenu,MF_STRING,0,NULL);
				AppendMenu(hMenu,MF_STRING,ID_RECYCLEBIN_RESTORE,L"&Restore");
				AppendMenu(hMenu,MF_STRING,ID_RECYCLEBIN_DELETE,L"&Delete");
				SetMenuDefaultItem(hMenu,ID_FILE_OPEN_FILE,FALSE);
				if( pt.x == -1 && pt.y == -1 )
				{
					pt.x = rcItem.left;
					pt.y = rcItem.bottom;
				}
				MapWindowPoints(m_hWndList,NULL,&pt,1);
				TrackPopupMenuEx(hMenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY,pt.x,pt.y,GetActiveWindow(),NULL);
				DestroyMenu(hMenu);
			}
		}
		return 0;
	}

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
			case WM_SIZE:
				return OnSize(hWnd,uMsg,wParam,lParam);
			case WM_NOTIFY:
				return OnNotify(hWnd,uMsg,wParam,lParam);
			case WM_SETFOCUS:
				return OnSetFocus(hWnd,uMsg,wParam,lParam);
			case WM_CONTEXTMENU:
				return OnContextMenu(hWnd,uMsg,wParam,lParam);
			case FSM_FINDITEM:
				return OnFindItem(hWnd,uMsg,wParam,lParam);
			case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
			default:
				break;
		}
		return CBaseWindow::WndProc(hWnd,uMsg,wParam,lParam);
	}

	VOID UpdateLayout(int cx,int cy)
	{
		if( m_hWndList )
		{
			SetWindowPos(m_hWndList,NULL,
					0,0,cx,cy,
					SWP_NOZORDER|SWP_NOMOVE);
		}
	}

	HRESULT InitList(HWND hWndList)
	{
		_EnableVisualThemeStyle(hWndList);

		ListView_SetExtendedListViewStyle(hWndList,LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT);

		if( !LoadColumns(hWndList) )
		{
			InitColumns(hWndList);
		}

		HIMAGELIST hImageList;
		HIMAGELIST hImageListSmall;
		GetShellImageList(&hImageList,&hImageListSmall);

		ListView_SetImageList(hWndList,hImageList,LVSIL_NORMAL);
		ListView_SetImageList(hWndList,hImageListSmall,LVSIL_SMALL);

		return S_OK;
	}

	void InitColumns(HWND hWndList)
	{
		LVCOLUMN lvc = {0};

		static COLUMN columns_filelist[] = {
			{ COLUMN_Name,                   L"Name",                0, 280, LVCFMT_LEFT },
			{ COLUMN_TrushFileName,          L"Trush File Name",     1, 140, LVCFMT_LEFT },
			{ COLUMN_Size,                   L"Size",                2, 100, LVCFMT_RIGHT },
			{ COLUMN_OriginalLastWriteDate,  L"Original File Date",  3, 160, LVCFMT_LEFT },
			{ COLUMN_OriginalFilePath,       L"Original File Path",  4, 360, LVCFMT_LEFT },
			{ COLUMN_DeletedTime,            L"Deleted Date",        5, 160, LVCFMT_LEFT },
			{ COLUMN_TrushFilePath,          L"Trush File Path",     6, 360, LVCFMT_LEFT },
		};

		m_columns.SetDefaultColumns(columns_filelist,ARRAYSIZE(columns_filelist));

		int i,c;
		c = m_columns.GetDefaultColumnCount();
		for(i = 0; i < c; i++)
		{
			const COLUMN *pcol = m_columns.GetDefaultColumnItem(i);
			lvc.mask    = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER;
			lvc.fmt     = pcol->fmt;
			lvc.cx      = DPI_SIZE_CX(pcol->cx);
			lvc.pszText = pcol->Name;
			lvc.iOrder  = pcol->iOrder;
			int index = ListView_InsertColumn(hWndList,lvc.iOrder,&lvc);

			ListViewEx_SetHeaderItemData( hWndList, index, pcol->id );
		}
	}

	BOOL LoadColumns(HWND hWndList)
	{
		COLUMN_TABLE *pcoltbl;
		if( m_columns.LoadUserDefinitionColumnTable(&pcoltbl,L"ColumnLayout.RecycleBin") == 0)
			return FALSE;

		LVCOLUMN lvc = {0};

		ULONG i;
		for(i = 0; i < pcoltbl->cItems; i++)
		{
			const COLUMN *pcol = &pcoltbl->column[i];
			lvc.mask    = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER;
			lvc.fmt     = pcol->fmt;
			lvc.cx      = pcol->cx;
			lvc.pszText = pcol->Name;
			lvc.iOrder  = pcol->iOrder;
			int index = ListView_InsertColumn(hWndList,lvc.iOrder,&lvc);

			ListViewEx_SetHeaderItemData( hWndList, index, pcol->id );
		}

		m_columns.FreeUserDefinitionColumnTable(pcoltbl);

		return TRUE;
	}

	int Insert(HWND hWndList,int iGroupId,int iItem,int iImage,RECYCLEBIN_FILE_ITEM *pRBI)
	{
		if( iItem == -1 )
			iItem = ListView_GetItemCount(hWndList);

		CRecycleBinItem *pItem = new CRecycleBinItem;

		pItem->Type = 0;
		pItem->pFI  = pRBI;

		LVITEM lvi = {0};
		lvi.mask     = LVIF_TEXT|LVIF_IMAGE|LVIF_INDENT|LVIF_PARAM;
		lvi.iItem    = iItem;
		lvi.iImage   = I_IMAGECALLBACK;
		lvi.pszText  = LPSTR_TEXTCALLBACK;
		lvi.iIndent  = 0;
		lvi.lParam   = (LPARAM)pItem;

		return ListView_InsertItem(hWndList,&lvi);
	}

	HRESULT FillItems(SELECT_ITEM *pSel)
	{
		CWaitCursor wait;
		HRESULT hr;

		SetRedraw(m_hWndList,FALSE);

		ListView_DeleteAllItems(m_hWndList);

		if( m_pRecycleBinItem != NULL )
		{
			FreeRecycleBinItems(m_pRecycleBinItem);
		}

		RECYCLEBINITEMLIST *pRecycleBinItem = NULL;
		hr = EnumRecycleBinItems(&pRecycleBinItem);

		if( hr == S_OK )
		{
			ULONG i;
			for(i = 0; i < pRecycleBinItem->cItemCount; i++)
			{
				Insert(m_hWndList,-1,i,I_IMAGECALLBACK,&pRecycleBinItem->Item[i]);
			}

			DoSort(m_Sort.CurrentSubItem);
		}

		m_pRecycleBinItem = pRecycleBinItem;

		SetRedraw(m_hWndList,TRUE);

		RedrawWindow(m_hWndList,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);

		return S_OK;
	}

	virtual HRESULT UpdateData(PVOID pFile)
	{
		return FillItems((SELECT_ITEM*)pFile);
	}

	virtual HRESULT QueryCmdState(UINT CmdId,UINT *State)
	{
		switch( CmdId )
		{
			case ID_EDIT_COPY:
				*State = ListView_GetSelectedCount(m_hWndList) ? UPDUI_ENABLED : UPDUI_DISABLED;
				return S_OK;
			case ID_FILE_OPEN_FILE:
				*State = ListView_GetSelectedCount(m_hWndList) ? UPDUI_ENABLED : UPDUI_DISABLED;
				return S_OK;
			case ID_EDIT_FIND:
			case ID_EDIT_FIND_NEXT:
			case ID_EDIT_FIND_PREVIOUS:
				*State = ListView_GetItemCount(m_hWndList) ? UPDUI_ENABLED : UPDUI_DISABLED;
				return S_OK;
			case ID_VIEW_REFRESH:
				*State = UPDUI_ENABLED;
				return S_OK;
		}
		return S_FALSE;
	}

	virtual HRESULT InvokeCommand(UINT CmdId)
	{
		switch( CmdId )
		{
			case ID_EDIT_COPY:
				OnCmdEditCopy();
				break;
			case ID_VIEW_REFRESH:
				OnCmdRefresh();
				break;
			case ID_RECYCLEBIN_RESTORE:
				OnCmdRestoreFile();
				break;
			case ID_RECYCLEBIN_DELETE:
				OnCmdDeleteFile();
				break;
		}
		return S_OK;
	}

	void OnCmdEditCopy()
	{
		if( GetKeyState(VK_SHIFT) < 0 )
		{
			SetClipboardTextFromListViewColumn(m_hWndList,SCTEXT_FORMAT_SELECTONLY|SCTEXT_FORMAT_DOUBLEQUATE,-1);
		}
		else
		{
			SetClipboardTextFromListView(m_hWndList,SCTEXT_UNICODE);
		}
	}

	void OnCmdRefresh()
	{
		SELECT_ITEM sel = {0};
		FillItems(&sel);
	}

	void OnCmdRestoreFile()
	{
		int iSelItem = ListView_GetNextItem(m_hWndList,-1,LVNI_SELECTED);
		if( iSelItem == -1 )
			return;

		int iItem = iSelItem;
		if( iItem != -1 )
		{
			HRESULT hr;
			do
			{
				CRecycleBinItem *pItem = (CRecycleBinItem *)ListViewEx_GetItemData(m_hWndList,iItem);

				hr = ExecRecycleBinItemCommand(m_hWnd,pItem->pFI->pidl,"undelete");

				if( hr == S_OK )
				{
					ListView_DeleteItem(m_hWndList,iItem);
					iItem--;
				}

				iItem = ListView_GetNextItem(m_hWndList,iItem,LVNI_SELECTED);
			}
			while(iItem != -1);
		}

		ListView_SetItemState(m_hWndList,iSelItem,LVNI_FOCUSED,LVNI_FOCUSED);
		SetFocus(m_hWndList);
	}

	LRESULT OnCmdDeleteFile()
	{
		if( GetVersion() < 0x600 )
		{
			int iSelItem = ListView_GetNextItem(m_hWndList,-1,LVNI_SELECTED);
			if( iSelItem == -1 )
				return 0;

			int iItem = iSelItem;
			if( iItem != -1 )
			{
				HRESULT hr;
				do
				{
					CRecycleBinItem *pItem = (CRecycleBinItem *)ListViewEx_GetItemData(m_hWndList,iItem);

					hr = ExecRecycleBinItemCommand(m_hWnd,pItem->pFI->pidl,"delete");

					if( hr == S_OK )
					{
						ListView_DeleteItem(m_hWndList,iItem);
						iItem--;
					}

					iItem = ListView_GetNextItem(m_hWndList,iItem,LVNI_SELECTED);
				}
				while(iItem != -1);
			}

			ListView_SetItemState(m_hWndList,iSelItem,LVNI_FOCUSED,LVNI_FOCUSED);
			SetFocus(m_hWndList);

			return 0;
		}

		int iSelItem = ListView_GetNextItem(m_hWndList,-1,LVNI_SELECTED);
		if( iSelItem == -1 )
			return 0;

		int iItem = iSelItem;
		if( iItem != -1 )
		{
			CSimpleValArray<LPITEMIDLIST> idls;

			HRESULT hr;
			do
			{
				CRecycleBinItem *pItem = (CRecycleBinItem *)ListViewEx_GetItemData(m_hWndList,iItem);

				idls.Add( pItem->pFI->pidl );

				iItem = ListView_GetNextItem(m_hWndList,iItem,LVNI_SELECTED);
			}
			while(iItem != -1);

			IShellFolder *pBitBucketFolder = NULL;
			IShellItemArray *psiItemArray = NULL;
			IFileOperation *pfo = NULL;
			do
			{
				hr = GetRecycleBinFolder( &pBitBucketFolder );
				if( FAILED(hr) )
					break;

				hr = SHCreateShellItemArray(NULL,pBitBucketFolder,
							idls.GetCount(),
							(PCUITEMID_CHILD_ARRAY)idls.GetBuffer(),
							&psiItemArray);
				if( FAILED(hr) )
					break;

				hr = CoCreateInstance(CLSID_FileOperation,NULL,CLSCTX_INPROC_SERVER,
							__uuidof(IFileOperation),(PVOID *)&pfo);
				if( FAILED(hr) )
						break;

				pfo->SetOwnerWindow( m_hWnd );

				HWND hwndActive = GetActiveWindow();
				EnableWindow(hwndActive,FALSE);

				hr = pfo->DeleteItems(psiItemArray);

				if( SUCCEEDED(hr) )
				{
					hr = pfo->PerformOperations();
				}

				EnableWindow(hwndActive,TRUE);

				//
				// Update list view
				//
				iItem = ListView_GetNextItem(m_hWndList,-1,LVNI_ALL|LVNI_SELECTED);
				if( iItem != - 1 )
				{
					do
					{
						CRecycleBinItem *pItem = (CRecycleBinItem *)ListViewEx_GetItemData(m_hWndList,iItem);

						if( !PathFileExists( pItem->pFI->ManagementPath ) )
						{
							ListView_DeleteItem(m_hWndList,iItem);
							iItem--;
						}

						iItem = ListView_GetNextItem(m_hWndList,iItem,LVNI_SELECTED);
					}
					while(iItem != -1);
				}
				ListView_SetItemState(m_hWndList,iSelItem,LVNI_FOCUSED,LVNI_FOCUSED);
				SetFocus(m_hWndList);

			} while( 0 );

			_SAFE_RELEASE(pBitBucketFolder);
			_SAFE_RELEASE(psiItemArray);
			_SAFE_RELEASE(pfo);
		}

		return 0;
	}

private:

	//////////////////////////////////////////////////////////////////////////
	//
	//  Sort
	//
	typedef struct _SORT_OPTION
	{
		CRecycleBinListViewPage *pThis;
		UINT id;
		int direction;
	} SORT_OPTION;

	int comp_name(CRecycleBinItem *pItem1,CRecycleBinItem *pItem2, const void *p)
	{
		return StrCmpLogicalW(pItem1->pFI->Name,pItem2->pFI->Name);
	}

	int comp_originalfilepath(CRecycleBinItem *pItem1,CRecycleBinItem *pItem2, const void *p)
	{
		return StrCmpLogicalW(pItem1->pFI->OriginalPath,pItem2->pFI->OriginalPath);
	}

	int comp_trushfilepath(CRecycleBinItem *pItem1,CRecycleBinItem *pItem2, const void *p)
	{
		return StrCmpLogicalW(pItem1->pFI->ManagementPath,pItem2->pFI->ManagementPath);
	}

	int comp_originallastwritedate(CRecycleBinItem *pItem1,CRecycleBinItem *pItem2, const void *p)
	{
		return _COMP(pItem1->pFI->WriteTime,pItem2->pFI->WriteTime);
	}

	int comp_deletedtime(CRecycleBinItem *pItem1,CRecycleBinItem *pItem2, const void *p)
	{
		return _COMP(pItem1->pFI->TrushFileInfo.ChangeTime.QuadPart,pItem2->pFI->TrushFileInfo.ChangeTime.QuadPart);
	}

	int comp_size(CRecycleBinItem *pItem1,CRecycleBinItem *pItem2, const void *p)
	{
		return _COMP(pItem1->pFI->Size,pItem2->pFI->Size);
	}

	void init_compare_proc_def_table()
	{
		static COMPARE_HANDLER_PROC_DEF<CRecycleBinListViewPage,CRecycleBinItem> comp_proc[] = 
		{
			{COLUMN_Name,                  &CRecycleBinListViewPage::comp_name},
			{COLUMN_Size,                  &CRecycleBinListViewPage::comp_size},
			{COLUMN_OriginalFilePath,      &CRecycleBinListViewPage::comp_originalfilepath},
			{COLUMN_TrushFilePath,         &CRecycleBinListViewPage::comp_trushfilepath},
			{COLUMN_DeletedTime,           &CRecycleBinListViewPage::comp_deletedtime},
			{COLUMN_OriginalLastWriteDate, &CRecycleBinListViewPage::comp_originallastwritedate},
		};

		m_comp_proc = new COMPARE_HANDLER_PROC_DEF<CRecycleBinListViewPage,CRecycleBinItem>[COLUMN_MaxItem];

		ZeroMemory(m_comp_proc,sizeof(COMPARE_HANDLER_PROC_DEF<CRecycleBinListViewPage,CRecycleBinItem>)*COLUMN_MaxItem);

		int i;
		for(i = 0; i < _countof(comp_proc); i++)
		{
			m_comp_proc[ comp_proc[i].colid ].colid = comp_proc[i].colid;
			m_comp_proc[ comp_proc[i].colid ].proc  = comp_proc[i].proc;
		}
	}

	int CompareItem(CRecycleBinItem *pItem1,CRecycleBinItem *pItem2,SORT_PARAM<CRecycleBinListViewPage> *op)
	{
		if( m_comp_proc == NULL )
		{
			init_compare_proc_def_table();
		}

		int iResult = 0;

		if( iResult == 0 && m_comp_proc[op->id].proc != NULL )
		{
			iResult = (this->*m_comp_proc[op->id].proc)(pItem1,pItem2,op);
		}

		iResult *= op->direction;

		return iResult;
	}

	static int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		CRecycleBinItem *pItem1 = (CRecycleBinItem *)lParam1;
		CRecycleBinItem *pItem2 = (CRecycleBinItem *)lParam2;
		SORT_PARAM<CRecycleBinListViewPage> *op = (SORT_PARAM<CRecycleBinListViewPage> *)lParamSort;
		return op->pThis->CompareItem(pItem1,pItem2,op);
	}

	void SortItems(UINT id)
	{
		SORT_PARAM<CRecycleBinListViewPage> op = {0};
		op.pThis = this;
		op.id = id;
		op.direction = m_Sort.Direction; // 1 or -1 do not use 0
		ListView_SortItems(m_hWndList,CompareProc,&op);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//  Find Item Mechanism
	//

	int m_iStartFindItem;
	int m_iFirstMatchItem;
	int m_iLastMatchItem;

	LRESULT OnFindItem(HWND,UINT,WPARAM wParam,LPARAM lParam)
	{
		LPFINDREPLACE lpfr = (LPFINDREPLACE)lParam;
		switch( LOWORD(wParam) )
		{
			case FIND_QUERYOPENDIALOG:
				return 0; // 0:accept 1:prevent

			case FIND_CLOSEDIALOG:
				m_iStartFindItem = -1;
				break;

			case FIND_SEARCH:
				m_iStartFindItem = ListViewEx_GetCurSel(m_hWndList);
				if( m_iStartFindItem == -1 )
					m_iStartFindItem = 0;
				SearchItem(lpfr->lpstrFindWhat,
						(BOOL) (lpfr->Flags & FR_DOWN), 
						(BOOL) (lpfr->Flags & FR_MATCHCASE)); 
				break;
			case FIND_SEARCH_NEXT:
			{
				int cItems = ListView_GetItemCount(m_hWndList);
				m_iStartFindItem = ListViewEx_GetCurSel(m_hWndList);
				if( m_iStartFindItem == -1 )
					m_iStartFindItem = 0;
				else
				{
					m_iStartFindItem = m_iStartFindItem + ((lpfr->Flags & FR_DOWN) ? 1 : -1);
					if( m_iStartFindItem <= 0 )
						m_iStartFindItem = 0;
					else if( m_iStartFindItem >= cItems )
						m_iStartFindItem = cItems-1;
				}
				SearchItem(lpfr->lpstrFindWhat,
						(BOOL) (lpfr->Flags & FR_DOWN), 
						(BOOL) (lpfr->Flags & FR_MATCHCASE)); 
				break;
			}
		}
		return 0;
	}

	VOID SearchItem(PWSTR pszFindText,BOOL Down,BOOL MatchCase)
	{
		int iItem,col,cItems,cColumns;

		const int cchText = MAX_PATH;
		WCHAR szText[cchText];

		cItems = ListView_GetItemCount(m_hWndList);
		cColumns = ListViewEx_GetColumnCount(m_hWndList);

		iItem = m_iStartFindItem;

		for(;;)
		{
			for(col = 0; col < cColumns; col++)
			{
				ListView_GetItemText(m_hWndList,iItem,col,szText,cchText);

				if( StrStrI(szText,pszFindText) != 0 )
				{
					ListViewEx_ClearSelectAll(m_hWndList,TRUE);
					ListView_SetItemState(m_hWndList,iItem,LVNI_SELECTED|LVNI_FOCUSED,LVNI_SELECTED|LVNI_FOCUSED);
					ListView_EnsureVisible(m_hWndList,iItem,FALSE);
					goto __found;
				}
			}

			Down ? iItem++ : iItem--;

			if( iItem == m_iStartFindItem )
			{
				MessageBeep(-1);
				break; // not found
			}

			// lap around
			if( iItem >= cItems )
				iItem = 0;
			else if( iItem < 0 )
				iItem = cItems-1;
		}

 __found:
		return;
	}
};
