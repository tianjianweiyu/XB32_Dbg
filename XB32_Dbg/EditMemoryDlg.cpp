// EditMemoryDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "EditMemoryDlg.h"
#include "afxdialogex.h"


extern DWORD g_SendAddress_Memory;
extern CString g_SendBytes_Memory;
extern DWORD g_RecvBytes_Memory;
extern BOOL g_Ok_Memory;


// EditMemoryDlg 对话框

IMPLEMENT_DYNAMIC(EditMemoryDlg, CDialogEx)

EditMemoryDlg::EditMemoryDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDITMEM_DIALOG, pParent)
{

}

EditMemoryDlg::~EditMemoryDlg()
{
}

void EditMemoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit_Address);
	DDX_Control(pDX, IDC_EDIT2, m_Edit_Data);
}


BEGIN_MESSAGE_MAP(EditMemoryDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &EditMemoryDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// EditMemoryDlg 消息处理程序


void EditMemoryDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

	CString nBytes;
	m_Edit_Data.GetWindowText(nBytes);

	_stscanf_s(nBytes, TEXT("%xc"), &g_RecvBytes_Memory);

	g_Ok_Memory = TRUE;

	EndDialog(0);
}


BOOL EditMemoryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CString nAddress;
	nAddress.Format(TEXT("%08X"), g_SendAddress_Memory);
	m_Edit_Address.SetWindowText(nAddress);
	m_Edit_Data.SetWindowText(g_SendBytes_Memory);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//截获回车键消息，使按下回车键与点击确定按钮功能一样
BOOL EditMemoryDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	//如果按下回车键
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (pMsg->hwnd == m_Edit_Data.m_hWnd)OnBnClickedOk();
		return FALSE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
