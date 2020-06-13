// CAttach.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "CAttach.h"
#include "afxdialogex.h"
#include <TlHelp32.h>
#include <Psapi.h>


// CAttach 对话框

IMPLEMENT_DYNAMIC(CAttach, CDialogEx)

CAttach::CAttach(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ATTACH, pParent)
{

}

CAttach::~CAttach()
{
}

void CAttach::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_BUTTON1, m_BtnOk);
	DDX_Control(pDX, IDC_BUTTON2, m_BtnCancel);
}


BEGIN_MESSAGE_MAP(CAttach, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CAttach::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CAttach::OnBnClickedButton2)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CAttach::OnNMDblclkList1)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CAttach 消息处理程序


BOOL CAttach::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CRect rect;
	m_list.GetClientRect(rect);

	// 设置列表的扩展风格
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERINALLVIEWS);

	m_list.InsertColumn(0, _T("PID："), 0, rect.right / 4);
	m_list.InsertColumn(1, _T("名称"), 0, rect.right / 4);
	m_list.InsertColumn(2, _T("路径"), 0, rect.right / 2);

	//获取进程快照
	HANDLE hToolhelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hToolhelp == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("获取进程快照失败"));
		return TRUE;
	}

	PROCESSENTRY32 stcProcess = { 0 };
	stcProcess.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hToolhelp, &stcProcess);
	int index = 0;
	do
	{
		CString buffer;
		m_list.InsertItem(index, _T(""));
		//进程ID
		buffer.Format(_T("%d"), stcProcess.th32ProcessID);
		m_list.SetItemText(index, 0, buffer);
		//进程名
		m_list.SetItemText(index, 1, stcProcess.szExeFile);
		//进程路径
		HANDLE	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, stcProcess.th32ProcessID);
		TCHAR	pszProcessPath[MAX_PATH + 2];
		if (GetModuleFileNameEx(hProcess, NULL, pszProcessPath, MAX_PATH + 2))
		{
			m_list.SetItemText(index, 2, pszProcessPath);
		}

		CloseHandle(hProcess);

	} while (Process32Next(hToolhelp, &stcProcess));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//附加按钮
void CAttach::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//获取选中内容所在的行
	UINT index = m_list.GetSelectionMark();
	if (index == -1)
	{
		this->MessageBox(_T("请选择一个进程"));
		return;
	}

	//获取选中的进程的ID
	TCHAR	pszPid[10];
	m_list.GetItemText(index, 0, pszPid, 10);
	DWORD nPid = _ttoi(pszPid);
	//获取进程路径
	CString strPath = m_list.GetItemText(index, 2);
	//向主窗口发送进程ID
	::SendMessage(this->GetParent()->m_hWnd, WM_AttachProcessId, nPid, (LPARAM)strPath.GetBuffer());
	SendMessage(WM_CLOSE);
}

//取消按钮
void CAttach::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	SendMessage(WM_CLOSE);
}

//响应双击鼠标左键
void CAttach::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	OnBnClickedButton1();
}


void CAttach::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	if (m_list.m_hWnd)
	{
		m_list.MoveWindow(15, 15, cx - 30, cy - 65, TRUE);
		m_BtnOk.MoveWindow(cx - 143, cy - 45, 62, 28, TRUE);
		m_BtnCancel.MoveWindow(cx - 77, cy - 45, 62, 28, TRUE);

		CRect rect;
		m_list.GetClientRect(rect);
		LVCOLUMN vol = { LVCF_WIDTH };
		vol.cx = rect.right / 4;
		m_list.SetColumn(0, &vol);
		m_list.SetColumn(1, &vol);
		vol.cx *= 2;
		m_list.SetColumn(2, &vol);

	}
}
