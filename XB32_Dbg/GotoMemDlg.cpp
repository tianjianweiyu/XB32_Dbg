// GotoMemDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "GotoMemDlg.h"
#include "afxdialogex.h"


extern DWORD g_SendAddress_Memory;
extern DWORD g_RecvAddress_Memory;
extern BOOL g_Ok_Memory;

// GotoMemDlg 对话框

IMPLEMENT_DYNAMIC(GotoMemDlg, CDialogEx)

GotoMemDlg::GotoMemDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GOTOMEMORY_DIALOG, pParent)
{

}

GotoMemDlg::~GotoMemDlg()
{
}

void GotoMemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit_Address);
}


BEGIN_MESSAGE_MAP(GotoMemDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &GotoMemDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// GotoMemDlg 消息处理程序


BOOL GotoMemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	g_RecvAddress_Memory = 0;
	CString nTemp;
	nTemp.Format(TEXT("%08X"), g_SendAddress_Memory);
	m_Edit_Address.SetWindowText(nTemp);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void GotoMemDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

	//获取编辑框地址
	CString szAddress;
	m_Edit_Address.GetWindowText(szAddress);
	_stscanf_s(szAddress, _T("%X"), &g_RecvAddress_Memory);

	if (!g_RecvAddress_Memory)return;

	//设置确定按钮按下标志
	g_Ok_Memory = TRUE;

	//关闭对话框
	EndDialog(0);
}

//截获回车键消息，使按下回车键与点击确定按钮功能一样
BOOL GotoMemDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	//如果按下回车键
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (pMsg->hwnd == m_Edit_Address.m_hWnd)OnBnClickedOk();
		return FALSE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
