//
//  miscshell.h
//
//  2023.06.28
// 
#include "stdafx.h"
#include "miscshell.h"

void InitShellImageList(HIMAGELIST& hImageList,HIMAGELIST& hImageListSmall)
{
	// Get Desktop folder
	LPITEMIDLIST pidl;
	HRESULT hr = SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP, &pidl);

	if( SUCCEEDED(hr) )
	{
		// Get system image lists
		SHFILEINFO sfi = { 0 };
		hImageList = (HIMAGELIST)SHGetFileInfo((LPWSTR)pidl, 0, &sfi, sizeof(sfi),SHGFI_PIDL|SHGFI_SYSICONINDEX|SHGFI_ICON);

		memset(&sfi, 0, sizeof(SHFILEINFO));
		hImageListSmall = (HIMAGELIST)SHGetFileInfo((LPWSTR)pidl, 0, &sfi, sizeof(sfi),SHGFI_PIDL|SHGFI_SYSICONINDEX|SHGFI_SMALLICON);

		CoTaskMemFree(pidl);
	}
}

int GetShellIconIndexIL(LPITEMIDLIST lpi, UINT uFlags)
{
	SHFILEINFO sfi = { 0 };
	uFlags |= (SHGFI_PIDL|SHGFI_SYSICONINDEX);
	DWORD_PTR dwRet = ::SHGetFileInfo((LPCTSTR)lpi, 0, &sfi, sizeof(SHFILEINFO), uFlags);
	if( sfi.hIcon )
		DestroyIcon(sfi.hIcon);
	return (dwRet != 0) ? sfi.iIcon : I_IMAGENONE;
}

HICON GetShellIconIL(LPITEMIDLIST lpi, UINT uFlags)
{
	SHFILEINFO sfi = { 0 };
	uFlags |= (SHGFI_PIDL|SHGFI_ICON);
	DWORD_PTR dwRet = ::SHGetFileInfo((LPCTSTR)lpi, 0, &sfi, sizeof(SHFILEINFO), uFlags);
	return (dwRet != 0) ? sfi.hIcon : NULL;
}

BOOL GetShellItemName(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi, DWORD dwFlags, LPWSTR lpFriendlyName,int cchFriendlyName)
{
	BOOL bSuccess = TRUE;
	STRRET str = { STRRET_CSTR };

	if (lpsf->GetDisplayNameOf(lpi, dwFlags, &str) == NOERROR)
	{
		switch (str.uType)
		{
		case STRRET_WSTR:
			if( str.pOleStr )
				lstrcpy(lpFriendlyName, str.pOleStr);
			else
				*lpFriendlyName = L'\0';
			CoTaskMemFree(str.pOleStr);
			break;
		case STRRET_OFFSET:
			lstrcpy(lpFriendlyName, (LPTSTR)lpi + str.uOffset);
			break;
		case STRRET_CSTR:
			MultiByteToWideChar(CP_ACP,0,str.cStr,-1,lpFriendlyName,cchFriendlyName);
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

HRESULT OpenShellItemPath(PCWSTR psz)
{
	HRESULT hr;

	SHELLEXECUTEINFO sei = {0};

	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_DEFAULT;
	sei.lpVerb = L"open";
	sei.lpFile = psz;

	ShellExecuteEx( &sei );

	hr = HRESULT_FROM_WIN32( GetLastError() );

	return hr;
}

HRESULT OpenShellItemIL(LPITEMIDLIST pidl)
{
	HRESULT hr;

	SHELLEXECUTEINFO sei = {0};

	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_INVOKEIDLIST;
	sei.lpVerb = L"open";
	sei.nShow = SW_SHOWNORMAL;
	sei.lpIDList = pidl;
	sei.hwnd = GetActiveWindow();

	ShellExecuteEx( &sei );

	hr = HRESULT_FROM_WIN32( GetLastError() );

	return hr;
}
