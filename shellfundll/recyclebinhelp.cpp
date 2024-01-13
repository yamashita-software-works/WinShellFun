//****************************************************************************
//
//  recyclebinhelp.cpp
//
//  Recycle bin helper functions.
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
#include <stdafx.h>
#include "recyclebinhelp.h"

HRESULT GetRecycleBinFolder( IShellFolder **RecycleBinFolder )
{
	HRESULT hr;

	IShellFolder *pDesktopFolder = NULL;
	hr = SHGetDesktopFolder(&pDesktopFolder);
	if( hr != S_OK )
		return hr;

	PIDLIST_ABSOLUTE pidlBitBucket = NULL;
	hr = SHGetSpecialFolderLocation(NULL,CSIDL_BITBUCKET,&pidlBitBucket);
	if( hr == S_OK )
	{
		hr = pDesktopFolder->BindToObject(pidlBitBucket,NULL,IID_IShellFolder,(LPVOID *)RecycleBinFolder);

		CoTaskMemFree(pidlBitBucket);
	}

	pDesktopFolder->Release();

	return hr;
}

HRESULT ExecShellFolderMenuCommand(HWND hWnd,IShellFolder *pFolder,LPITEMIDLIST pidl,LPCSTR pszRequestCommand)
{
	HRESULT hr;

	IShellFolder2 *pFolder2 = NULL;
	hr = pFolder->QueryInterface(IID_IShellFolder2, (void**)&pFolder2);
	if( hr != S_OK )
	{
		return hr;
	}

	IContextMenu *pCtxMenu = NULL;
	hr = pFolder2->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl, IID_IContextMenu, NULL, (LPVOID *)&pCtxMenu);
	if( hr != S_OK )
	{
		pFolder2->Release();
		return hr;
	}

	UINT uiID = UINT (-1);
	UINT uiCommand = 0;
	UINT uiMenuFirst = 1;
	UINT uiMenuLast = 0x00007FFF;
	HMENU hmenuCtx;
	int iMenuPos = 0;
	int cMenuItems = 0;

	hmenuCtx = CreatePopupMenu();
	hr = pCtxMenu->QueryContextMenu(hmenuCtx, 0, uiMenuFirst, uiMenuLast, CMF_NORMAL);

	if( SUCCEEDED(hr) )
	{
		CHAR verb[256];
		cMenuItems = GetMenuItemCount(hmenuCtx);
			
		for(iMenuPos = 0; iMenuPos < cMenuItems; iMenuPos++)
		{
			uiID = GetMenuItemID(hmenuCtx,iMenuPos);
				
			if( uiID != -1 && uiID != 0 )
			{
				hr = pCtxMenu->GetCommandString(uiID-1, GCS_VERBA, NULL, verb, sizeof(verb));
				if( SUCCEEDED(hr) )
				{
					if( stricmp(verb, pszRequestCommand) == 0 )
					{
						uiCommand = uiID - 1;
						break;
					}
				}
			}
		}
	}

	if( uiCommand != 0 )
	{
		CMINVOKECOMMANDINFO cmi;
		
		ZeroMemory(&cmi, sizeof(CMINVOKECOMMANDINFO));
		cmi.cbSize		 = sizeof(CMINVOKECOMMANDINFO);
		cmi.fMask		 = CMIC_MASK_FLAG_NO_UI;
		cmi.hwnd		 = hWnd;
		cmi.lpParameters = NULL;
		cmi.lpDirectory	 = NULL;
		cmi.lpVerb		 = MAKEINTRESOURCEA(uiCommand);
		cmi.nShow		 = SW_SHOWNORMAL;
		cmi.dwHotKey	 = NULL;
		cmi.hIcon		 = NULL;
		hr = pCtxMenu->InvokeCommand(&cmi);
	}
	else
	{
		hr = E_FAIL;
	}

	pCtxMenu->Release();
	pFolder2->Release();

	return hr;
}

// pcaszCommand
//  - "undelete"
//  - "delete"
HRESULT ExecRecycleBinItemCommand(HWND hWnd,LPITEMIDLIST pidl,PCSTR pcaszCommand)
{
	HRESULT hr;

	IShellFolder *pBitBucketFolder = NULL;
	hr = GetRecycleBinFolder( &pBitBucketFolder );

	if( hr == S_OK )
	{
		hr = ExecShellFolderMenuCommand(hWnd,pBitBucketFolder,pidl,pcaszCommand);

		pBitBucketFolder->Release();
	}

	return hr;
}

