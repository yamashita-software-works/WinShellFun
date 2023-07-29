//****************************************************************************
//
//  shellfoldertree.cpp
//
//  Implements the shell folder tree view host window.
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
#include "basewindow.h"
#include "shellfoldertree.h"
#include "miscshell.h"
#include "folderhelp.h"

#define WC_FOLDERTREEWINDOW L"ShellFolderTreeWindow"

#define ID_TREEVIEW 1

class CShellFolderTree : public CBaseWindow
{
public:
	HWND m_hWndTree;
	HWND m_hWndNotice;

	VOID SetNotifyWnd(HWND hwnd) { m_hWndNotice = hwnd; }
	HWND GetNotifyWnd() { return m_hWndNotice; }

	PWSTR m_pszDirectoryPath;

	BOOL m_bPreventAction;

	BOOL m_bGetUniqueIcon;

	CShellFolderTree()
	{
		m_hWndTree = NULL;
		m_hWndNotice = NULL;
		m_pszDirectoryPath = NULL;
		m_bPreventAction = FALSE;
		m_bGetUniqueIcon = FALSE;
	}

	~CShellFolderTree()
	{
		_SafeMemFree( m_pszDirectoryPath );
	}

	typedef struct _TREEITEM
	{
		TREEITEMTYPE ItemType;
		PWSTR DisplayName;
		PWSTR Path;
		GUID Guid;
	} TREEITEM, *PTREEITEM;

	HTREEITEM
	AddItem(
		TREEITEMTYPE ItemType,
		PCWSTR pszDisplayName,
		PCWSTR pszPath,
		GUID *pGuid,
		PVOID Reserved1,
		PVOID Reserved2,
		int iImage=-2,
		UINT Flags=0,
		HTREEITEM hItem = TVI_ROOT,
		HTREEITEM hAfter = TVI_LAST,
		int HasChild = 0
		)
	{
		TVINSERTSTRUCT tvins = {0};

		tvins.hParent = hItem;
		tvins.hInsertAfter = hAfter;
		tvins.itemex.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|TVIF_CHILDREN;
		tvins.itemex.pszText = (PWSTR)pszDisplayName;
		tvins.itemex.cChildren = HasChild;

		TREEITEM *pItem = new TREEITEM;

		ZeroMemory(pItem,sizeof(TREEITEM));

		pItem->ItemType = ItemType;
		pItem->DisplayName = _MemAllocString(pszDisplayName);
		if( pszPath )
			pItem->Path = _MemAllocString(pszPath);
		if( pGuid )
			pItem->Guid = *pGuid;

		tvins.itemex.pszText = (PWSTR)pszDisplayName; // LPSTR_TEXTCALLBACK;
		tvins.itemex.iImage = iImage;                 // I_IMAGECALLBACK;
		tvins.itemex.iSelectedImage = tvins.itemex.iImage;

		tvins.itemex.lParam = (LPARAM)pItem;

		return TreeView_InsertItem(m_hWndTree,&tvins);
	}

	HTREEITEM InsertBlank(HTREEITEM hParent,HTREEITEM hInsert)
	{
		TREEITEM *pItem = new TREEITEM;
		memset(pItem,0,sizeof(TREEITEM));

		pItem->ItemType = ITEM_BLANK;

		TVINSERTSTRUCT tvi = {0};
		tvi.hInsertAfter = TVI_LAST;
		tvi.hParent =  TVI_ROOT;
		tvi.itemex.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_PARAM|TVIF_INTEGRAL;
		tvi.itemex.pszText = L"";
		tvi.itemex.lParam = (LPARAM)pItem;
		tvi.itemex.iImage = tvi.itemex.iExpandedImage = I_IMAGENONE;
		tvi.itemex.iIntegral = 1;  // blank line height
		tvi.itemex.uStateEx = 0;
		HTREEITEM h = TreeView_InsertItem(m_hWndTree, &tvi);
		TVITEMEX tviex = {0};
		tviex.mask = TVIF_STATEEX;
		tviex.hItem = h;
		tviex.uStateEx = TVIS_EX_DISABLED;
		TreeView_SetItem( m_hWndTree, &tviex );
		return h;
	}

