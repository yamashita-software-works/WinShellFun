#pragma once
//***************************************************************************
//*                                                                         *
//*  page_shellitemlist.h                                                   *
//*                                                                         *
//*  Shell folder contents browser list page.                               *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Create: 2023-07-06                                                     *
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
#include "folderhelp.h"

typedef struct _LVSHELLITEM
{
	UINT Type;
	CShellItem *pFI;
} LVSHELLITEM, *PLVSHELLITEM;

struct CLVShellItem : public LVSHELLITEM
{
	CLVShellItem()
	{
		memset(this,0,sizeof(LVSHELLITEM));
	}
};

//////////////////////////////////////////////////////////////////////////////

class CShellItemListViewPage : public CPageWndBase
{
	HWND m_hWndList;

	struct {
		int CurrentSubItem;
		int Direction;
	} m_Sort;

	COLUMN_HANDLER_DEF<CShellItemListViewPage> *m_disp_proc;
	COMPARE_HANDLER_PROC_DEF<CShellItemListViewPage,CLVShellItem> *m_comp_proc;

	CColumnList m_columns;

	GUID m_CurGuid;
	PWSTR m_pszCurPath;

public:
	CShellItemListViewPage()
	{
		m_hWndList = NULL;
		m_Sort.CurrentSubItem = -1;
		m_Sort.Direction = 0;
		m_disp_proc = NULL;
		m_comp_proc = NULL;
		m_pszCurPath = NULL;
		ZeroMemory(&m_CurGuid,sizeof(m_CurGuid));
	}

