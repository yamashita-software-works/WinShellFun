#pragma once

#include "resource.h"

HWND _GetActiveView(HWND hwndMDIChildFrame=NULL);

// find.cpp
extern VOID OnEditFind(HWND hWnd,UINT CmdId);
extern BOOL IsFindEventMessage(UINT message,LPARAM lParam);
extern BOOL IsFindDialogMessage(MSG *pmsg);
extern BOOL HasFindText();
extern VOID InitFind();
