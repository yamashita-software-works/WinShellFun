#pragma once

void InitShellImageList(HIMAGELIST& hImageList,HIMAGELIST& hImageListSmall);
int GetShellIconIndexIL(LPITEMIDLIST lpi, UINT uFlags);
HICON GetShellIconIL(LPITEMIDLIST lpi, UINT uFlags);
BOOL GetShellItemName(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi, DWORD dwFlags, LPWSTR lpFriendlyName,int cchFriendlyName);
HRESULT OpenShellItemIL(LPITEMIDLIST lpidl);
HRESULT OpenShellItemPath(PCWSTR psz);