	DWORD UpdateView( PCWSTR )
	{
		SetRedraw(m_hWndTree,FALSE);
		m_bPreventAction = TRUE;

		DWORD dwError;

		dwError = 0;

		if( dwError == ERROR_SUCCESS )
		{
			_SafeMemFree( m_pszDirectoryPath );

			// Prevent repeated notification message of selection item at next lines DeleteAllItems.
			TreeView_Select(m_hWndTree,NULL,TVGN_CARET);
			TreeView_DeleteAllItems( m_hWndTree );

			//
			// Shell Folders
			//
			FOLDER_GUID_TABLE *pTblPtr = NULL;
			WCHAR szIniFile[MAX_PATH];

			if( GetIniFilePath(szIniFile,MAX_PATH) && 
				LoadFolderGuids(&pTblPtr,szIniFile) > 0 )
			{
				WCHAR sz[260];
				int iImageIndex;
				HICON hIcon;
				ULONG i;

				for(i = 0; i < pTblPtr->cItems; i++)
				{
					hIcon = NULL;
					iImageIndex = I_IMAGENONE;

					if( IsEqualGUID(FOLDERID_GodMode,pTblPtr->Folder[i].Guid) )
					{
						WCHAR szAllTasks[MAX_PATH];
						MakeTempGodModeFolder(szAllTasks,MAX_PATH);	

						GetShellFolderNameFromPath(szAllTasks,sz,ARRAYSIZE(sz),NULL,&hIcon);

						iImageIndex = ImageList_AddIcon(TreeView_GetImageList(m_hWndTree,TVSIL_NORMAL),hIcon);
						DestroyIcon(hIcon);
						AddItem(ITEM_SHELL_FOLDER_ITEMS,L"God Mode",szAllTasks,&pTblPtr->Folder[i].Guid,NULL,NULL,iImageIndex,0,TVI_ROOT,TVI_LAST);
					}
					else
					{
						if( GetShellFolderName(&pTblPtr->Folder[i].Guid,sz,ARRAYSIZE(sz),NULL,&hIcon) )
						{
							iImageIndex = ImageList_AddIcon(TreeView_GetImageList(m_hWndTree,TVSIL_NORMAL),hIcon);
							DestroyIcon(hIcon);
							AddItem(ITEM_SHELL_FOLDER_ITEMS,sz,NULL,&pTblPtr->Folder[i].Guid,NULL,NULL,iImageIndex,0,TVI_ROOT,TVI_LAST);
						}
					}
				}

				FreeFolderGuids(pTblPtr);

				dwError = ERROR_SUCCESS;
			}
			else
			{
				dwError = GetLastError();
			}
		}

		m_bPreventAction = FALSE;
		SetRedraw(m_hWndTree,TRUE);

		return dwError;
	}

	VOID FreeFileItems(PtrArray<CShellItem>& pa)
	{
		int cFiles = 0;
		int i,cItems = pa.GetCount();

		for(i = 0; i < cItems; i++)
		{
			delete pa.Get(i);
		}		

		pa.DeleteAll();		
	}

	BOOL SelectFolder(PCWSTR pszFolderName)
	{
		HTREEITEM hItem;

		hItem = FindItem(pszFolderName);

		if( hItem )
		{
			return TreeView_Select(m_hWndTree,hItem,TVGN_CARET);
		}
		return FALSE;
	}

	HTREEITEM FindItem(PCWSTR pszName)
	{
		WCHAR szItem[MAX_PATH];

		HTREEITEM hItem = TreeView_GetRoot(m_hWndTree);
		while( hItem )
		{
			if( TreeView_GetItemText(m_hWndTree,hItem,szItem,ARRAYSIZE(szItem)) )
			{
				if( wcsicmp(szItem,pszName) == 0 )
				{
					break;
				}
			}
			hItem = TreeView_GetNextSibling(m_hWndTree,hItem);
		}
		return hItem;
	}

	VOID NotifyHost(USHORT code,GUID *pGuid,PCWSTR pszDisplayName,PCWSTR pszPath,TREEITEM *pItem,HICON hIcon)
	{
		WPARAM wParam;
		wParam = MAKEWPARAM(code,0); // hiword:reserved

		SELECT_ITEM path = {0};
		path.pszPath = (PWSTR)pszPath;
		path.pszName = (PWSTR)pszDisplayName;
		path.pszLocation = (PWSTR)NULL;
		path.pGuid = pGuid;
		path.Type = pItem->ItemType;
		path.hIcon = hIcon;

		SendMessage(m_hWndNotice,WM_CONTROL_MESSAGE,wParam,(LPARAM)&path);
	}

