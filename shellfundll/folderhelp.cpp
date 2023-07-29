// 2023.07.06
#include "stdafx.h"
#include "folderhelp.h"
#include "dsarray.h"
#include "miscshell.h"

#include <knownfolders.h>

//
// Private FOLDERID
//

// CLSID_All_Tasks
// {ED7BA470-8E54-465E-825C-99712043E01C}
DEFINE_KNOWN_FOLDER(FOLDERID_GodMode,             0xED7BA470, 0x8E54, 0x465E, 0x82, 0x5C, 0x99, 0x71, 0x20, 0x43, 0xE0, 0x1C);

static GUID guidGodMode = {0};

const PWSTR GetGotModeGuidString()
{
	return L"{ED7BA470-8E54-465E-825C-99712043E01C}";
}

const GUID GetGotModeGuid()
{
	return FOLDERID_GodMode;
}

static BOOL PaeseLine(PWSTR pszLine,FOLDER_GUID *pcol)
{
	HRESULT hr = E_FAIL;
	wchar_t *sep;
	GUID Guid;

	PWSTR pLine = _MemAllocString(pszLine);

	sep = wcschr(pLine,L';');
	if( sep )
	{
		*sep = L'\0';
	}

	// make to "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
	{
		StrTrim(pLine,L" \t");

		if( (hr = IIDFromString(pLine,&Guid)) == S_OK )
		{
			pcol->Guid = Guid;
		}
	}

	_MemFree(pLine);

	return (hr == S_OK);
}

