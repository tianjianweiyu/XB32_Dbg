// ConditionBreakDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "ConditionBreakDlg.h"
#include "afxdialogex.h"
#include "MyHead.h"

extern BOOL g_OK_ConditionBreakPoint;
extern CONDITION g_RecvCondition;

// ConditionBreakDlg 对话框

IMPLEMENT_DYNAMIC(ConditionBreakDlg, CDialogEx)

ConditionBreakDlg::ConditionBreakDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CONDITIONBREAK_DIALOG, pParent)
{

}

ConditionBreakDlg::~ConditionBreakDlg()
{
}

void ConditionBreakDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ConditionBreakDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &ConditionBreakDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &ConditionBreakDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// ConditionBreakDlg 消息处理程序

//确定按钮
void ConditionBreakDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString szValue;
	DWORD dwValue;
	CButton* pBtn;
	CEdit* pEdit;
	UINT ButtonId[8] = { IDC_CHECK1,IDC_CHECK2,IDC_CHECK3,IDC_CHECK4,IDC_CHECK5,IDC_CHECK6,IDC_CHECK7,IDC_CHECK8 };
	UINT EditId[8] = { IDC_EDIT1,IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5,IDC_EDIT6, IDC_EDIT7, IDC_EDIT8 };
	DWORD oneGroupsize = sizeof(BOOL) + sizeof(DWORD);
	for (int index = 0; index < 8; index++)
	{
		//获取
		pBtn = (CButton*)GetDlgItem(ButtonId[index]);
		pEdit = (CEdit*)GetDlgItem(EditId[index]);

		if (pBtn->GetCheck())
		{
			pEdit->GetWindowText(szValue);
			if (szValue != "")
			{
				_stscanf_s(szValue, TEXT("%x"), &dwValue);

				*(BOOL*)((DWORD)&g_RecvCondition + oneGroupsize * index) = TRUE;
				*(DWORD*)((DWORD)&g_RecvCondition +sizeof(BOOL)+oneGroupsize*index)= dwValue;
			}
		}
	}

	g_OK_ConditionBreakPoint = TRUE;
	EndDialog(0);

}

//取消按钮
void ConditionBreakDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

	g_OK_ConditionBreakPoint = FALSE;
	EndDialog(0);
}
