#pragma once

interface IFileViewBaseWindow
{
	virtual HWND GetHWND() const = 0;
	virtual HRESULT Create(HWND hWnd,HWND *phWndFileList=NULL) = 0;
	virtual HRESULT Destroy() = 0;
	virtual HRESULT InitData() = 0;
	virtual HRESULT InitLayout(const RECT *prc) = 0;
	virtual HRESULT SelectData(SELECT_ITEM *Path) = 0;
	virtual HRESULT QueryCmdState(UINT CmdId,UINT *State) = 0;
	virtual HRESULT InvokeCommand(UINT CmdId) = 0;
};

HRESULT ShellFolderView_CreateObject(HINSTANCE hInstance,IFileViewBaseWindow **pObject);
