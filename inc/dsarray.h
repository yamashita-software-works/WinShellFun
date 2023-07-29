#pragma once

template<class T>
class DSArray
{
	HDSA    m_hdsa;

public:
	DSArray()
	{
		m_hdsa = NULL;
	}

	~DSArray()
	{
		Destroy();
	}

	BOOL Create(int cGrow=1)
	{
		m_hdsa = DSA_Create(sizeof(T),cGrow);
		return m_hdsa ? TRUE : FALSE;
	}

	BOOL Destroy()
	{
		if( DSA_Destroy(m_hdsa) )
		{
			m_hdsa = NULL;
			return TRUE;
		}
		return FALSE;
	}

	VOID DestroyCallback(PFNDAENUMCALLBACK pfn,PVOID pd=NULL)
	{
		DSA_DestroyCallback(m_hdsa,pfn,pd);
	}

	INT GetCount()
	{
		return DSA_GetItemCount(m_hdsa);
	}

	INT Add(T* pitem)
	{
		return DSA_InsertItem(m_hdsa,DSA_APPEND,pitem);
	}

	BOOL GetItem(int index,T *p)
	{
		return DSA_GetItem(m_hdsa,index,p);
	}

// DSA_EnumCallback
/*
	T** GetData()
	{
		return (T**)DSA_GetPtrPtr(m_hdsa);
	}

	INT Add(T* pitem)
	{
		return DSA_AppendPtr(m_hdsa,pitem);
	}

	INT Insert(INT iIndex,T* ptr)
	{
		return DSA_InsertPtr(m_hdsa,iIndex,ptr);
	}

	T* Get(INT iIndex)
	{
		return (T*)DSA_GetPtr(m_hdsa,iIndex);
	}

	INT GetIndex(const T *ptr)
	{
		return DSA_GetPtrIndex(m_hdsa,ptr);
	}
*/
	BOOL DeleteAll()
	{
		return DSA_DeleteAllItems(m_hdsa);
	}

	BOOL Delete(int iIndex)
	{
		return DSA_DeleteItem(m_hdsa,iIndex);
    }
};
