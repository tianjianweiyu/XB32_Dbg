// EditRegDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "EditRegDlg.h"
#include "afxdialogex.h"

extern DWORD g_SendValue_Reg;
extern DWORD g_RecvValue_Reg;
extern BOOL g_Ok_Reg;

// EditRegDlg 对话框

IMPLEMENT_DYNAMIC(EditRegDlg, CDialogEx)

EditRegDlg::EditRegDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDITREG_DIALOG, pParent)
{

}

EditRegDlg::~EditRegDlg()
{
}

void EditRegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit_Reg);
}


BEGIN_MESSAGE_MAP(EditRegDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &EditRegDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// EditRegDlg 消息处理程序


BOOL EditRegDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	g_RecvValue_Reg = 0;
	CString nTemp;
	nTemp.Format(TEXT("%08X"), g_SendValue_Reg);
	m_Edit_Reg.SetWindowText(nTemp);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void EditRegDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

	//获取编辑框地址
	CString regValue;
	m_Edit_Reg.GetWindowText(regValue);
	_stscanf_s(regValue, _T("%X"), &g_RecvValue_Reg);

	if (!g_RecvValue_Reg)return;

	//设置确定按钮按下标志
	g_Ok_Reg = TRUE;

	//关闭对话框
	EndDialog(0);
}

//截获回车键消息，使按下回车键与点击确定按钮功能一样
BOOL EditRegDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	//如果按下回车键
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (pMsg->hwnd == m_Edit_Reg.m_hWnd)OnBnClickedOk();
		return FALSE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
