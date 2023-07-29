// 2023.02.18
#include "stdafx.h"
#include "column.h"
#include "dsarray.h"

static COLUMN columns[] = {
	{ COLUMN_Name,           L"Name",                  0, 280, LVCFMT_LEFT|LVCFMT_SPLITBUTTON },
	{ COLUMN_ShellItemAttributes, L"Attributes",        16, 100, LVCFMT_LEFT },
	{ COLUMN_ShellItemParseName,  L"Parse Name",        17, 360, LVCFMT_LEFT },
};

int InitColumns()
{
	return 0;
}

int CleanColumns()
{
	return 0;
}

int GetDefaultColumnCount()
{
	return ARRAYSIZE(columns);
}

const COLUMN *GetDefaultColumnItem(int index)
{
	if( 0 <= index && index < ARRAYSIZE(columns) )
		return &columns[index];
	return NULL;
}

static int findColumnItem(UINT id)
{
	for(int i = 0; i < ARRAYSIZE(columns); i++)
	{
		if(columns[i].id == id)
		{
			return i;
		}
	}
	return -1;
}

const COLUMN *GetDefaultColumnItemFromId(UINT id)
{
	int index = findColumnItem(id);
	if( index != -1 )
	{
		return &columns[index];
	}
	return NULL;
}

int CALLBACK cb(void *p,void *pData)
{
	COLUMN *x = (COLUMN*)p;
	return 0;
}

/*++
DSArray<COLUMN> *parse(wchar_t *string)
{
	wchar_t seps[] = L",";
	wchar_t *token = NULL;
	wchar_t *next_token = NULL;

	DSArray<COLUMN> *pdsa = new DSArray<COLUMN>;

	pdsa->Create();

	token = wcstok_s(string, seps, &next_token);

	while( token != NULL )
	{
		UINT id = (UINT)_wtol(token);

		int index = findColumnItem(id);
		if( index != -1 )
		{
			COLUMN col = {0};
			const COLUMN *p = GetDefaultColumnItem(index);

			ASSERT(p != NULL);

			col.id = p->id;
			col.cx = p->cx;
			col.fmt = p->fmt;
			col.iOrder = p->iOrder;
			col.Name = _MemAllocString(p->Name);

			pdsa->Add(&col);
		}
		token = wcstok_s(NULL, seps, &next_token);
	}

	return pdsa;
}
--*/

/*++

[ColumnLayout]
Name=256,
Last=



--*/
typedef struct _COLUMN_NAME
{
	UINT id;
	PCWSTR Name;
	UINT dummy;
} COLUMN_NAME;

static COLUMN_NAME column_name_map[] = {
	{ COLUMN_Name,                L"Name",               0},
	{ COLUMN_ShellItemAttributes, L"Attributes",         0},
	{ COLUMN_ShellItemParseName,  L"ParseName",          0},
};

UINT NameToId(PCWSTR pszName)
{
	for(int i = 0; i < ARRAYSIZE(column_name_map); i++)
	{
		if( _wcsicmp(pszName,column_name_map[i].Name) == 0 )
			return column_name_map[i].id;
	}
	return 0;
}

BOOL PaeseLine(PWSTR pszLine,COLUMN *pcol)
{
	wchar_t *delim = L",";
	wchar_t *context;
	wchar_t *tok;
	wchar_t *sep;
	UINT id = 0;

	PWSTR pLine = _MemAllocString(pszLine);

	sep = wcschr(pLine,L'=');
	if( sep )
	{
		*sep = L'\0';

		StrTrim(pLine,L" \t");

		id = NameToId(pLine);

		if( id != 0 )
		{
			const COLUMN *pdef = GetDefaultColumnItemFromId(id);

			pcol->id     = pdef->id;
			pcol->Name   = pdef->Name;
			pcol->cx     = pdef->cx;
			pcol->fmt    = pdef->fmt;
			pcol->iOrder = pdef->iOrder;

			// =width[,reserved]
			tok = wcstok_s(++sep,delim,&context);

			if( tok != NULL )
			{
				pcol->cx = wcstoul(tok,NULL,10);

				while( tok )
				{	
					tok = wcstok_s(NULL,delim,&context); // reserved
				}
			}
		}
	}

	_MemFree(pLine);

	return (id != 0);
}

DSArray<COLUMN> *GetColumnLayout(PCWSTR pszIniFile)
{
	PCWSTR pszSectionName = L"ColumnLayout";

	DWORD cch = 32767;
	PWSTR buf = new WCHAR[cch];
	DWORD ret;

	if( pszIniFile && PathFileExists(pszIniFile) )
	{
		ret = GetPrivateProfileSection(pszSectionName,buf,cch,pszIniFile);
		if( ret == 0 )
		{
			delete[] buf;
			return NULL;
		}
	}
	else
	{
		WCHAR szDefault[] = 
			L"Name=180\0"
			L"Attributes=120\0"
			L"ParseName=460\0";

		memcpy(buf,szDefault,sizeof(szDefault));
	}

	DSArray<COLUMN> *pdsa = new DSArray<COLUMN>;

	pdsa->Create();

	PWSTR p = buf;
	COLUMN col = {0};
	int iOrder = 0;
	while( *p )
	{
		if( PaeseLine(p,&col) )
		{
			col.iOrder = iOrder++;
			pdsa->Add(&col);
		}

		p += (wcslen(p) + 1);
	}

	delete[] buf;

	return pdsa;
}

int LoadUserDefinitionColumnTable(COLUMN_TABLE **pColTblPtr,PCWSTR pszIniFile)
{
	int cItems = 0;

	*pColTblPtr = NULL;

	DSArray<COLUMN> *pc = GetColumnLayout(pszIniFile);

	if( pc )
	{
		cItems = pc->GetCount();

		if( cItems > 0 )
		{
			COLUMN_TABLE *pcoltbl = (COLUMN_TABLE *)_MemAllocZero( sizeof(COLUMN_TABLE) + ((cItems - 1) * sizeof(COLUMN)) );

			pcoltbl->cItems = cItems;

			for(UINT i = 0; i < pcoltbl->cItems; i++)
			{
				pc->GetItem(i,&pcoltbl->column[i]);
			}

			*pColTblPtr = pcoltbl;
		}

		delete pc;
	}

	return cItems;
}

int FreeUserDefinitionColumnTable(COLUMN_TABLE *pColTbl)
{
	_SafeMemFree(pColTbl);
	return 0;
}
