//
//  folderhelp.h
//
//  2023.07.06
//
typedef struct _FOLDER_GUID
{
	GUID Guid;
} FOLDER_GUID;

typedef struct _FOLDER_GUID_TABLE
{
	ULONG cItems;
	FOLDER_GUID Folder[1];
} FOLDER_GUID_TABLE;

int LoadFolderGuids(FOLDER_GUID_TABLE **pTblPtr,PCWSTR pszIniFile);
void FreeFolderGuids(FOLDER_GUID_TABLE *pTblPtr);

BOOL GetShellFolderName(GUID *pGuid,PWSTR pszName,int cchName,int *piImageIndex,HICON *phIcon);
BOOL GetShellFolderNameFromPath(PCWSTR pszPath,PWSTR pszName,int cchName,int *piImageIndex,HICON *phIcon);

//
// God Mode helper
//
EXTERN_C const GUID FOLDERID_GodMode;
const GUID GetGotModeGuid();
const PWSTR GetGotModeGuidString();

EXTERN_C
HRESULT
WINAPI
MakeTempGodModeFolder(
	PWSTR pszPath,
	int cchPath
	);

HRESULT
WINAPI
EnumShellItems(
	const GUID *pGuid,
	PCWSTR pszPath,
	PtrArray<CShellItem>* pa
	);
