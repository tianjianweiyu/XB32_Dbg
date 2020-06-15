// ModuleDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "ModuleDlg.h"
#include "afxdialogex.h"
#include "XB32_DbgDlg.h"


extern DWORD g_RecvAddress_Module;
extern CXB32DbgDlg* g_pDlg;


// ModuleDlg 对话框

IMPLEMENT_DYNAMIC(ModuleDlg, CDialogEx)

ModuleDlg::ModuleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MODULE_DIALOG, pParent)
{

}

ModuleDlg::~ModuleDlg()
{
}

void ModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List_Module);
}


BEGIN_MESSAGE_MAP(ModuleDlg, CDialogEx)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &ModuleDlg::OnNMDblclkListModule)
END_MESSAGE_MAP()


// ModuleDlg 消息处理程序


BOOL ModuleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	g_RecvAddress_Module = 0;

	CRect rect;
	m_List_Module.GetClientRect(rect);

	m_List_Module.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_List_Module.InsertColumn(0, TEXT("模块名"), 0, rect.right * 2 / 11);
	m_List_Module.InsertColumn(1, TEXT("模块路径"), 0, rect.right * 5 / 11);
	m_List_Module.InsertColumn(2, TEXT("模块地址"), 0, rect.right * 2 / 11);
	m_List_Module.InsertColumn(3, TEXT("模块大小"), 0, rect.right * 2 / 11);

	//枚举被调试进程的模块
	EnumModuleList(g_pDlg->m_Pid);
	//显示模块信息到列表
	ShowModuleList();
	//ClearModuleList(nModuleList);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL ModuleDlg::EnumModuleList(DWORD nPid)
{
	//1.先给当前进程的模块拍一个快照
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, nPid);
	//判断模块快照是否创建成功
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	//2.从快照中提取出模块信息
	MODULEENTRY32 stcmod = { sizeof(MODULEENTRY32) };
	Module32First(hSnapShot, &stcmod);
	do
	{
		//将模块的信息结构体保存在模块信息结构体数组的尾部
		m_modList.push_back(stcmod);

	} while (Module32Next(hSnapShot, &stcmod));

	//关闭模块快照
	CloseHandle(hSnapShot);

	return true;
}


VOID ModuleDlg::ShowModuleList()
{

	for (auto&i : m_modList)
	{
		//获取列表行数
		DWORD index = m_List_Module.GetItemCount();
		//插入到列表底部
		CString buffer;
		m_List_Module.InsertItem(index, _T(""));
		//模块名
		m_List_Module.SetItemText(index, 0, i.szModule);
		//模块路径
		m_List_Module.SetItemText(index, 1, i.szExePath);
		//模块地址
		buffer.Format(_T("%p"), i.modBaseAddr);
		m_List_Module.SetItemText(index, 2, buffer);
		//模块大小
		buffer.Format(_T("%08X"), i.modBaseSize);
		m_List_Module.SetItemText(index, 3, buffer);
	}
}

//模块框双击左键
void ModuleDlg::OnNMDblclkListModule(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	DWORD nItem = pNMItemActivate->iItem;
	DWORD nSubItem = pNMItemActivate->iSubItem;

	if (nItem == -1)return;

	CString nAddress;
	nAddress = m_List_Module.GetItemText(nItem, 2);
	_stscanf_s(nAddress, TEXT("%X"), &g_RecvAddress_Module);

	EndDialog(0);
}
