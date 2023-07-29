#pragma once

#include "mem.h"

INT
APIENTRY
_initialize_libmisc(
	void
	);

INT
APIENTRY
_uninitialize_libmisc(
	void
	);

VOID
WINAPI
_EnableVisualThemeStyle(
	HWND hWnd
	);

SIZE
WINAPI
GetLogicalPixels(
	HWND hWnd
	);

INT
WINAPI
GetLogicalPixelsX(
	HWND hWnd
	);

INT
WINAPI
GetLogicalPixelsY(
	HWND hWnd
	);

#define DPI_SIZE_CY(n)  (int)((96.0/((double)GetLogicalPixelsY(NULL))) * (double)n)

int
WINAPI
_DPI_Adjust_X(
	int x
	);

int
WINAPI
_DPI_Adjust_Y(
	int y
	);

void
WINAPI
_DPI_Adjust_XY(
	int *px,
	int *py
	);

DWORD
WINAPI
_GetDesktopWorkArea(
	HWND hwnd,
	RECT *prc
	);

BOOL
WINAPI
_SetProcessDPIAware(
	VOID
	);

BOOL
WINAPI
_MonGetMonitorRectFromWindow(
	HWND hWnd,
	RECT *prcResult,
	ULONG Flags,
	BOOLEAN bWorkspace
	);

BOOL
WINAPI
_CenterWindow(
	HWND hwndChild,
	HWND hwndParent
	);

LPTSTR
WINAPI
_CommaFormatString(
	ULONGLONG val,
	LPTSTR pszOut
	);

void
WINAPI
_GetDateTimeString(
	const ULONG64 DateTime,
	LPTSTR pszText,
	int cchTextMax
	);

LPTSTR
WINAPI
_GetDateTimeStringFromFileTime(
	const FILETIME *DateTime,
	LPTSTR pszText,
	int cchTextMax
	);

VOID
WINAPI
_GetDateTimeStringEx(
	ULONG64 DateTime,
	LPTSTR pszText,
	int cchTextMax,
	LPTSTR DateFormat,
	LPTSTR TimeFormat,
	BOOL bDisplayAsUTC
	);

VOID
WINAPI
_GetDateTimeStringEx2(
	ULONG64 DateTime,
	LPTSTR pszText,
	int cchTextMax,
	LPTSTR DateFormat,
	LPTSTR TimeFormat,
	BOOL bDisplayAsUTC,
	BOOL bMilliseconds
	);

VOID
WINAPI
_GetDateTime(
	ULONG64 DateTime,
	LPTSTR pszText,
	int cchTextMax
	);

VOID
WINAPI
_GetDateTimeFromFileTime(
	FILETIME *DateTime,
	LPTSTR pszText,
	int cchTextMax
	);

#ifdef __cplusplus

// simple heap string buffer
class CStringBuffer
{
	wchar_t *m_psz;
	int m_cch;
public:
	CStringBuffer(int cch)
	{
		m_cch = cch;
		m_psz = _MemAllocStringBuffer(m_cch);
	}

	~CStringBuffer()
	{
		_SafeMemFree(m_psz);
		m_cch = 0;
	}

	operator wchar_t*() const
	{
		return m_psz;
	}

	int GetSize() const
	{
		return m_cch * sizeof(wchar_t);
	}

	int GetLength() const
	{
		return m_cch;
	}
};

#endif

#define SEPCHAR_TAB    0x1 
#define SEPCHAR_COMMA  0x2
#define SEPCHAR_SPACE  0x0

LONG
WINAPI
SetClipboardTextFromListView(
	HWND hwndLV,
	ULONG Flags
	);

#define SCTEXT_ANSI      1
#define SCTEXT_UNICODE   0

LONG
WINAPI
SetClipboardText(
	HWND hwndCilpboardOwner,
	PVOID pszCopyString,
	ULONG CodeType
	);

#define SCTEXT_FORMAT_CSV          0x00000001
#define SCTEXT_FORMAT_TSV          0x00000002
#define SCTEXT_FORMAT_SELECTONLY   0x00004000
#define SCTEXT_FORMAT_DOUBLEQUATE  0x00008000

LONG
WINAPI
SetClipboardTextFromListViewColumn(
	HWND hwndLV,
	UINT uFormat,
	int iColumn
	);
