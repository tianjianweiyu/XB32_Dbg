// GoToAsmDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "GoToAsmDlg.h"
#include "afxdialogex.h"


extern DWORD g_SendAddress_Asm;
extern DWORD g_RecvAddress_Asm;
extern BOOL g_Ok_Asm;

// GoToAsmDlg 对话框

IMPLEMENT_DYNAMIC(GoToAsmDlg, CDialogEx)

GoToAsmDlg::GoToAsmDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GOTOASM_DIALOG, pParent)
{

}

GoToAsmDlg::~GoToAsmDlg()
{
}

void GoToAsmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit_Address);
}


BEGIN_MESSAGE_MAP(GoToAsmDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &GoToAsmDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// GoToAsmDlg 消息处理程序


BOOL GoToAsmDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	g_RecvAddress_Asm = 0;
	CString nTemp;
	nTemp.Format(TEXT("%08X"), g_SendAddress_Asm);
	m_Edit_Address.SetWindowText(nTemp);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//截获回车键消息，使按下回车键与点击确定按钮功能一样
BOOL GoToAsmDlg::PreTranslateMessage(MSG* pMsg)
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


void GoToAsmDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

	//获取编辑框地址
	CString szAddress;
	m_Edit_Address.GetWindowText(szAddress);
	_stscanf_s(szAddress, _T("%X"), &g_RecvAddress_Asm);

	if (!g_RecvAddress_Asm)return;

	//设置确定按钮按下标志
	g_Ok_Asm = TRUE;

	//关闭对话框
	EndDialog(0);
}
