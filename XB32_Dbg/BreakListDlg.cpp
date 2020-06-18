// BreakListDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "BreakListDlg.h"
#include "afxdialogex.h"
#include "XB32_DbgDlg.h"

extern CXB32DbgDlg* g_pDlg;

// BreakListDlg 对话框

IMPLEMENT_DYNAMIC(BreakListDlg, CDialogEx)

BreakListDlg::BreakListDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BREAKLIST_DIALOG, pParent)
{

}

BreakListDlg::~BreakListDlg()
{
}

void BreakListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BREAKLIST, m_List_Break);
}


BEGIN_MESSAGE_MAP(BreakListDlg, CDialogEx)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_BREAKLIST, &BreakListDlg::OnNMDblclkListBreaklist)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_BREAKLIST, &BreakListDlg::OnNMRClickListBreaklist)
END_MESSAGE_MAP()


// BreakListDlg 消息处理程序


BOOL BreakListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CRect rect;
	m_List_Break.GetClientRect(rect);

	m_List_Break.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List_Break.InsertColumn(0, TEXT("断点地址"), 0, rect.right / 2);
	m_List_Break.InsertColumn(1, TEXT("断点类型"), 0, rect.right / 2);

	CString nTemp;
	DWORD index = 0;

	for (DWORD i = 0; i < g_pDlg->m_BreakPoint.size(); i++)
	{
		index = m_List_Break.GetItemCount();
		nTemp.Format(TEXT("%08X"), g_pDlg->m_BreakPoint[i].nAddress);
		m_List_Break.InsertItem(index, _T(""));
		m_List_Break.SetItemText(index, 0, nTemp);
		m_List_Break.SetItemText(index, 1, TEXT("软件断点"));
	}

	for (DWORD i = 0; i < _countof(g_pDlg->m_HardBreakPoint); i++)
	{
		if (g_pDlg->m_HardBreakPoint[i].nAddress)
		{
			index = m_List_Break.GetItemCount();
			nTemp.Format(TEXT("%08X"), g_pDlg->m_HardBreakPoint[i].nAddress);
			m_List_Break.InsertItem(index, _T(""));
			m_List_Break.SetItemText(index, 0, nTemp);
			m_List_Break.SetItemText(index, 1, TEXT("硬件断点"));
		}
	}

	if (g_pDlg->HaveMemoryBreakPoint())
	{
		nTemp.Format(TEXT("%08X"), g_pDlg->m_MemoryBreakPoint.nAddress);

		index = m_List_Break.GetItemCount();
		m_List_Break.InsertItem(index, _T(""));
		m_List_Break.SetItemText(index, 0, nTemp);
		m_List_Break.SetItemText(index, 1, TEXT("内存断点"));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void BreakListDlg::OnNMDblclkListBreaklist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取选定项的行数列数
	DWORD nItem = pNMItemActivate->iItem;
	DWORD nSubItem = pNMItemActivate->iSubItem;

	if (nItem == -1)return;

	CString szBreakAddress;
	DWORD dwBreakAddress;
	//获取断点地址
	szBreakAddress = m_List_Break.GetItemText(nItem, 0);
	_stscanf_s(szBreakAddress, TEXT("%X"), &dwBreakAddress);

	g_pDlg->InitAsm(dwBreakAddress);
	g_pDlg->m_list_asm.SetFocus();
	g_pDlg->m_list_asm.SetSelectionEx(0);

	EndDialog(0);
}


void BreakListDlg::OnNMRClickListBreaklist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取选定项的行数列数
	DWORD nItem = pNMItemActivate->iItem;
	DWORD nSubItem = pNMItemActivate->iSubItem;

	if (nItem == -1)return;

	CString nBreakType;
	CString szBreakAddress;
	DWORD dwBreakAddress;
	//获取断点地址
	szBreakAddress = m_List_Break.GetItemText(nItem, 0);
	_stscanf_s(szBreakAddress, TEXT("%X"), &dwBreakAddress);
	//获取断点类型
	nBreakType = m_List_Break.GetItemText(nItem, 1);

	//如果是软件断点，删除软件断点
	if (nBreakType == CString("软件断点"))
	{
		g_pDlg->DelBreakPoint(dwBreakAddress);
	}
	//如果是硬件断点，删除硬件断点
	else if (nBreakType == CString("硬件断点"))
	{
		g_pDlg->DelHardBreakPoint(dwBreakAddress);
	}
	//如果是内存断点，删除内存断点
	else if (nBreakType == CString("内存断点"))
	{
		g_pDlg->DelMemoryBreakPoint(dwBreakAddress);
	}

	//删除选中的项
	m_List_Break.DeleteItem(nItem);

}