static DSArray<FOLDER_GUID> *ReadFolderGuids(PCWSTR pszIniFile)
{
	PCWSTR pszSectionName = L"ShellFolders";

	DWORD cch = 32767;
	PWSTR buf = new WCHAR[cch];
	DWORD ret;

	ret = GetPrivateProfileSection(pszSectionName,buf,cch,pszIniFile);
	if( ret == 0 )
	{
		delete[] buf;
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	DSArray<FOLDER_GUID> *pdsa = new DSArray<FOLDER_GUID>;

	pdsa->Create();

	PWSTR p = buf;
	FOLDER_GUID fldr = {0};
	int iOrder = 0;
	while( *p )
	{
		if( PaeseLine(p,&fldr) )
		{
			pdsa->Add(&fldr);
		}

		p += (wcslen(p) + 1);
	}

	delete[] buf;

	SetLastError(ERROR_SUCCESS);
	return pdsa;
}

int LoadFolderGuids(FOLDER_GUID_TABLE **pTblPtr,PCWSTR pszIniFile)
{
	int cItems = 0;

	*pTblPtr = NULL;

	DSArray<FOLDER_GUID> *pfa = ReadFolderGuids(pszIniFile);

	if( pfa )
	{
		cItems = pfa->GetCount();

		if( cItems > 0 )
		{
			FOLDER_GUID_TABLE *pfldrtbl = (FOLDER_GUID_TABLE *)_MemAllocZero( sizeof(FOLDER_GUID_TABLE) + ((cItems - 1) * sizeof(FOLDER_GUID)) );

			pfldrtbl->cItems = cItems;

			for(UINT i = 0; i < pfldrtbl->cItems; i++)
			{
				pfa->GetItem(i,&pfldrtbl->Folder[i]);
			}

			*pTblPtr = pfldrtbl;
		}

		delete pfa;
	}

	return cItems;
}

void FreeFolderGuids(FOLDER_GUID_TABLE *pTblPtr)
{
	_SafeMemFree(pTblPtr);
}

BOOL GetShellFolderName(GUID *pGuid,PWSTR pszName,int cchName,int *piImageIndex,HICON *phIcon)
{
	HRESULT hr;
	IShellFolder *pshf = NULL;
	IShellFolder *pFolder = NULL;
	LPITEMIDLIST pidlFolder = NULL;

	SHGetDesktopFolder( &pshf );

	hr = SHGetKnownFolderIDList(*pGuid,0,NULL,&pidlFolder);
	if( hr != S_OK )
	{
		pshf->Release();
		SetLastError(hr);
		return FALSE;
	}

	WCHAR szBuf[MAX_PATH];

	if( GetShellItemName(pshf,pidlFolder,SHGDN_NORMAL,szBuf,ARRAYSIZE(szBuf)) )
	{
		wcscpy_s(pszName,cchName,szBuf);
	}
	else
	{
		*pszName = L'\0';
	}

	if( piImageIndex )
		*piImageIndex = GetShellIconIndexIL(pidlFolder,SHGFI_SMALLICON);

	if( phIcon )
		*phIcon = GetShellIconIL(pidlFolder,SHGFI_SMALLICON|SHGFI_ICON);

	ILFree(pidlFolder);
	pshf->Release();

	return TRUE;
}

BOOL GetShellFolderNameFromPath(PCWSTR pszPath,PWSTR pszName,int cchName,int *piImageIndex,HICON *phIcon)
{
	HRESULT hr;
	IShellFolder *pshf = NULL;
	IShellFolder *pFolder = NULL;
	LPITEMIDLIST pidlFolder = NULL;

	SHGetDesktopFolder( &pshf );

	hr = SHGetKnownFolderIDList(FOLDERID_ComputerFolder,0,NULL,&pidlFolder);
	if( hr != S_OK )
	{
		pshf->Release();
		SetLastError(hr);
		return FALSE;
	}

	pidlFolder = ILCreateFromPath(pszPath);

	WCHAR szBuf[MAX_PATH];

	if( GetShellItemName(pshf,pidlFolder,SHGDN_NORMAL,szBuf,ARRAYSIZE(szBuf)) )
	{
		wcscpy_s(pszName,cchName,szBuf);
	}
	else
	{
		*pszName = L'\0';
	}

	if( piImageIndex )
		*piImageIndex = GetShellIconIndexIL(pidlFolder,SHGFI_SMALLICON);

	if( phIcon )
		*phIcon = GetShellIconIL(pidlFolder,SHGFI_SMALLICON);

	ILFree(pidlFolder);
	pshf->Release();

	return TRUE;
}

EXTERN_C
HRESULT
WINAPI
MakeTempGodModeFolder(
	PWSTR pszPath,
	int cchPath
	)
{
	HRESULT hr = E_FAIL;
	WCHAR szTemp[MAX_PATH];
	GetTempPath(MAX_PATH,szTemp);

	WCHAR szGodModeFolderName[MAX_PATH];
	StringCchPrintf(szGodModeFolderName,MAX_PATH,L".%s",GetGotModeGuidString());

	PathCombine(szTemp,szTemp,szGodModeFolderName);

	if( PathFileExists(szTemp) )
	{
		if( PathIsDirectory(szTemp) )
		{
			// alredy exists directory
			if( pszPath )
				StringCchCopy(pszPath,cchPath,szTemp);
			SetLastError(ERROR_ALREADY_EXISTS);
			hr = S_FALSE; // directory already exists
		}
		else
		{
			// exists file
			SetLastError(ERROR_ALREADY_EXISTS);
			hr = HRESULT_FROM_WIN32( ERROR_ALREADY_EXISTS );
		}
	}
	else
	{
		if( CreateDirectory(szTemp,NULL) )
		{
			if( pszPath )
				StringCchCopy(pszPath,cchPath,szTemp);
			hr = S_OK; // create
		}
		else
		{
			hr = HRESULT_FROM_WIN32( ERROR_ALREADY_EXISTS );
		}
	}

	return hr;
}

HRESULT
WINAPI
EnumShellItems(
	const GUID *pGuid,
	PCWSTR pszPath,
	PtrArray<CShellItem>* pa
	)
{
	HRESULT hr;
	LPITEMIDLIST pidlFolder = NULL;
	LPITEMIDLIST pidl;
	ULONG ulFetched = 0;
	WCHAR szName[260];
	WCHAR szPath[260];
	int iIndex = 0;

	IShellFolder *psfRoot = NULL;
	IShellFolder *psfFolder = NULL;

	if( IsEqualGUID(*pGuid,FOLDERID_Desktop) )
	{
		hr = SHGetDesktopFolder( &psfFolder );
	}
	else
	{
		SHGetDesktopFolder( &psfRoot );
		if( pszPath )
		{
			pidlFolder = ILCreateFromPath(pszPath);
		}
		else
		{
			SHGetKnownFolderIDList(*pGuid,0,NULL,&pidlFolder);
		}

		hr = psfRoot->BindToObject(pidlFolder,NULL,IID_IShellFolder,(void **)&psfFolder);
	}

	if( hr == S_OK )
	{
		IEnumIDList *pEnumIDList = NULL;
		hr = psfFolder->EnumObjects(NULL,SHCONTF_FOLDERS|SHCONTF_NONFOLDERS|SHCONTF_INCLUDEHIDDEN,&pEnumIDList);

		if( hr == S_OK )
		{
			while( pEnumIDList->Next(1, &pidl, &ulFetched) == S_OK )
			{
				ULONG ulAttrs = SHCIDS_BITMASK|SHCIDS_COLUMNMASK;
				hr = psfFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&pidl, &ulAttrs);

				szPath[0] = L'\0';
				szName[0] = L'\0';

				GetShellItemName(psfFolder,pidl,SHGDN_FORPARSING,szPath,ARRAYSIZE(szPath));
				if( !GetShellItemName(psfFolder,pidl,SHGDN_NORMAL,szName,ARRAYSIZE(szName)) )
				{
					GetShellItemName(psfFolder,pidl,SHGDN_FORPARSING|SHGDN_INFOLDER,szName,ARRAYSIZE(szName));
				}

				if( szName[0] == L'\0' )
				{
					StringCchCopy(szName,ARRAYSIZE(szName),PathFindFileName(szPath));
				}

				LPITEMIDLIST lpidlPath;
				lpidlPath = ILCombine(pidlFolder,pidl);

				CShellItem *pFI = new CShellItem(szPath,szName);

				pFI->pidl = lpidlPath;
				pFI->FileAttributes = ulAttrs;
				pFI->EnumOrderIndex = iIndex++;

				pa->Add( pFI );	

				ILFree(pidl);
			}

			pEnumIDList->Release();
		}

		psfFolder->Release();
	}

	ILFree(pidlFolder);

	if( psfRoot )
		psfRoot->Release();

	return hr;
}