BOOL GetFileDateTime(PCWSTR pszPath,FILE_BASIC_INFO *pFileBasicInfo)
{
	HANDLE hFile;
	hFile = CreateFile(pszPath,FILE_READ_ATTRIBUTES,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);

	if( hFile == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	DWORD dwError = ERROR_SUCCESS;
	if( !GetFileInformationByHandleEx(hFile,FileBasicInfo,pFileBasicInfo,sizeof(FILE_BASIC_INFO)) )
	{
		dwError = GetLastError();
	}

	CloseHandle(hFile);

	SetLastError(dwError);

	return (dwError == ERROR_SUCCESS);
}

BOOL _GetItemName(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi, DWORD dwFlags, LPTSTR lpFriendlyName,DWORD cch)
{
	BOOL bSuccess = TRUE;
	STRRET str = { STRRET_CSTR };

	if( lpsf->GetDisplayNameOf(lpi, dwFlags, &str) == NOERROR )
	{
		switch (str.uType)
		{
			case STRRET_WSTR:
				StringCchCopy(lpFriendlyName,cch,str.pOleStr);
				CoTaskMemFree(str.pOleStr);
				break;
			case STRRET_OFFSET:
				StringCchCopy(lpFriendlyName,cch,(LPWSTR)lpi + str.uOffset);
				break;
			case STRRET_CSTR:
				MultiByteToWideChar(CP_ACP,0,str.cStr,-1,lpFriendlyName,cch);
				break;
			default:
				bSuccess = FALSE;
				break;
		}
	}
	else
	{
		bSuccess = FALSE;
	}

	return bSuccess;
}

static LONGLONG _ConvertDateTime(VARIANT& vt)
{
	SYSTEMTIME st;
	VariantTimeToSystemTime(vt.date,&st);

	FILETIME ft;
	SystemTimeToFileTime(&st,&ft);

	LARGE_INTEGER li;
	li.HighPart = ft.dwHighDateTime;
	li.LowPart = ft.dwLowDateTime;

	return li.QuadPart;
}

HRESULT
EnumRecycleBinItems(
	RECYCLEBINITEMLIST **RecycleBinItemsPtr
	)
{
	IShellFolder *pFolder;
	HRESULT hr;

	if( RecycleBinItemsPtr == NULL )
		return E_INVALIDARG;

	hr = SHGetDesktopFolder(&pFolder);
	if( hr != S_OK )
	{
		return hr;
	}

	WCHAR szBuffer[MAX_PATH];
    PIDLIST_ABSOLUTE pidlBitBucket = NULL;
	IShellFolder *pBitBucket = NULL;
	IEnumIDList *pEnumIDList = NULL;
	PtrArray<RECYCLEBIN_FILE_ITEM> paItems;

	try
	{
		RECYCLEBIN_FILE_ITEM *pItem;
		ULONG ulFetched = 0;
		LPITEMIDLIST pidl;

		paItems.Create(32);

		if( (hr = SHGetSpecialFolderLocation(NULL,CSIDL_BITBUCKET,&pidlBitBucket)) != S_OK )
		{
			throw hr;
		}

		if( (hr = pFolder->BindToObject(pidlBitBucket,NULL,IID_IShellFolder,(LPVOID *)&pBitBucket)) != S_OK )
		{
			throw hr;
		}

		if( (hr = pBitBucket->EnumObjects(NULL,SHCONTF_FOLDERS|SHCONTF_NONFOLDERS,&pEnumIDList)) != S_OK )
		{
			throw hr;
		}

		while( pEnumIDList->Next(1, &pidl, &ulFetched) == S_OK )
		{
			ULONG ulAttrs = SFGAO_DISPLAYATTRMASK;
			hr = pBitBucket->GetAttributesOf(1,(const struct _ITEMIDLIST **)&pidl, &ulAttrs);

			if( _GetItemName(pBitBucket, pidl, SHGDN_FORPARSING, szBuffer, MAX_PATH) )
			{
				pItem = new RECYCLEBIN_FILE_ITEM;
				if( pItem == NULL )
				{
					throw E_OUTOFMEMORY;
				}

				ZeroMemory(pItem,sizeof(RECYCLEBIN_FILE_ITEM));

				pItem->pidl = pidl;
				pItem->ManagementPath = _MemAllocString( szBuffer );

				// The attachment of extensions depends on the shell settings.
				// It cannot be acquired if [Do not display registered extensions] is set to ON.
				if( _GetItemName(pBitBucket, pidl, SHGDN_NORMAL, szBuffer, MAX_PATH) )
					pItem->OriginalPath = _MemAllocString( szBuffer );

				if( _GetItemName(pBitBucket, pidl, SHGDN_INFOLDER, szBuffer, MAX_PATH) )
					pItem->Name = _MemAllocString( szBuffer );

				if( pItem->OriginalPath == NULL || 
					pItem->Name == NULL ||
					pItem->ManagementPath == NULL )
				{
					ILFree(pidl);
					_SafeMemFree( pItem->OriginalPath );
					_SafeMemFree( pItem->Name );
					_SafeMemFree( pItem->ManagementPath );
					throw E_OUTOFMEMORY;
				}

				GetFileDateTime(pItem->ManagementPath,&pItem->TrushFileInfo);

				IShellFolder2 *pFolder2;
				if( pBitBucket->QueryInterface(IID_IShellFolder2, (void**)&pFolder2) == S_OK )
				{
					SHCOLUMNID sci = {0};
					sci.fmtid = FMTID_Storage;

					VARIANT val;

					sci.pid = PID_STG_ATTRIBUTES;
					hr = pFolder2->GetDetailsEx(pidl,&sci,&val);
					if( SUCCEEDED(hr) )
						pItem->FileAttributes = val.ulVal;

					sci.pid = PID_STG_WRITETIME;
					hr = pFolder2->GetDetailsEx(pidl,&sci,&val);
					if( SUCCEEDED(hr) )
						pItem->WriteTime = _ConvertDateTime( val );

					sci.pid = PID_STG_CREATETIME;
					hr = pFolder2->GetDetailsEx(pidl,&sci,&val);
					if( SUCCEEDED(hr) )
						pItem->CreateTime = _ConvertDateTime( val );

					sci.pid = PID_STG_SIZE;
					hr = pFolder2->GetDetailsEx(pidl,&sci,&val);
					if( SUCCEEDED(hr) )
						pItem->Size = val.llVal;

					pFolder2->Release();
				}

				paItems.Add(pItem);
			}
		}

		hr = S_OK;
	}
	catch(HRESULT hrError)
	{
		hr = hrError;
	}

	if( pidlBitBucket )
		CoTaskMemFree(pidlBitBucket);

	if( pEnumIDList )
		pEnumIDList->Release();
	if( pBitBucket )
		pBitBucket->Release();
	if( pFolder )
		pFolder->Release();

	if( hr == S_OK )
	{
		ULONG i,cItems = paItems.GetCount();

		RECYCLEBINITEMLIST *pBuffer = (RECYCLEBINITEMLIST *)_MemAlloc(sizeof(RECYCLEBINITEMLIST) 
										+ (cItems * sizeof(RECYCLEBIN_FILE_ITEM)) - sizeof(RECYCLEBIN_FILE_ITEM));

		if( pBuffer != NULL )
		{
			pBuffer->cItemCount = cItems;

			for(i = 0; i < cItems; i++)
			{
				pBuffer->Item[i] = *paItems.Get(i);

				delete paItems.Get(i);
			}

			*RecycleBinItemsPtr = pBuffer;
		}
		else
		{
			*RecycleBinItemsPtr = NULL;
			hr = E_OUTOFMEMORY;
		}
	}
	return hr;
}

HRESULT
FreeRecycleBinItems(
	RECYCLEBINITEMLIST *RecycleBinItems
	)
{
	if( RecycleBinItems == NULL )
		return E_INVALIDARG;
	ULONG i;
	for(i = 0;i < RecycleBinItems->cItemCount; i++)
	{
		ILFree( RecycleBinItems->Item[i].pidl );
		_SafeMemFree( RecycleBinItems->Item[i].Name );
		_SafeMemFree( RecycleBinItems->Item[i].OriginalPath );
		_SafeMemFree( RecycleBinItems->Item[i].ManagementPath );
	}
	_SafeMemFree(RecycleBinItems);
	return S_OK;
}