	HWND CreateFolderTreeView(HWND hwndParent)
	{ 
		HWND hwndTreeView;

		// TVS_LINESATROOT
		hwndTreeView = CreateWindowEx(0,
							WC_TREEVIEW,
							TEXT("ShellFolderTreeCtrl"),
							WS_VISIBLE|WS_CHILD|WS_TABSTOP|TVS_DISABLEDRAGDROP|TVS_NOHSCROLL|
							TVS_HASBUTTONS|TVS_INFOTIP|TVS_FULLROWSELECT|TVS_NONEVENHEIGHT|TVS_SHOWSELALWAYS, 
							0, 0, 0, 0,
							hwndParent, 
							(HMENU)ID_TREEVIEW, 
							GETINSTANCE(hwndParent),
							NULL); 

		TreeView_SetExtendedStyle(hwndTreeView,
					TVS_EX_DOUBLEBUFFER,
					TVS_EX_DOUBLEBUFFER
					);

		_EnableVisualThemeStyle(hwndTreeView);

		HIMAGELIST himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32,64,16);
		TreeView_SetImageList(hwndTreeView,himl,TVSIL_NORMAL);

		int cx,cy;
		ImageList_GetIconSize(himl,&cx,&cy);

		TreeView_SetIndent(hwndTreeView,cx/2);
		TreeView_SetItemHeight(hwndTreeView,cy+8);

		TreeView_SetBkColor(hwndTreeView,RGB(243,243,243)); // dark mode not considering

