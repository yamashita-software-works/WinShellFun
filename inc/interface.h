#pragma once

interface __declspec(novtable) IViewBaseWindow
{
	virtual HWND GetHWND() const = 0;
	virtual HRESULT Create(HWND hWndParent,HWND *phWnd=NULL) = 0;
	virtual HRESULT Destroy() = 0;
	virtual HRESULT InitData(PVOID,LPARAM) = 0;
	virtual HRESULT InitLayout(const RECT *prc) = 0;
	virtual HRESULT SelectView(SELECT_ITEM *Path) = 0;
	virtual HRESULT UpdateData(SELECT_ITEM *Sel) = 0;
	virtual HRESULT QueryCmdState(UINT CmdId,UINT *State) = 0;
	virtual HRESULT InvokeCommand(UINT CmdId) = 0;
	virtual HRESULT GetState(int,ULONG *pul) = 0;
	virtual HRESULT SetViewData(SELECT_ITEM *Sel) = 0;
};

interface __declspec(novtable) IItemTree
{
	virtual HWND GetHWND() const = 0;
	virtual HRESULT Create(HWND hWndParent,HWND *phWnd=NULL) = 0;
	virtual HRESULT Destroy() = 0;
	virtual HRESULT InitLayout(const RECT *prc) = 0;
	virtual HRESULT FillTreeItems(UINT,PVOID,PCWSTR pszIniFilePath) = 0;
	virtual HWND SetNotifyWnd(HWND hwnd) = 0;
	virtual HWND GetNotifyWnd() = 0;
};

interface __declspec(novtable) IFolderTree
{
	virtual HWND GetHWND() const = 0;
	virtual HRESULT Create(HWND hWndParent,HWND *phWnd=NULL) = 0;
};

interface __declspec(novtable) IDirBookmarkReader
{
	virtual ULONG __stdcall AddRef() = 0;
	virtual ULONG __stdcall Release() = 0;
	virtual HRESULT Load(PCWSTR pszFileName) = 0;
	virtual HRESULT GetItemCount(ULONG *pulCount) = 0;
	virtual HRESULT get_Path(ULONG ulIndex,PCWSTR *pszPath) const = 0;
	virtual HRESULT get_Name(ULONG ulIndex,PCWSTR *pszName) const = 0;
	virtual HRESULT get_Signature(ULONG ulIndex,PULONGLONG pulSignature) const = 0;
};
