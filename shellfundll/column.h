#pragma once

#include "dsarray.h"
#include "multisz.h"

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
	COLUMN_Size,
	COLUMN_ShellItemAttributes,
	COLUMN_ShellItemParseName,
	COLUMN_OriginalFilePath,
	COLUMN_TrushFileName,
	COLUMN_TrushFilePath,
	COLUMN_DeletedTime,
	COLUMN_OriginalLastWriteDate,
	COLUMN_MaxItem,
	COLUMN_MaxCount=COLUMN_MaxItem,
};

BOOL SaveColumnTable(COLUMN_TABLE *pColTblPtr,PCWSTR,PCWSTR);
BOOL SaveColumns(HWND hWndList,LPCWSTR SectionName);

class CColumnList
{
	COLUMN *m_columns;
	int     m_column_count;
	PWSTR   m_inifile_path;

public:
	CColumnList()
	{
		m_columns = NULL;
		m_column_count = 0;
		m_inifile_path = NULL;
	}

	~CColumnList()
	{
		_SafeMemFree(m_inifile_path);
	}

	void SetDefaultColumns(COLUMN *colmums,int column_count)
	{
		m_columns = colmums;
		m_column_count = column_count;
	}

	int GetDefaultColumnCount()
	{
		return m_column_count;
	}

	int LoadUserDefinitionColumnTable(COLUMN_TABLE **pColTblPtr,LPCWSTR pszSectionName);
	int FreeUserDefinitionColumnTable(COLUMN_TABLE *pColTbl);
	const COLUMN *GetDefaultColumnItemFromId(UINT id);
	const COLUMN *GetDefaultColumnItem(int index);
	BOOL SetIniFilePath(PCWSTR Path);
private:
	int findColumnItem(UINT id);
	BOOL PaeseLine(PWSTR pszLine,COLUMN *pcol);
	DSArray<COLUMN> *GetColumnLayout(PCWSTR pszSectionName);
};