	~CShellItemListViewPage()
	{
		delete[] m_disp_proc;
		delete[] m_comp_proc;
		_SafeMemFree(m_pszCurPath);
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
			case LVN_ITEMACTIVATE:
				return OnItemActivate(pnmhdr);
			case LVN_COLUMNCLICK:
				return OnColumnClick(pnmhdr);
			case NM_SETFOCUS:
				return OnNmSetFocus(pnmhdr);
		}
		return 0;
	}

	LRESULT OnDeleteItem(NMHDR *pnmhdr)
	{
		NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmhdr;

		CLVShellItem *pItem = (CLVShellItem *)pnmlv->lParam;
		delete pItem->pFI;
		delete pItem;
		return 0;
	}

	LRESULT OnItemActivate(NMHDR *pnmhdr)
	{
		NMITEMACTIVATE *pnmia = (NMITEMACTIVATE *)pnmhdr;

		OpenItem(pnmia->iItem);
	
		return 0;
	}

	LRESULT OnNmSetFocus(NMHDR *pnmhdr)
	{
		pnmhdr->hwndFrom = m_hWnd;
		pnmhdr->idFrom = GetWindowLong(m_hWnd,GWL_ID);
		SendMessage(GetParent(m_hWnd),WM_NOTIFY,0,(LPARAM)pnmhdr);
		return 0;
	}

	LRESULT OnColumnClick(NMHDR *pnmhdr)
	{
		NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmhdr;
		CLVShellItem *pItem = (CLVShellItem *)pnmlv->lParam;

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
			if( iSubItem == 0 )
			{
				// 'name' column
				if( m_Sort.CurrentSubItem != iSubItem )
				{
					m_Sort.Direction = 0;
				}
				else
				{
					if( m_Sort.Direction == 0 )
						m_Sort.Direction = 1;
					else if( m_Sort.Direction == 1 )
						m_Sort.Direction = 2;
					else if( m_Sort.Direction == 2 )
						m_Sort.Direction = 0;
				}
			}
			else
			{
				// other columns
				if( m_Sort.CurrentSubItem != iSubItem )
					m_Sort.Direction = -1;
				m_Sort.Direction *= -1;
			}
		}

		SortItems(id,NULL);

		if( iSubItem == 0 )
		{
			int n;
			if( m_Sort.Direction == 0 )
				n = 1;
			else if( m_Sort.Direction == 1 )
				n = -1;
			else
				n = 0;

			ListViewEx_SetHeaderArrow(hwndLV,iSubItem,n);
		}
		else
		{
			ListViewEx_SetHeaderArrow(hwndLV,iSubItem,m_Sort.Direction);
		}

		m_Sort.CurrentSubItem = iSubItem;
	}

	LRESULT OnDisp_Name(UINT,NMLVDISPINFO *pnmlvdi)
	{
		CLVShellItem *pItem = (CLVShellItem *)pnmlvdi->item.lParam;
		pnmlvdi->item.pszText = pItem->pFI->hdr.FileName;
		return 0;
	}

	LRESULT OnDisp_Attributes(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		CLVShellItem *pItem = (CLVShellItem *)pnmlvdi->item.lParam;
		StringCchPrintf(pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,L"0x%08X",pItem->pFI->FileAttributes);
		return 0;
	}

	LRESULT OnDisp_Path(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		CLVShellItem *pItem = (CLVShellItem *)pnmlvdi->item.lParam;
		pnmlvdi->item.pszText = pItem->pFI->hdr.Path;
		return 0;
	}

	void InitColumnTable()
	{
		static COLUMN_HANDLER_DEF<CShellItemListViewPage> ch[] =
		{
			COL_HANDLER_MAP_DEF(COLUMN_Name,                   &CShellItemListViewPage::OnDisp_Name),
			COL_HANDLER_MAP_DEF(COLUMN_ShellItemAttributes,    &CShellItemListViewPage::OnDisp_Attributes),
			COL_HANDLER_MAP_DEF(COLUMN_ShellItemParseName,     &CShellItemListViewPage::OnDisp_Path),
		};

		m_disp_proc = new COLUMN_HANDLER_DEF<CShellItemListViewPage>[COLUMN_MaxItem];

		ZeroMemory(m_disp_proc,sizeof(COLUMN_HANDLER_DEF<CShellItemListViewPage>) * COLUMN_MaxItem);

		for(int i = 0; i < _countof(ch); i++)
		{
			m_disp_proc[ ch[i].colid ].colid = ch[i].colid;
			m_disp_proc[ ch[i].colid ].pfn   = ch[i].pfn;
		}
	}

	LRESULT OnGetDispInfo(NMHDR *pnmhdr)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmhdr;
		CLVShellItem *pItem = (CLVShellItem *)pdi->item.lParam;

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
				AppendMenu(hMenu,MF_STRING,ID_FILE_OPEN_FILE,L"&Open");
				AppendMenu(hMenu,MF_STRING,ID_EDIT_COPY,L"&Copy");
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
			{ COLUMN_Name,                L"Name",          0, 280, LVCFMT_LEFT|LVCFMT_SPLITBUTTON },
			{ COLUMN_ShellItemAttributes, L"Attributes",    1, 100, LVCFMT_LEFT },
			{ COLUMN_ShellItemParseName,  L"Parse Name",    2, 360, LVCFMT_LEFT },
		};

		m_columns.SetDefaultColumns(columns_filelist,ARRAYSIZE(columns_filelist));

		int i,c;
		c = m_columns.GetDefaultColumnCount();
		for(i = 0; i < c; i++)
		{
			const COLUMN *pcol = m_columns.GetDefaultColumnItem(i);
			lvc.mask    = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER;
			lvc.fmt     = pcol->fmt;
			lvc.cx      = pcol->cx;
			lvc.pszText = pcol->Name;
			lvc.iOrder  = pcol->iOrder;
			int index = ListView_InsertColumn(hWndList,lvc.iOrder,&lvc);

			ListViewEx_SetHeaderItemData( hWndList, index, pcol->id );
		}
	}

	BOOL LoadColumns(HWND hWndList)
	{
		COLUMN_TABLE *pcoltbl;
		if( m_columns.LoadUserDefinitionColumnTable(&pcoltbl,L"ColumnLayout") == 0)
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

	int Insert(HWND hWndList,int iGroupId,int iItem,int iImage,CShellItem *pFI)
	{
		if( iItem == -1 )
			iItem = ListView_GetItemCount(hWndList);

		CLVShellItem *pItem = new CLVShellItem;

		pItem->Type = 0;
		pItem->pFI  = pFI;

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

		PtrArray<CShellItem> *pa = new PtrArray<CShellItem>;

		pa->Create( 4096 );

		hr = EnumShellItems(pSel->pGuid,pSel->pszPath,pa);

		if( hr == S_OK )
		{
			_SafeMemFree(m_pszCurPath);

			m_CurGuid = *pSel->pGuid;
			if( pSel->pszPath )
				m_pszCurPath = _MemAllocString(pSel->pszPath);

			SHELLITEMLIST fl = {0};
			fl.cItemCount = pa->GetCount();
			fl.pFI = (CShellItem **)pa->GetData();
			fl.Reserved = pSel->pszPath;

			ULONG i;
			for(i = 0; i < fl.cItemCount; i++)
			{
				Insert(m_hWndList,-1,i,I_IMAGECALLBACK,fl.pFI[i]);
			}

			DoSort(m_Sort.CurrentSubItem);
		}

		delete pa;

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
			case ID_FILE_OPEN_FILE:
				*State = ListView_GetSelectedCount(m_hWndList) ? UPDUI_ENABLED : UPDUI_DISABLED;
				return S_OK;
			case ID_EDIT_FIND:
			case ID_EDIT_FIND_NEXT:
			case ID_EDIT_FIND_PREVIOUS:
			case ID_VIEW_REFRESH:
				*State = ListView_GetItemCount(m_hWndList) ? UPDUI_ENABLED : UPDUI_DISABLED;
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
			case ID_FILE_OPEN_FILE:
				OnCmdOpenFile();
				break;
			case ID_VIEW_REFRESH:
				OnCmdRefresh();
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

	void OnCmdOpenFile()
	{
		int iItem = ListViewEx_GetCurSel(m_hWndList);
		if( iItem != -1 )
			OpenItem(iItem);
	}

	void OnCmdRefresh()
	{
		SELECT_ITEM sel = {0};
		sel.pGuid = &m_CurGuid;
		sel.pszPath = m_pszCurPath;
		FillItems(&sel);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//  Works
	//
private:
	HRESULT OpenItem(int iItem)
	{
		CLVShellItem *pItem = (CLVShellItem *)ListViewEx_GetItemData(m_hWndList,iItem);

		return OpenShellItemIL((LPITEMIDLIST)pItem->pFI->pidl);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//  Sort
	//
	typedef struct _SORT_OPTION
	{
		CShellItemListViewPage *pThis;
		UINT id;
		int direction;
	} SORT_OPTION;

	int comp_name(CLVShellItem *p1,CLVShellItem *p2,const void *p)
	{
		CLVShellItem *pItem1 = (CLVShellItem *)p1;
		CLVShellItem *pItem2 = (CLVShellItem *)p2;
		SORT_OPTION *op = (SORT_OPTION *)p;
		
		int iret;
		if( op->direction == 2 )
			iret = _COMP(pItem1->pFI->EnumOrderIndex,pItem2->pFI->EnumOrderIndex);
		else
		{
			int d = (op->direction == 0) ? 1  : -1;
			iret = StrCmp(pItem1->pFI->hdr.FileName,pItem2->pFI->hdr.FileName);
			iret *= d;
		}
		return iret;
	}

	int comp_parsename(CLVShellItem *p1,CLVShellItem *p2,const void *p)
	{
		CLVShellItem *pItem1 = (CLVShellItem *)p1;
		CLVShellItem *pItem2 = (CLVShellItem *)p2;
		SORT_OPTION *op = (SORT_OPTION *)p;
		return StrCmp(pItem1->pFI->hdr.Path,pItem2->pFI->hdr.Path);
	}

	int comp_attributes(CLVShellItem *p1,CLVShellItem *p2,const void *p)
	{
		CLVShellItem *pItem1 = (CLVShellItem *)p1;
		CLVShellItem *pItem2 = (CLVShellItem *)p2;
		SORT_OPTION *op = (SORT_OPTION *)p;

		ULONG ln1,ln2;

		ln1 = pItem1->pFI->FileAttributes;
		ln2 = pItem2->pFI->FileAttributes;

		return _COMP(ln1,ln2);
	}

	void InitCompareItemTable()
	{
		static COMPARE_HANDLER_PROC_DEF<CShellItemListViewPage,CLVShellItem> comp_proc[] = 
		{
			{COLUMN_Name,                &CShellItemListViewPage::comp_name},
			{COLUMN_ShellItemAttributes, &CShellItemListViewPage::comp_attributes},
			{COLUMN_ShellItemParseName,  &CShellItemListViewPage::comp_parsename},
		};

		m_comp_proc = new COMPARE_HANDLER_PROC_DEF<CShellItemListViewPage,CLVShellItem>[COLUMN_MaxCount];

		ZeroMemory(m_comp_proc,sizeof(COMPARE_HANDLER_PROC_DEF<CShellItemListViewPage,CLVShellItem>)*COLUMN_MaxItem);

		for(int i = 0; i < _countof(comp_proc); i++)
		{
			m_comp_proc[ comp_proc[i].colid ].colid = comp_proc[i].colid;
			m_comp_proc[ comp_proc[i].colid ].proc  = comp_proc[i].proc;
		}
	}

	int CompareItem(CLVShellItem *pItem1,CLVShellItem *pItem2,SORT_OPTION *op)
	{
		int iResult = 0;

		if( m_comp_proc == NULL )
		{
			InitCompareItemTable();
		}

		if( iResult == 0 && m_comp_proc[op->id].proc != NULL )
			iResult = (this->*m_comp_proc[op->id].proc)(pItem1,pItem2,op);

		if( op->id != COLUMN_Name )
			iResult *= op->direction;

		return iResult;
	}

	static int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		CLVShellItem *pItem1 = (CLVShellItem *)lParam1;
		CLVShellItem *pItem2 = (CLVShellItem *)lParam2;
		SORT_OPTION *op = (SORT_OPTION *)lParamSort;
		return op->pThis->CompareItem(pItem1,pItem2,op);
	}

	void SortItems(UINT id,CLVShellItem *)
	{
		SORT_OPTION op = {0};
		op.pThis = this;
		op.id = id;
		op.direction = m_Sort.Direction; // 1 or -1 do not use 0 (exclude the name column)
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
