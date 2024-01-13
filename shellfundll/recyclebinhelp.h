#pragma once

typedef struct _RECYCLEBIN_FILE_ITEM
{
	PWSTR Name;
	PWSTR OriginalPath;
	PWSTR ManagementPath;
	LONGLONG WriteTime;
	LONGLONG CreateTime;
	LONGLONG Size;
	DWORD FileAttributes;
	LPITEMIDLIST pidl;
	FILE_BASIC_INFO TrushFileInfo;
} RECYCLEBIN_FILE_ITEM;

typedef struct _RECYCLEBINITEMLIST
{
	ULONG cItemCount;
	RECYCLEBIN_FILE_ITEM Item[1];
} RECYCLEBINITEMLIST,*PRECYCLEBINITEMLIST;

HRESULT
EnumRecycleBinItems(
	RECYCLEBINITEMLIST **RecycleBinItemsPtr
	);

HRESULT
FreeRecycleBinItems(
	RECYCLEBINITEMLIST *pRecycleBinItems
	);

HRESULT
GetRecycleBinFolder(
	IShellFolder **RecycleBinFolder
	);

HRESULT
ExecRecycleBinItemCommand(
	HWND hWnd,
	LPITEMIDLIST pidl,
	PCSTR pcaszCommand
	);
