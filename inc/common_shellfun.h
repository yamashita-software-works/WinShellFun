#pragma once

#define FSM_BASE (WM_APP+0x6600)

//
// FSM_FINDITEM
//   wParam  -
//   lParam  -
//   lResult -
//
#define FSM_FINDITEM (FSM_BASE+20)

#define FIND_QUERYOPENDIALOG  0
#define FIND_OPENDIALOG       1
#define FIND_CLOSEDIALOG      2
#define FIND_SEARCH           3
#define FIND_SEARCH_NEXT      4

enum {
	VIEW_FILELIST=1,
};