		return hwndTreeView;
	}

	/////////////////////////////////////////////////////////////////////////////

	//
	// Message Handlers
	//
	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		m_hWndTree = CreateFolderTreeView(hWnd);
		return 0;
	}

	LRESULT OnDestroy(HWND,UINT,WPARAM,LPARAM)
	{
		HIMAGELIST himl = TreeView_GetImageList(m_hWndTree,TVSIL_NORMAL);
		if( himl )
			ImageList_Destroy(himl);
		return 0;
	}

	LRESULT OnSetFocus(HWND,UINT,WPARAM,LPARAM)
	{
		SetFocus(m_hWndTree);
		return 0;
	}

	LRESULT OnSize(HWND,UINT,WPARAM,LPARAM lParam)
	{
		if( m_hWndTree )
		{
			int cx = GET_X_LPARAM(lParam);
			int cy = GET_Y_LPARAM(lParam);
			SetWindowPos(m_hWndTree,NULL,0,0,cx,cy,SWP_NOZORDER|SWP_NOMOVE);
		}
		return 0;
	}

	LRESULT OnDeleteItem(LPNMTREEVIEW pnmtv)
	{
		TREEITEM *pItem = (TREEITEM *)pnmtv->itemOld.lParam;
		_SafeMemFree(pItem->DisplayName);
		_SafeMemFree(pItem->Path);
		delete pItem;
		return 0;
	}

	LRESULT OnGetDispInfo(LPNMTVDISPINFO ptvdi)
	{
		TREEITEM *pItem = (TREEITEM *)ptvdi->item.lParam;

		if( ptvdi->item.mask & TVIF_IMAGE )
		{
//			ptvdi->item.iImage = ptvdi->item.iSelectedImage = pItem->...
		}

		if( ptvdi->item.mask & TVIF_TEXT )
		{
//			ptvdi->item.pszText = pItem->...
		}

		return 0;
	}

	LRESULT OnSelChanged(LPNMTREEVIEW pnmtv)
	{
		if( pnmtv->itemNew.state & TVIS_SELECTED )
		{
			TREEITEM *pItem = (TREEITEM *)pnmtv->itemNew.lParam;

			if( pItem )
			{
				TVITEM tvi = {0};

				tvi.mask = TVIF_IMAGE;
				tvi.hItem = pnmtv->itemNew.hItem;
				TreeView_GetItem(pnmtv->hdr.hwndFrom,&tvi);

				HICON hIcon;
				hIcon = ImageList_GetIcon(TreeView_GetImageList(pnmtv->hdr.hwndFrom,TVSIL_NORMAL),tvi.iImage,ILD_NORMAL);

				NotifyHost(CODE_SELECT_FOLDER,&pItem->Guid,pItem->DisplayName,pItem->Path,pItem,hIcon);
			}
		}
		return 0;
	}

	LRESULT OnKeyDown(LPNMTVKEYDOWN pnmhdr)
	{
		if( pnmhdr->wVKey == VK_RETURN )
			OnNmDblClk((LPNMHDR)pnmhdr);
		return 0;
	}

	LRESULT OnNmDblClk(LPNMHDR pnmhdr)
	{
		if( m_bPreventAction )
			return 0;

		HTREEITEM hItem = TreeView_GetSelection(pnmhdr->hwndFrom);
		
		return 0;
	}

	LRESULT OnNmSetFocus(NMHDR *pnmhdr)
	{
		SendMessage(GetParent(m_hWnd),WM_NOTIFY,0,(LPARAM)pnmhdr);
		return 0;
	}

	LRESULT OnCustomDraw(LPNMLVCUSTOMDRAW pcd)
	{
		if( pcd->nmcd.dwDrawStage == CDDS_PREPAINT )
		{
			return CDRF_NOTIFYITEMDRAW;
		}

		TREEITEM *pItem = (TREEITEM *)pcd->nmcd.lItemlParam;

		if( pItem->ItemType != ITEM_BLANK )
			return CDRF_DODEFAULT;

		if( pcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
		{
			pcd->clrTextBk = TreeView_GetBkColor(m_hWndTree);

			return CDRF_NEWFONT;
		}

		return CDRF_DODEFAULT;
	}

	LRESULT OnSetCursor(LPNMMOUSE lpnmm)
	{
		GetCursorPos(&lpnmm->pt);

		TVHITTESTINFO tvht = {0};
		tvht.pt    = lpnmm->pt;
		ScreenToClient(m_hWndTree,&tvht.pt);
		tvht.flags = TVHT_ONITEM|TVHT_ONITEMSTATEICON|TVHT_ONITEMRIGHT;
		tvht.hItem = NULL;
		HTREEITEM hItem = TreeView_HitTest(m_hWndTree,&tvht);
		TREEITEM *pItem = (TREEITEM *)TreeView_GetItemData(m_hWndTree,hItem);

		if( pItem && pItem->ItemType == ITEM_BLANK )
		{
			SetCursor( LoadCursor(NULL,IDC_ARROW) );
			return 1;
		}
		return 0;
	}

	LRESULT OnSelChanging(NMTREEVIEW *pnmtv)
	{
		TVITEMEX tiex={0};
		tiex.mask = TVIF_STATEEX;
		tiex.hItem = pnmtv->itemNew.hItem;
		TreeView_GetItem(m_hWndTree,&tiex);
		if( tiex.uStateEx & TVIS_EX_DISABLED )
		{
			if( pnmtv->action == 2 )
			{
				BOOL bPageKey = FALSE;

				BYTE keystate[256] = {0};
				GetKeyboardState(keystate);

				SetRedraw(m_hWndTree,FALSE);

				if( (keystate[VK_HOME] & 0x80) || (keystate[VK_PRIOR] & 0x80) || (keystate[VK_NEXT] & 0x80) )
					bPageKey = TRUE;

				HTREEITEM hNextItem = NULL;

				if( bPageKey )
				{
					TreeView_Select(m_hWndTree,(hNextItem = TreeView_GetNextVisible(m_hWndTree,pnmtv->itemNew.hItem)),TVGN_CARET);
				}
				else
				{
					if( GetKeyState(VK_DOWN) < 0 )
						hNextItem = TreeView_GetNextVisible(m_hWndTree,pnmtv->itemNew.hItem);
					else
						hNextItem = TreeView_GetPrevVisible(m_hWndTree,pnmtv->itemNew.hItem);

					if( hNextItem )
						TreeView_Select(m_hWndTree,hNextItem,TVGN_CARET);
				}

				if( hNextItem )
				{
					TreeView_EnsureVisible(m_hWndTree, hNextItem);
				}

				SetRedraw(m_hWndTree,TRUE);
			}
			return TRUE;
		}
		return 0;
	}

	LRESULT OnNotify(HWND,UINT,WPARAM,LPARAM lParam)
	{
		NMHDR *pnmhdr = (NMHDR *)lParam;

		switch( pnmhdr->code )
		{
			case NM_CUSTOMDRAW:
				return OnCustomDraw((LPNMLVCUSTOMDRAW)pnmhdr);
			case NM_SETCURSOR:
				return OnSetCursor((LPNMMOUSE)pnmhdr);
			case TVN_GETDISPINFO:
				return OnGetDispInfo((LPNMTVDISPINFO)pnmhdr);
			case TVN_DELETEITEM:
				return OnDeleteItem((LPNMTREEVIEW)pnmhdr);
			case TVN_SELCHANGING:
				return OnSelChanging((LPNMTREEVIEW)pnmhdr);
			case TVN_SELCHANGED:
				return OnSelChanged((LPNMTREEVIEW)pnmhdr);
			case TVN_KEYDOWN:
				return OnKeyDown((LPNMTVKEYDOWN)pnmhdr);
			case NM_DBLCLK:
				return OnNmDblClk((LPNMHDR)pnmhdr);
			case NM_SETFOCUS:
				return OnNmSetFocus((LPNMHDR)pnmhdr);
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
		    case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
	        case WM_DESTROY:
				return OnDestroy(hWnd,uMsg,wParam,lParam);
			case WM_SETFOCUS:
				return OnSetFocus(hWnd,uMsg,wParam,lParam);
			default:
				return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}
		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  ShellFolderTree_CreateWindow()
//
//----------------------------------------------------------------------------
HRESULT ShellFolderTree_CreateWindow(HWND hWnd,HWND *phWnd)
{
    HINSTANCE hInstance;
    hInstance = GETINSTANCE(hWnd);

	CShellFolderTree *pWnd = new CShellFolderTree;

	CShellFolderTree::RegisterClass(hInstance);

	HWND hwnd = pWnd->Create(hWnd,0,NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);

	*phWnd = hwnd;

	return S_OK;
}

//----------------------------------------------------------------------------
//
//  ShellFolderTree_InitData()
//
//----------------------------------------------------------------------------
HRESULT ShellFolderTree_InitData(HWND hWnd,HWND hWndNotify)
{
	CShellFolderTree *pObj = (CShellFolderTree *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		pObj->SetNotifyWnd(hWndNotify);
	}
	return S_OK;
}

//----------------------------------------------------------------------------
//
//  ShellFolderTree_InitLayout()
//
//----------------------------------------------------------------------------
HRESULT ShellFolderTree_InitLayout(HWND hWnd,RECT *prc)
{
	return S_OK;
}

//----------------------------------------------------------------------------
//
//  ShellFolderTree_SetNotifyWnd()
//
//----------------------------------------------------------------------------
HRESULT ShellFolderTree_SetNotifyWnd(HWND hWnd,HWND hWndNotify)
{
	CShellFolderTree *pObj = (CShellFolderTree *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		pObj->SetNotifyWnd(hWndNotify);
	}
	return S_OK;
}

//----------------------------------------------------------------------------
//
//  ShellFolderTree_GetNotifyWnd()
//
//----------------------------------------------------------------------------
HRESULT ShellFolderTree_GetNotifyWnd(HWND hWnd,HWND *phWnd)
{
	CShellFolderTree *pObj = (CShellFolderTree *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		*phWnd = pObj->GetNotifyWnd();
	}
	return S_OK;
}

//----------------------------------------------------------------------------
//
//  ShellFolderTree_FillItems()
//
//----------------------------------------------------------------------------
HRESULT ShellFolderTree_FillItems(HWND hWnd,PCWSTR Reserved)
{
	CShellFolderTree *pObj = (CShellFolderTree *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		return HRESULT_FROM_WIN32( pObj->UpdateView(Reserved) );
	}
	return E_FAIL;
}

//----------------------------------------------------------------------------
//
//  ShellFolderTree_SelectFolder()
//
//----------------------------------------------------------------------------
HRESULT ShellFolderTree_SelectFolder(HWND hWnd, PCWSTR Reserved1, UINT Reserved2 )
{
	CShellFolderTree *pObj = (CShellFolderTree *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		pObj->SelectFolder(Reserved1);
	}
	return E_FAIL;
}
