// ClistCtrlEx.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "ClistCtrlEx.h"


// ClistCtrlEx

IMPLEMENT_DYNAMIC(ClistCtrlEx, CListCtrl)

ClistCtrlEx::ClistCtrlEx()
{

}

ClistCtrlEx::~ClistCtrlEx()
{
}




BEGIN_MESSAGE_MAP(ClistCtrlEx, CListCtrl)
END_MESSAGE_MAP()



// ClistCtrlEx 消息处理程序

VOID ClistCtrlEx::SetSelectionEx(DWORD nItem)
{
	//选中列表第一行
	SendMessage(LVM_SETSELECTIONMARK, 0, nItem);
	//设置选中行高亮
	SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	//选中时第二个参数值为1，取消选中时第二个参数为0
}

BOOL ClistCtrlEx::IsSelectionItem(DWORD nItem)
{
	return SendMessage(LVM_GETITEMSTATE, nItem, 2) == 2;
}

DWORD ClistCtrlEx::GetSelectionItem()
{
	return SendMessage(LVM_GETSELECTIONMARK, 0, 0);
}

DWORD ClistCtrlEx::GetSelectionSubItem()
{
	LVHITTESTINFO nListInfo{};
	//检索的最后一条消息的光标位置
	DWORD   nPos = GetMessagePos();
	CPoint   nPoint(LOWORD(nPos), HIWORD(nPos));
	//将屏幕坐标转换为客户端区域坐标
	ScreenToClient(&nPoint);
	nListInfo.pt = nPoint;
	nListInfo.flags = LVHT_ABOVE;
	SubItemHitTest(&nListInfo);

	return nListInfo.iSubItem;
}