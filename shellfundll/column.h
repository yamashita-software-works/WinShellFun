#pragma once

typedef struct _COLUMN
{
	UINT id;
	PWSTR Name;
	int iOrder;
	int cx;
	int fmt;
} COLUMN;

typedef struct _COLUMN_TABLE
{
	ULONG cItems;
	COLUMN column[1];
} COLUMN_TABLE;

enum {
	COLUMN_None=0,
    COLUMN_Name,
	COLUMN_ShellItemAttributes,
	COLUMN_ShellItemParseName,
	COLUMN_MaxCount,
};

int InitColumns();
int CleanColumns();
int GetDefaultColumnCount();
const COLUMN *GetDefaultColumnItem(int index);
const COLUMN *GetDefaultColumnItemFromId(UINT id);
int LoadUserDefinitionColumnTable(COLUMN_TABLE **pColTblPtr,PCWSTR pszIniFile);
int FreeUserDefinitionColumnTable(COLUMN_TABLE *pColTbl);

//
// GetDisp Hanlder
//
template< class T > struct _COLUMN_HANDLER_DEF
{
	UINT colid;
	LRESULT (T::*pfn)(UINT,NMLVDISPINFO*);
};
#define COL_HANDLER_MAP_DEF(colid,pfn) { colid,pfn }
// colid: reserved
