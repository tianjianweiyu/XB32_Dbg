#pragma once


// ClistCtrlEx

class ClistCtrlEx : public CListCtrl
{
	DECLARE_DYNAMIC(ClistCtrlEx)

public:
	ClistCtrlEx();
	virtual ~ClistCtrlEx();

protected:
	DECLARE_MESSAGE_MAP()

public:

	/*!
	*  函 数 名： SetSelectionEx
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： DWORD nItem 项的行数
	*  功    能： 选中列表第一行，并将其设置为高亮
	*/
	VOID SetSelectionEx(DWORD nItem);

	/*!
	*  函 数 名： IsSelectionItem
	*  日    期： 2020/06/06
	*  返回类型： BOOL
	*  参    数： DWORD nItem 项的行数
	*  功    能： 检测指定行是否被选中，选中返回TRUE,没选中返回FALSE
	*/
	BOOL IsSelectionItem(DWORD nItem);


	/*!
	*  函 数 名： GetSelectionItem
	*  日    期： 2020/06/07
	*  返回类型： DWORD
	*  功    能： 获取选中的项的行数
	*/
	DWORD GetSelectionItem();

	/*!
	*  函 数 名： GetSelectionSubItem
	*  日    期： 2020/06/12
	*  返回类型： DWORD
	*  功    能： 获取选中项的列数
	*/
	DWORD GetSelectionSubItem();

};


