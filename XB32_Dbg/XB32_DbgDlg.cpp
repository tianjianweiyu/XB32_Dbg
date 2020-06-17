
// XB32_DbgDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "XB32_Dbg.h"
#include "XB32_DbgDlg.h"
#include "afxdialogex.h"
#include "CAttach.h"
#include <tchar.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include "ConditionBreakDlg.h"
#include "GotoMemDlg.h"
#include "EditAsmDlg.h"
#include "EditRegDlg.h"
#include "EditMemoryDlg.h"
#include "GoToAsmDlg.h"
#include "ModuleDlg.h"
#include "TableDlg.h"

#include <DbgHelp.h>
#pragma  comment (lib,"DbgHelp.lib")



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CXB32DbgDlg* g_pDlg = NULL;

//回调函数地址
LONG g_AsmListProc;
LONG g_MemoryListProc;

//保存反汇编列表第一项中地址
DWORD nReadAddressFirst;
//保存反汇编列表第二项中地址
DWORD nReadAddressSecond;

//用于判断用户设置条件断点后按下确认键
BOOL g_OK_ConditionBreakPoint = FALSE;
//保存条件断点的条件信息
CONDITION g_RecvCondition = {};

//用于判断用户是否确定跳转到目标地址
BOOL g_Ok_Memory = FALSE;
//当前内存框选定的内存地址
DWORD g_SendAddress_Memory = 0;
//当前内存框选定要修改的数据
CString g_SendBytes_Memory;
//修改后的数据
DWORD g_RecvBytes_Memory = 0;
//要跳转到的内存地址
DWORD g_RecvAddress_Memory = 0;

//当前反汇编框选定的地址
DWORD g_SendAddress_Asm = 0;
//当前反汇编框选定的指令
CString g_SendAsm_Asm;
//当前反汇编框选定的指令对应十六进制的长度
DWORD g_SendBytesNum_Asm = 0;
//用于接收转换后的Opcode
BYTE g_RecvBytes_Asm[256]{};
//用于接收转换后的Opcode的大小(字节)
DWORD g_RecvBytesNum_Asm = 0;
//用于判断用户是否确定修改代码
BOOL g_Ok_Asm = FALSE;
//要跳转到的内存地址
DWORD g_RecvAddress_Asm = 0;

//当前寄存器框选定的寄存器的值
DWORD g_SendValue_Reg = 0;
//修改后的寄存器的值
DWORD g_RecvValue_Reg = 0;
//用于判断用户是否确定修改寄存器的值
BOOL g_Ok_Reg = FALSE;

//用于接收选中模块的加载地址
DWORD g_RecvAddress_Module = 0;

//开启断点标志
BOOL g_OpenBreak = FALSE;

#define SETUP_STOP 0	//暂停
#define SETUP_IN 1		//步入
#define SETUP_OUT 2		//步过
#define SETUP_RUN 3		//运行

#define BREAK_SOFT 0	//软件断点
#define BREAK_HARD 1	//硬件断点
#define BREAK_MEMORY_EXCU 2	//内存执行
#define BREAK_MEMORY_RW 3	//内存读写

//断点类型，主要用于单步处理时区分
BOOL g_BreakType = BREAK_SOFT;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()

};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CXB32DbgDlg 对话框



CXB32DbgDlg::CXB32DbgDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_XB32_DBG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CXB32DbgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list_asm);
	DDX_Control(pDX, IDC_LIST2, m_list_reg);
	DDX_Control(pDX, IDC_LIST3, m_list_mem);
	DDX_Control(pDX, IDC_LIST4, m_list_stack);
}

BEGIN_MESSAGE_MAP(CXB32DbgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_32771, &CXB32DbgDlg::OnCreateProcess)
	ON_COMMAND(ID_32772, &CXB32DbgDlg::OnDebugActiveProcess)
	ON_WM_SIZE()
	ON_MESSAGE(WM_AttachProcessId, &CXB32DbgDlg::OnAttachprocessid)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, &CXB32DbgDlg::OnNMCustomdrawListAsm)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST2, &CXB32DbgDlg::OnNMCustomdrawListReg)
	ON_COMMAND(ID_32773, &CXB32DbgDlg::OnIn)
	ON_COMMAND(ID_32774, &CXB32DbgDlg::OnJump)
	ON_COMMAND(ID_32775, &CXB32DbgDlg::OnRun)
	ON_COMMAND(ID_32776, &CXB32DbgDlg::OnSoftbreak)
	ON_COMMAND(ID_32777, &CXB32DbgDlg::OnConditionsoftbreak)
	ON_COMMAND(ID_32778, &CXB32DbgDlg::OnHardbreak)
	ON_COMMAND(ID_32779, &CXB32DbgDlg::OnConditionhardbreak)
	ON_COMMAND(ID_32780, &CXB32DbgDlg::OnMemoryBreak)
	ON_COMMAND(ID_32795, &CXB32DbgDlg::OnHardwrite1)
	ON_COMMAND(ID_32796, &CXB32DbgDlg::OnHardwrite2)
	ON_COMMAND(ID_32797, &CXB32DbgDlg::OnHardwrite4)
	ON_COMMAND(ID_32798, &CXB32DbgDlg::OnHardread1)
	ON_COMMAND(ID_32799, &CXB32DbgDlg::OnHardread2)
	ON_COMMAND(ID_32800, &CXB32DbgDlg::OnHardread4)
	ON_COMMAND(ID_32786, &CXB32DbgDlg::OnMemoryWrite)
	ON_COMMAND(ID_32787, &CXB32DbgDlg::OnMemoryRead)
	ON_COMMAND(ID_32801, &CXB32DbgDlg::OnGotomemory)
	ON_NOTIFY(NM_RCLICK, IDC_LIST3, &CXB32DbgDlg::OnNMRClickListMemory)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST3, &CXB32DbgDlg::OnNMCustomdrawListMemory)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CXB32DbgDlg::OnNMRClickListAsm)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CXB32DbgDlg::OnNMDblclkListAsm)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST2, &CXB32DbgDlg::OnNMDblclkListReg)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST3, &CXB32DbgDlg::OnNMDblclkListMemory)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST4, &CXB32DbgDlg::OnNMDblclkListStack)
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CXB32DbgDlg::OnNMRClickListReg)
	ON_COMMAND(ID_32802, &CXB32DbgDlg::OnJumpToMemShow)
	ON_COMMAND(ID_32803, &CXB32DbgDlg::OnJumpToStackShow)
	ON_COMMAND(ID_32804, &CXB32DbgDlg::OnJumpToAsmShow)
	ON_COMMAND(ID_32805, &CXB32DbgDlg::OnGotoAsm)
	ON_COMMAND(ID_32807, &CXB32DbgDlg::OnShowModule)
	ON_COMMAND(ID_32808, &CXB32DbgDlg::OnShowImportExportedTable)
	ON_WM_TIMER()
	ON_COMMAND(ID_32810, &CXB32DbgDlg::OnAntiNtQueryInformationProcess)
	ON_COMMAND(ID_32809, &CXB32DbgDlg::OnAntiPeb)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()


// CXB32DbgDlg 消息处理程序

LRESULT CALLBACK ListAsmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//鼠标滚轮消息
	if (uMsg == WM_MOUSEWHEEL)
	{
		//获取高位字，大于0表示向上滑动
		SHORT zDelta = (SHORT)HIWORD(wParam);

		//向上滑动
		if (zDelta > 0)
		{
			g_pDlg->PrintOnceAsm(g_pDlg->GetBeforeAddress(nReadAddressFirst));

			if (g_pDlg->m_list_asm.GetSelectionItem() <= g_pDlg->m_Num_Asm - 2)
			{
				//获取选中的项所在的行数
				int nIndex = g_pDlg->m_list_asm.GetSelectionItem();
				//设置其下一行为选中状态
				g_pDlg->m_list_asm.SetSelectionEx(nIndex + 1);
			}
		}
		//向下滑动
		else
		{
			g_pDlg->PrintOnceAsm(nReadAddressSecond);
			if (g_pDlg->m_list_asm.GetSelectionItem() != 0)
			{
				int nIndex = g_pDlg->m_list_asm.GetSelectionItem();
				g_pDlg->m_list_asm.SetSelectionEx(nIndex - 1);
			}
		}
	}
	//键盘按下
	else if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
			//设置/取消软件断点
		case VK_F2:
			g_pDlg->OnSoftbreak();
			break;
			//软件条件断点
		case VK_F3:
			g_pDlg->OnConditionsoftbreak();
			break;
			//硬件断点
		case VK_F4:
			g_pDlg->OnHardbreak();
			break;
			//硬件条件断点
		case VK_F5:
			g_pDlg->OnConditionhardbreak();
			break;
			//内存执行断点
		case VK_F6:
			g_pDlg->OnMemoryBreak();
			break;
			//单步步入
		case VK_F7:
			g_pDlg->OnIn();
			break;
			//单步步过
		case VK_F8:
			g_pDlg->OnJump();
			break;
			//运行
		case VK_F9:
			g_pDlg->OnRun();
			break;
			//空格键-编辑汇编指令
		case VK_SPACE:
			g_pDlg->OnChangeasm();
			break;
		}
	}
	else if (uMsg == WM_CHAR)
	{
		//按下G键
		if (wParam == 0x67)
		{
			g_pDlg->OnGotoAsm();
		}
		//按下M键
		else if (wParam == 0x6d)
		{
			g_pDlg->OnShowModule();
		}
		//按下T键
		else if (wParam == 0x74)
		{
			g_pDlg->OnShowImportExportedTable();
		}
	}

	return CallWindowProc((WNDPROC)g_AsmListProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ListMemoryProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//鼠标滚轮消息
	if (uMsg == WM_MOUSEWHEEL)
	{
		//获取高位字，大于0表示向上滑动
		SHORT zDelta = (SHORT)HIWORD(wParam);

		CString nTemp;
		DWORD nAddressFirst;
		DWORD nAddressSecond;
		DWORD nSeg;
		//向上滑动
		if (zDelta > 0)
		{
			nTemp = g_pDlg->m_list_mem.GetItemText(0, g_pDlg->m_Flag_MemAddress);
			_stscanf_s(nTemp, TEXT("%x"), &nAddressFirst);
			nTemp = g_pDlg->m_list_mem.GetItemText(1, g_pDlg->m_Flag_MemAddress);
			_stscanf_s(nTemp, TEXT("%x"), &nAddressSecond);

			nSeg = nAddressSecond - nAddressFirst;
			g_pDlg->ShowMemory(nAddressFirst - nSeg);


			if (g_pDlg->m_list_mem.GetSelectionItem() <= g_pDlg->m_Num_Memory - 2)
			{
				int nIndex = g_pDlg->m_list_mem.GetSelectionItem();
				g_pDlg->m_list_mem.SetSelectionEx(nIndex + 1);
			}

		}
		//向下滑动
		else
		{
			nTemp = g_pDlg->m_list_mem.GetItemText(0, g_pDlg->m_Flag_MemAddress);
			_stscanf_s(nTemp, TEXT("%x"), &nAddressFirst);
			nTemp = g_pDlg->m_list_mem.GetItemText(1, g_pDlg->m_Flag_MemAddress);
			_stscanf_s(nTemp, TEXT("%x"), &nAddressSecond);

			nSeg = nAddressSecond - nAddressFirst;
			g_pDlg->ShowMemory(nAddressFirst + nSeg);


			if (g_pDlg->m_list_mem.GetSelectionItem() != 0)
			{
				int nIndex = g_pDlg->m_list_mem.GetSelectionItem();
				g_pDlg->m_list_mem.SetSelectionEx(nIndex - 1);
			}

		}
	}
	else if (uMsg == WM_CHAR)
	{
		//按下G键
		if (wParam == 0x67)
		{
			g_pDlg->OnGotomemory();
		}

	}
	//如果单击左键或右键
	else if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN)
	{
		//获取单击位置所在的行数/列数
		g_pDlg->m_nTempItem = g_pDlg->m_list_mem.GetSelectionItem();
		g_pDlg->m_nTempSubItem = g_pDlg->m_list_mem.GetSelectionSubItem();
		g_pDlg->SetFocus();
		g_pDlg->m_list_mem.SetFocus();
	}
	return CallWindowProc((WNDPROC)g_MemoryListProc, hwnd, uMsg, wParam, lParam);
}

VOID CXB32DbgDlg::ShowContext(CONTEXT ct)
{

	//输出寄存器信息
	CString temp_reg;
	CString oldData;
	//通用寄存器
	oldData = m_list_reg.GetItemText(0, 1);
	temp_reg.Format(_T("%08X"), ct.Eax);
	m_list_reg.SetItemText(0, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(0, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(0, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(1, 1);
	temp_reg.Format(_T("%08X"), ct.Ebx);
	m_list_reg.SetItemText(1, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(1, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(1, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(2, 1);
	temp_reg.Format(_T("%08X"), ct.Ecx);
	m_list_reg.SetItemText(2, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(2, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(2, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(3, 1);
	temp_reg.Format(_T("%08X"), ct.Edx);
	m_list_reg.SetItemText(3, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(3, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(3, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(4, 1);
	temp_reg.Format(_T("%08X"), ct.Ebp);
	m_list_reg.SetItemText(4, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(4, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(4, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(5, 1);
	temp_reg.Format(_T("%08X"), ct.Esp);
	m_list_reg.SetItemText(5, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(5, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(5, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(6, 1);
	temp_reg.Format(_T("%08X"), ct.Esi);
	m_list_reg.SetItemText(6, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(6, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(6, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(7, 1);
	temp_reg.Format(_T("%08X"), ct.Edi);
	m_list_reg.SetItemText(7, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(7, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(7, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(8, 1);
	temp_reg.Format(_T("%08X"), ct.Eip);
	m_list_reg.SetItemText(8, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(8, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(8, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	//段寄存器
	oldData = m_list_reg.GetItemText(9, 1);
	temp_reg.Format(_T("%04X"), ct.SegEs);
	m_list_reg.SetItemText(10, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(9, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(9, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(11, 1);
	temp_reg.Format(_T("%04X"), ct.SegCs);
	m_list_reg.SetItemText(11, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(11, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(11, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(12, 1);
	temp_reg.Format(_T("%04X"), ct.SegSs);
	m_list_reg.SetItemText(12, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(12, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(12, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(13, 1);
	temp_reg.Format(_T("%04X"), ct.SegDs);
	m_list_reg.SetItemText(13, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(13, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(13, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(14, 1);
	temp_reg.Format(_T("%04X"), ct.SegFs);
	m_list_reg.SetItemText(14, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(14, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(14, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(15, 1);
	temp_reg.Format(_T("%04X"), ct.SegGs);
	m_list_reg.SetItemText(15, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(15, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(15, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	//标志寄存器
	PEFLAGS eflags = (PEFLAGS)&ct.EFlags;
	oldData = m_list_reg.GetItemText(17, 1);
	temp_reg.Format(_T("%d"), eflags->CF);
	m_list_reg.SetItemText(17, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(17, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(17, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(18, 1);
	temp_reg.Format(_T("%d"), eflags->PF);
	m_list_reg.SetItemText(18, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(18, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(18, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(19, 1);
	temp_reg.Format(_T("%d"), eflags->AF);
	m_list_reg.SetItemText(19, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(19, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(19, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(20, 1);
	temp_reg.Format(_T("%d"), eflags->ZF);
	m_list_reg.SetItemText(20, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(20, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(20, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(21, 1);
	temp_reg.Format(_T("%d"), eflags->SF);
	m_list_reg.SetItemText(21, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(21, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(21, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(22, 1);
	temp_reg.Format(_T("%d"), eflags->TF);
	m_list_reg.SetItemText(22, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(22, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(22, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(23, 1);
	temp_reg.Format(_T("%d"), eflags->IF);
	m_list_reg.SetItemText(23, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(23, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(23, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(24, 1);
	temp_reg.Format(_T("%d"), eflags->DF);
	m_list_reg.SetItemText(24, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(24, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(24, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(25, 1);
	temp_reg.Format(_T("%d"), eflags->OF);
	m_list_reg.SetItemText(25, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(25, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(25, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(26, 1);
	temp_reg.Format(_T("%08X"), ct.EFlags);
	m_list_reg.SetItemText(26, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(26, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(26, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	//调试寄存器
	oldData = m_list_reg.GetItemText(28, 1);
	temp_reg.Format(_T("%08X"), ct.Dr0);
	m_list_reg.SetItemText(28, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(28, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(28, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(29, 1);
	temp_reg.Format(_T("%08X"), ct.Dr1);
	m_list_reg.SetItemText(29, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(29, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(29, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(30, 1);
	temp_reg.Format(_T("%08X"), ct.Dr2);
	m_list_reg.SetItemText(30, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(30, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(30, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(31, 1);
	temp_reg.Format(_T("%08X"), ct.Dr3);
	m_list_reg.SetItemText(31, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(31, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(31, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(32, 1);
	temp_reg.Format(_T("%08X"), ct.Dr6);
	m_list_reg.SetItemText(32, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(32, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(32, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

	oldData = m_list_reg.GetItemText(33, 1);
	temp_reg.Format(_T("%08X"), ct.Dr7);
	m_list_reg.SetItemText(33, 1, temp_reg);
	if (temp_reg != oldData)
	{
		SetItemColor(33, 1, COLOR_RED, COLOR_WHITE, RegBlock);
	}
	else
	{
		SetItemColor(33, 1, COLOR_BLACK, COLOR_WHITE, RegBlock);
	}

}

DWORD CXB32DbgDlg::DispatchException()
{
	//框架的第三层

	//第三层是专门负责修复异常的
	//如果是调试器自身设置的异常，那么可以修复，返回DBG_CONTINUE
	//如果不是调试器自身设置的异常，那么不能修复，
	//返回DBG_EXCEPTION_NOT_HANDLED

	DWORD resContinueStaus = DBG_CONTINUE;

	//是否还要寻址内存断点准确位置的标志
	static BOOL nFindMemoryBreakAddress = FALSE;

	switch (m_dbgEvent.u.Exception.ExceptionRecord.ExceptionCode)
	{
		//软件断点
	case EXCEPTION_BREAKPOINT:
	{
		// 当进程被创建的时候，操作系统会检测当前的
		// 进程是否处于被调试状态，如果被调试了，就
		// 会通过 int 3 设置一个软件断点，这个断点
		// 通常不需要处理。

		//1.获取线程环境块，将eip-1
		CONTEXT context = { CONTEXT_FULL };
		GetThreadContext(m_ThreadHandle, &context);
		context.Eip = context.Eip - 1;

		//判断是否为永久软件断点
		DWORD nBreakPoint = IsBreakPoint(context.Eip);
		//判断是否为临时软件断点
		DWORD nTempBreakPoint = IsTempBreakPoint(context.Eip);
		//判断是否为条件断点
		BOOL nIsCondition = IsConditionBreakPoint(nBreakPoint);
		//判断条件断点是否停在条件成立时
		BOOL nIsConditionSucess = IsConditionSucessBreakPoint(nBreakPoint, context, nIsCondition);

		//如果断点地址在断点列表内(永久，一次性)
		if (nBreakPoint != -1 || nTempBreakPoint != -1)
		{
			//如果是一次性断点
			if (nTempBreakPoint != -1)
				//修复断点处数据并将断点信息从数组中删除
				DelTempBreakPoint(context.Eip,nTempBreakPoint);

			//如果是条件断点成立或者是普通软件断点
			if ((nIsCondition && nIsConditionSucess) || nIsCondition == FALSE)
			{
				//显示寄存器信息
				ShowContext(context);

				//将所有永久软件断点处数据修复
				CloseAllBreakPoint();

				ShowSingleAsm(context.Eip);

				//显示栈区信息
				ShowStack(context.Esp);

				SetThreadContext(m_ThreadHandle, &context);

				//挂起线程
				SuspendThread(m_ThreadHandle);

				//标志为软件断点，便于处理单步
				g_BreakType = BREAK_SOFT;
			}
			//是条件断点，但条件不成立
			else
			{
				//将所有永久软件断点处数据修复
				CloseAllBreakPoint();

				//设置TF断点
				context.EFlags |= 0x100;
				SetThreadContext(m_ThreadHandle, &context);

				//标志为软件断点，便于处理单步
				g_BreakType = BREAK_SOFT;
				//设置为需要重新打开断点
				g_OpenBreak = TRUE;
			}
		}

		break;
	}
		//硬件断点、TF单步断点
	case EXCEPTION_SINGLE_STEP:
	{
		//1.获取线程环境块
		CONTEXT context = { CONTEXT_FULL };
		GetThreadContext(m_ThreadHandle, &context);

		//判断是不是永久硬件断点
		DWORD nHardBreak = IsHardBreakPoint(context.Eip);
		//判断是不是一次性硬件断点
		DWORD nTempHardBreak = IsTempHardBreakPoint(context.Eip);
		//判断是不是硬件读写断点
		DWORD nHardBreakEx = IsHardBreakPointEx();
		//判断是不是条件断点
		BOOL nIsCondition = IsConditionHardBreakPoint(nHardBreak);
		//判断条件断点是否停在条件成立时
		BOOL nIsConditionSucess = IsConditionSucessHardBreakPoint(nHardBreak, context, nIsCondition);

		//读写访问异常
		if (nHardBreakEx != -1)
		{
			//获取到前一条指令的地址，因为硬件读写断点触发时会停在下一条指令地址处，
			//所以前一条指令才是真正触发读写异常的指令
			DWORD nReadWriteAddress;
			nReadWriteAddress = GetBeforeAddress(context.Eip);

			AddBreakReadWrite(nReadWriteAddress, GetOnceAsm(nReadWriteAddress),1);

			//显示寄存器信息
			ShowContext(context);

			ShowSingleAsm(context.Eip);

			ShowMemory(g_RecvAddress_Memory);

			//显示栈区信息
			ShowStack(context.Esp);

			//挂起线程
			SuspendThread(m_ThreadHandle);
		}
		//如果是还原条件断点
		else if (g_OpenBreak)
		{
			//如果是软件断点
			if (g_BreakType == BREAK_SOFT)
			{
				OpenAllBreakPoint();
			}
			//如果是硬件断点
			else if (g_BreakType == BREAK_HARD)
			{
				OpenAllHardBreakPoint();
			}
			//如果是内存执行断点
			else if (g_BreakType == BREAK_MEMORY_EXCU)
			{
				//将内存执行断点重启
				VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, PAGE_NOACCESS, &m_MemoryBreakPoint.nOldPageProtect);

			}
			//如果是内存读写断点
			else if (g_BreakType == BREAK_MEMORY_RW)
			{
				//将内存执行断点重启
				VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, PAGE_NOACCESS, &m_MemoryBreakPoint.nOldPageProtect);
			}
			g_OpenBreak = FALSE;
		}
		//如果是执行断点异常或TF断点
		else
		{
			//如果是硬件断点触发
			if (nHardBreak != -1 || nTempHardBreak != -1)
			{

				//如果是一次性断点
				if (nTempHardBreak != -1)
					//修复断点处数据并将断点信息从数组中删除
					DelTempHardBreakPoint(context.Eip);

				//如果是条件断点成立或者是普通硬件断点或者是临时断点
				if ((nIsCondition && nIsConditionSucess) || nIsCondition == FALSE)
				{

					//显示寄存器信息
					ShowContext(context);

					//将所有永久硬件断点处数据修复
					CloseAllHardBreakPoint();

					//显示栈区信息
					ShowStack(context.Esp);

					ShowSingleAsm(context.Eip);

					//挂起线程
					SuspendThread(m_ThreadHandle);

					//标志为硬件断点，便于处理单步
					g_BreakType = BREAK_HARD;
				}
				//是条件断点，但条件不成立
				else
				{
					//将所有永久硬件断点处数据修复
					CloseAllHardBreakPoint();

					//设置TF断点
					context.EFlags |= 0x100;
					SetThreadContext(m_ThreadHandle, &context);

					//标志为硬件断点，便于处理单步
					g_BreakType = BREAK_HARD;
					//设置为需要重新开启断点
					g_OpenBreak = TRUE;
				}

			}
			//如果是软件断点后单步 触发 或是硬件断点后单步 触发
			else if (g_BreakType == BREAK_SOFT || g_BreakType == BREAK_HARD)
			{
				
				//显示寄存器信息
				ShowContext(context);

				//显示栈区信息
				ShowStack(context.Esp);

				ShowSingleAsm(context.Eip);

				//挂起线程
				SuspendThread(m_ThreadHandle);

			}
			//如果是内存执行断点导致单步 触发
			else if (g_BreakType == BREAK_MEMORY_EXCU)
			{

				//如果需要寻找内存断点准确位置
				if (nFindMemoryBreakAddress)
				{
					//如果找到了内存断点位置
					if (IsMemoryBreakPoint(context.Eip))
					{
						//显示寄存器信息
						ShowContext(context);

						//显示栈区信息
						ShowStack(context.Esp);

						ShowSingleAsm(context.Eip);

						//挂起线程
						SuspendThread(m_ThreadHandle);

						nFindMemoryBreakAddress = FALSE;
					}
					//如果没有找到内存断点位置
					else
					{
						nFindMemoryBreakAddress = TRUE;
						context.EFlags = context.EFlags | 0x100;
						SetThreadContext(m_ThreadHandle, &context);
					}
				}
				//如果不需要寻找内存断点准确位置
				else
				{
					//显示寄存器信息
					ShowContext(context);

					//显示栈区信息
					ShowStack(context.Esp);

					ShowSingleAsm(context.Eip);

					//挂起线程
					SuspendThread(m_ThreadHandle);
				}

			}
			//如果是内存读写断点 导致单步 触发
			else if (g_BreakType == BREAK_MEMORY_RW)
			{
				//如果需要寻找内存断点准确位置
				if (nFindMemoryBreakAddress)
				{
					//如果找到了内存断点位置
					if (IsMemoryBreakPoint(context.Eip))
					{
						//显示寄存器信息
						ShowContext(context);

						//显示栈区信息
						ShowStack(context.Esp);

						ShowSingleAsm(context.Eip);

						//挂起线程
						SuspendThread(m_ThreadHandle);

						nFindMemoryBreakAddress = FALSE;
					}
					//如果没有找到内存断点位置
					else
					{
						nFindMemoryBreakAddress = TRUE;
					}
				}
				//如果不需要寻找内存断点准确位置
				else
				{
					//显示寄存器信息
					ShowContext(context);

					//显示栈区信息
					ShowStack(context.Esp);

					ShowSingleAsm(context.Eip);

					//挂起线程
					SuspendThread(m_ThreadHandle);
				}
			}
		}
		break;
	}
		//内存访问页异常断点
	case EXCEPTION_ACCESS_VIOLATION:
	{
		//1.获取线程环境块
		CONTEXT context = { CONTEXT_FULL };
		GetThreadContext(m_ThreadHandle, &context);

		//nExceptType：8=执行异常，0=读取异常，1=写入异常
		DWORD nExceptType = m_dbgEvent.u.Exception.ExceptionRecord.ExceptionInformation[0];
		//nRwAddress：内存分页异常的地址
		DWORD nRwAddress = m_dbgEvent.u.Exception.ExceptionRecord.ExceptionInformation[1];

		//如果是执行异常
		if (nExceptType == 8 && nExceptType == GetMemoryBreakPointType())
		{

				//如果命中的不是内存断点
				if (!IsMemoryBreakPoint(context.Eip))
				{
					DWORD nOldProtect;
					//恢复页保护属性
					VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, m_MemoryBreakPoint.nOldPageProtect, &nOldProtect);

					//设置TF断点
					context.EFlags = context.EFlags | 0x100;
					SetThreadContext(m_ThreadHandle, &context);

					//设置还需要继续查找准确断点位置
					nFindMemoryBreakAddress = TRUE;

					//设置内存执行断点标志
					g_BreakType = BREAK_MEMORY_EXCU;

				}

		}
		//如果是读取,读写异常
		else if (nExceptType == 0 || nExceptType == 1)
		{
			//如果命中的不是内存断点
			if (!IsMemoryBreakPoint(nRwAddress))
			{

				DWORD nOldProtect;
				//恢复页保护属性
				VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, m_MemoryBreakPoint.nOldPageProtect, &nOldProtect);

				//设置TF断点
				context.EFlags = context.EFlags | 0x100;
				SetThreadContext(m_ThreadHandle, &context);

				//设置还需要继续查找准确断点位置
				nFindMemoryBreakAddress = TRUE;

				//设置内存读写断点标志
				g_BreakType = BREAK_MEMORY_RW;

			}
			//如果命中的是内存断点
			else
			{
				//显示寄存器信息
				ShowContext(context);

				//显示栈区信息
				ShowStack(context.Esp);

				ShowSingleAsm(context.Eip);

				DWORD nReadWriteAddress = 0;
				nReadWriteAddress = (DWORD)m_dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress;

				AddBreakReadWrite(nReadWriteAddress, GetOnceAsm(nReadWriteAddress), 2);

				//挂起线程
				SuspendThread(m_ThreadHandle);

				//设置内存读写断点标志
				g_BreakType = BREAK_MEMORY_RW;
			}

		}

		break;
	}
	default:
	{
		//没有进行处理
		resContinueStaus = DBG_EXCEPTION_NOT_HANDLED;
		break;
	}
	}

	return resContinueStaus;
}

DWORD CXB32DbgDlg::DispatchEvent()
{
	//框架的第二层
	//将调试事件主要分为两部分来处理
	//第一部分为异常调试事件，继续派发到第三层去处理
	//第二部分为除异常调试事件外的其它调试事件，本函数类处理

	DWORD resContinueStaus;

	switch (m_dbgEvent.dwDebugEventCode)
	{
	//异常调试事件
	case EXCEPTION_DEBUG_EVENT:
	{
		resContinueStaus = DispatchException();
		break;
	}
	//进程创建事件
	case CREATE_PROCESS_DEBUG_EVENT:
	{
		//载入进程的符号表
		LoadSymbol(&m_dbgEvent.u.CreateProcessInfo);

		SetTimer(TABLECLOCK, 1000, NULL);

		//反反调试
		FunDbg();

		//调用插件CreateProcessEvent接口
		m_LoadPlugin.CallCreateProcessEvent((DWORD)m_dbgEvent.dwProcessId, (DWORD)m_dbgEvent.dwThreadId);

		//如果不是附加进程，是创建进程
		if (m_nPid == 0)
		{
			//初始化汇编，内存
			InitInformationEx((DWORD)m_dbgEvent.u.CreateProcessInfo.lpBaseOfImage);

			//如果断点列表中没有oep断点，就下一个oep断点
			if (IsBreakPoint(GetOepAddressEx((DWORD)m_dbgEvent.u.CreateProcessInfo.lpBaseOfImage)) == -1)
			{
				SetBreakPoint(GetOepAddressEx((DWORD)m_dbgEvent.u.CreateProcessInfo.lpBaseOfImage), FALSE, {});
			}
		}
		resContinueStaus = DBG_CONTINUE;

		break;
	}
	case CREATE_THREAD_DEBUG_EVENT:
	{
		//调用插件CreateThreadEvent接口
		m_LoadPlugin.CallCreateThreadEvent((DWORD)m_dbgEvent.u.CreateThread.hThread, (DWORD)m_dbgEvent.u.CreateThread.lpThreadLocalBase);

		resContinueStaus = DBG_CONTINUE;
		break;
	}
	default:
	{
		resContinueStaus = DBG_CONTINUE;
		break;
	}
	}

	return resContinueStaus;
}

//线程回调函数
DWORD WINAPI DebugThread(LPVOID lpThreadParameter)
{
	CXB32DbgDlg* pDlg = (CXB32DbgDlg*)lpThreadParameter;

	//框架的第一层

	//函数主要分为3个部分： 1.等待调试事件 2.处理调试事件 
	//3.回复调试子系统

	if (pDlg->m_nPid == 0)
	{
		//以调试的方式创建进程
		if (!pDlg->Open(pDlg->m_strFilePath))
			return 0;
	}
	else
	{
		//以调试的方式附加进程
		if (!pDlg->Open(pDlg->m_nPid))
			return 0;

	}

	//用于保存处理的结果
	DWORD m_ContinueStaus;

	//通过循环不断的从调试对象中获取到调试信息
	while (WaitForDebugEvent(&pDlg->m_dbgEvent, INFINITE))
	{
		//用于保存处理的结果
		m_ContinueStaus = DBG_CONTINUE;

		//分发调试事件，处理调试事件
		m_ContinueStaus = pDlg->DispatchEvent();

		//回复调试子系统
		ContinueDebugEvent(pDlg->m_dbgEvent.dwProcessId, pDlg->m_dbgEvent.dwThreadId, m_ContinueStaus);
	}

	return 0;
}

BOOL CXB32DbgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//提权
	MyRtlAdjustPrivilege();

	g_pDlg = this;

	//加载菜单
	m_Menu_Main.LoadMenu(IDR_MENU1);
	SetMenu(&m_Menu_Main);
	m_Menu_Secondary.LoadMenu(IDR_MENU2);

	//设置插件菜单名字
	SetPluginMenu();

	//设置List控件属性
	SetListAttr();
	//设置控件钩子
	SetAllWindowsLong();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CXB32DbgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CXB32DbgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CXB32DbgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

VOID CXB32DbgDlg::MyRtlAdjustPrivilege()
{
	const unsigned int SE_DEBUG_PRIVILEGE = 0x14;
	int nAddress = 0;
	typedef int(WINAPI*RtlAdjustPrivilege)(int, bool, bool, int*);
	HMODULE module = LoadLibrary(TEXT("ntdll.dll"));
	RtlAdjustPrivilege pRt = (RtlAdjustPrivilege)GetProcAddress(module, "RtlAdjustPrivilege");
	pRt(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &nAddress);
	FreeLibrary(module);
}

//文件->打开按钮
void CXB32DbgDlg::OnCreateProcess()
{
	// TODO: 在此添加命令处理程序代码

	//文件类型过滤
	const TCHAR pszFilter[] = _T("可执行文件 (*.exe *.dll)|*.exe;*.dll|所有文件 (*.*)|*.*||");
	//第一个参数为TRUE是打开文件，为FALSE是文件另存为
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		pszFilter, this);

	TCHAR path[255];
	//获取当前用户的桌面路径
	SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, 0);
	//设置对话框默认呈现的路径
	dlg.m_ofn.lpstrInitialDir = path; 

	if (dlg.DoModal() == IDOK)
	{
		//获取选中的文件的路径
		m_strFilePath = dlg.GetPathName();
	}

	//创建线程
	CreateThread(NULL, NULL, DebugThread, (LPVOID)this, NULL, NULL);
}

//文件->附加按钮
void CXB32DbgDlg::OnDebugActiveProcess()
{
	// TODO: 在此添加命令处理程序代码
	CAttach MyAttachDlg;
	MyAttachDlg.DoModal();
}

BOOL CXB32DbgDlg::Open(const TCHAR* pszFile)
{
	//如果路径为空，返回FALSE
	if (pszFile == nullptr)
	{
		return FALSE;
	}

	STARTUPINFO stcStartupInfo = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION stcProcInfo = { 0 };		//进程信息

	//DEBUG_PROCESS表示以调试的方式打开目标进程，并且
	//当被调试创建新的进程时，同样接收新进程的调试信息
	//DEBUG_ONLY_THIS_PROCESS 只调试目标进程，不调试目标
	//进程创建的新的进程
	//CREATE_NEW_CONSOLE 表示新创建的 CUI 程序会使用一个
	//独立的控制台运行，如果不写就和调试器共用控制台

	//创建调试进程
	BOOL bRet = CreateProcess(pszFile,		//可执行模块路径
		NULL,		//命令行
		NULL,		//安全描述符
		NULL,		//线程属性是否可继承
		FALSE,		//是否从调用进程处继承句柄，FALSE是否
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,		//以调试的方式启动
		NULL,		//新进程的环境块
		NULL,		//新进程的当前工作路径(当前目录)
		&stcStartupInfo,	//指定进程的主窗口特性
		&stcProcInfo);		//接收新进程的识别信息

	if (!bRet)
	{
		MessageBox(TEXT("创建调试进程失败！"), TEXT("提示"), MB_ICONERROR);
		return FALSE;
	}

	CString winText;
	winText.Format(L"%s - %s", pszFile, L"调试进程");
	SetWindowText(winText);

	//保存进程/线程句柄,进程PID
	m_ProcessHandle = stcProcInfo.hProcess;
	m_ThreadHandle = stcProcInfo.hThread;
	m_Pid = stcProcInfo.dwProcessId;

	return TRUE;

}

BOOL CXB32DbgDlg::Open(DWORD nPid)
{
	//如果路径为空，返回FALSE
	if (nPid == 0)
	{
		return FALSE;
	}

	if (!DebugActiveProcess(m_nPid))
	{
		::MessageBox(NULL, _T("调试进程失败！"), _T("提示"), MB_ICONERROR);
	}

	CString winText;
	winText.Format(L"%s - %s", m_strFilePath, L"调试进程");
	SetWindowText(winText);

	m_ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, nPid);
	if (m_ProcessHandle == nullptr)MessageBox(TEXT("打开进程失败！"), TEXT("提示"), MB_ICONERROR);

	DWORD nTid = GetProcessThreadId(nPid);
	m_ThreadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, nTid);
	if (m_ProcessHandle == nullptr)MessageBox(TEXT("打开线程失败！"), TEXT("提示"), MB_ICONERROR);

	return TRUE;
}

DWORD CXB32DbgDlg::GetProcessThreadId(DWORD nPid)
{
	BOOL nRet;
	DWORD nIndex = 0;
	THREADENTRY32 nThread;

	nThread.dwSize = sizeof(THREADENTRY32);
	HANDLE nSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, nPid);

	nRet = Thread32First(nSnapShot, &nThread);
	while (nRet)
	{
		if (nThread.th32OwnerProcessID == nPid)
		{
			CloseHandle(nSnapShot);
			return nThread.th32ThreadID;
		}
		nRet = Thread32Next(nSnapShot, &nThread);
	}
	CloseHandle(nSnapShot);
	return NULL;
}

VOID CXB32DbgDlg::SetListAttr()
{

	//获取控件大小
	CRect rect;
	m_list_asm.GetClientRect(rect);
	//反汇编list
	m_list_asm.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list_asm.InsertColumn(m_Flag_Break, TEXT(""), 0, 18);
	m_list_asm.InsertColumn(m_Flag_Address, TEXT("地址"), 0, rect.right/6);
	m_list_asm.InsertColumn(m_Flag_Opcode, TEXT("十六进制"), 0, rect.right / 6);
	m_list_asm.InsertColumn(m_Flag_Asm, TEXT("汇编"), 0, rect.right / 3);
	m_list_asm.InsertColumn(m_Flag_Mark, TEXT("注释"), 0, rect.right / 3 - 35);

	for (DWORD i = 0; i < m_Num_Asm; i++) m_list_asm.InsertItem(i, TEXT(""));

	m_list_reg.GetClientRect(rect);
	//寄存器list
	m_list_reg.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	m_list_reg.InsertColumn(0, _T("通用寄存器"), 0, rect.right/2);
	m_list_reg.InsertColumn(1, _T("值"), 0, rect.right / 2);
	m_list_reg.InsertItem(0, _T("EAX"));
	m_list_reg.InsertItem(1, _T("EBX"));
	m_list_reg.InsertItem(2, _T("ECX"));
	m_list_reg.InsertItem(3, _T("EDX"));
	m_list_reg.InsertItem(4, _T("EBP"));
	m_list_reg.InsertItem(5, _T("ESP"));
	m_list_reg.InsertItem(6, _T("ESI"));
	m_list_reg.InsertItem(7, _T("EDI"));
	m_list_reg.InsertItem(8, _T("EIP"));
	m_list_reg.InsertItem(9, _T("段寄存器"));
	m_list_reg.InsertItem(10, _T("ES"));
	m_list_reg.InsertItem(11, _T("CS"));
	m_list_reg.InsertItem(12, _T("SS"));
	m_list_reg.InsertItem(13, _T("DS"));
	m_list_reg.InsertItem(14, _T("FS"));
	m_list_reg.InsertItem(15, _T("GS"));
	m_list_reg.InsertItem(16, _T("标志寄存器"));
	m_list_reg.InsertItem(17, _T("CF"));
	m_list_reg.InsertItem(18, _T("PF"));
	m_list_reg.InsertItem(19, _T("AF"));
	m_list_reg.InsertItem(20, _T("ZF"));
	m_list_reg.InsertItem(21, _T("SF"));
	m_list_reg.InsertItem(22, _T("TF"));
	m_list_reg.InsertItem(23, _T("IF"));
	m_list_reg.InsertItem(24, _T("DF"));
	m_list_reg.InsertItem(25, _T("OF"));
	m_list_reg.InsertItem(26, _T("Eflags"));
	m_list_reg.InsertItem(27, _T("调试寄存器"));
	m_list_reg.InsertItem(28, _T("DR0"));
	m_list_reg.InsertItem(29, _T("DR1"));
	m_list_reg.InsertItem(30, _T("DR2"));
	m_list_reg.InsertItem(31, _T("DR3"));
	m_list_reg.InsertItem(32, _T("DR6"));
	m_list_reg.InsertItem(33, _T("DR7"));

	m_list_stack.GetClientRect(rect);
	//堆栈list
	m_list_stack.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list_stack.InsertColumn(m_Flag_StackAddress, TEXT("地址"), 0, rect.right / 2);
	m_list_stack.InsertColumn(m_Flag_StackData, TEXT("数据"), 0, rect.right / 2);

	for (DWORD i = 0; i < m_Num_Stack; i++) m_list_stack.InsertItem(0, TEXT(""));

	m_list_mem.GetClientRect(rect);
	//内存数据list
	m_list_mem.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list_mem.InsertColumn(m_Flag_MemAddress, TEXT("地址"), 0, 70);
	for (DWORD i = 1; i < 17; i++)
	{
		CString nTemp;
		nTemp.Format(TEXT("%02X"), i-1);
		m_list_mem.InsertColumn(i, nTemp, 0, 33);
	}
	m_list_mem.InsertColumn(m_Flag_MemStr, TEXT("文本"), 0, 200);

	for (DWORD i = 0; i < m_Num_Memory; i++) m_list_mem.InsertItem(0, TEXT(""));
}

VOID CXB32DbgDlg::SetPluginMenu()
{
	CMenu *nMenu = m_Menu_Main.GetSubMenu(5);
	for (DWORD i = 0; i < m_LoadPlugin.m_PluginName.size(); i++)
	{
		nMenu->AppendMenuW(MF_STRING, 0x1000 + i, m_LoadPlugin.m_PluginName[i]);
	}

}

BOOL CXB32DbgDlg::LoadSymbol(CREATE_PROCESS_DEBUG_INFO* pInfo) 
{

	//初始化符号处理器
	SymInitialize(m_ProcessHandle, NULL, FALSE);

	//载入符号文件
	SymLoadModule64(m_ProcessHandle, pInfo->hFile, NULL, NULL, (DWORD64)pInfo->lpBaseOfImage, 0);

	IMAGEHLP_MODULE64 nIMAGEHLP_MODULE64{};
	nIMAGEHLP_MODULE64.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	//获取模块信息
	SymGetModuleInfo64(m_ProcessHandle, (DWORD64)pInfo->lpBaseOfImage, &nIMAGEHLP_MODULE64);

	return TRUE;
}

void CXB32DbgDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	if (m_list_asm.m_hWnd)
	{
		m_list_asm.MoveWindow(0, 0, cx * 3 / 4, cy * 3 / 5, TRUE);
		m_list_reg.MoveWindow(cx * 3 / 4, 0, cx / 4, cy * 3 / 5, TRUE);
		m_list_mem.MoveWindow(0, cy * 3 / 5, cx * 3 / 4, cy * 2 / 5, TRUE);
		m_list_stack.MoveWindow(cx * 3 / 4, cy * 3 / 5, cx / 4, cy * 2 / 5, TRUE);

		//获取控件大小
		CRect rect;
		m_list_asm.GetClientRect(rect);
		m_list_asm.SetColumnWidth(m_Flag_Break, 18);
		m_list_asm.SetColumnWidth(m_Flag_Address, rect.right / 6);
		m_list_asm.SetColumnWidth(m_Flag_Opcode, rect.right / 6);
		m_list_asm.SetColumnWidth(m_Flag_Asm, rect.right / 3);
		m_list_asm.SetColumnWidth(m_Flag_Mark, rect.right / 3 - 35);

		m_list_reg.GetClientRect(rect);
		m_list_reg.SetColumnWidth(0, rect.right / 2);
		m_list_reg.SetColumnWidth(1, rect.right / 2);

		m_list_stack.GetClientRect(rect);
		m_list_stack.SetColumnWidth(m_Flag_StackAddress, rect.right / 2);
		m_list_stack.SetColumnWidth(m_Flag_StackData, rect.right / 2);

	}
}

afx_msg LRESULT CXB32DbgDlg::OnAttachprocessid(WPARAM wParam, LPARAM lParam)
{
	//获取附加进程ID
	m_nPid = (DWORD)wParam;
	//获取附加进程的路径
	m_strFilePath = CString((PTCHAR)lParam);

	//创建线程
	CreateThread(NULL, NULL, DebugThread, (LPVOID)this, NULL, NULL);

	return 0;
}

VOID CXB32DbgDlg::InitInformationEx(DWORD nImageBase)
{
	InitAsm(GetOepAddressEx(nImageBase));
	//显示内存数据
	ShowMemory(nImageBase);
	//选中列表第一行并高亮
	m_list_asm.SetSelectionEx(0);
}

VOID CXB32DbgDlg::InitAsm(DWORD nReadAddress)
{

	BYTE nReadBuff[1024]{};
	CString nAddr;
	DISASM disAsm = { 0 };
	COLORREF nTextColor;
	COLORREF nBkColor;

	ReadMemoryBytes(nReadAddress, nReadBuff, 1024);

	disAsm.EIP = (UIntPtr)nReadBuff;
	disAsm.VirtualAddr = nReadAddress;
	disAsm.Archi = 0;
	disAsm.Options = 0x000;
	DWORD nOpcodeSize = 1024;
	INT nLen = 0;

	//清除反汇编区高亮颜色
	ClearItemColor();

	for (DWORD i = 0; i < m_Num_Asm; i++)
	{
		nLen = Disasm(&disAsm);
		if (nLen == -1)
		{
			disAsm.EIP += 1;
			disAsm.VirtualAddr += 1;
			continue;
		}

		PrintBreakFlag((DWORD)disAsm.VirtualAddr, i);
		nAddr.Format(TEXT("%08X"), disAsm.VirtualAddr);
		m_list_asm.SetItemText(i, m_Flag_Address, ShowExportName(nAddr));
		ShowOpcode((BYTE*)disAsm.EIP, nLen, i);
		m_list_asm.SetItemText(i, m_Flag_Asm, ShowAsmName(nAddr, CString(disAsm.CompleteInstr)));
	
		disAsm.EIP += nLen;
		disAsm.VirtualAddr += nLen;

		//修复int3断点处的数据
		RepairAsm(nReadAddress, (DWORD)disAsm.VirtualAddr, nReadBuff);

		//如果是高亮指令，将指令信息存入动态数组中，以便后面绘画使用
		if (IsColorWord(CString(disAsm.CompleteInstr), &nTextColor, &nBkColor))
		{
			SetItemColor(i, m_Flag_Asm, nTextColor, nBkColor,AsmBlock);
		}
	}

	_stscanf_s(m_list_asm.GetItemText(0, m_Flag_Address), TEXT("%x"), &nReadAddressFirst);
	_stscanf_s(m_list_asm.GetItemText(1, m_Flag_Address), TEXT("%x"), &nReadAddressSecond);
	//_stscanf_s(m_list_asm.GetItemText(m_Num_Asm - 1, m_Flag_Address), TEXT("%x"), &nReadAddressEnd);
}

VOID CXB32DbgDlg::ClearItemColor()
{
	m_AsmColor.clear();
	m_AsmColor.swap(vector<LISTCOLOR>());
}

DWORD CXB32DbgDlg::GetOepAddressEx(DWORD nImageBase)
{
	BYTE nProcessBuff[1024]{};

	//读取模块前1024字节的数据
	ReadMemoryBytes(nImageBase, nProcessBuff, 1024);

	PIMAGE_DOS_HEADER pDosHead = (PIMAGE_DOS_HEADER)nProcessBuff;
	PIMAGE_NT_HEADERS pNtHead = (PIMAGE_NT_HEADERS)(pDosHead->e_lfanew + nProcessBuff);

	return nImageBase + pNtHead->OptionalHeader.AddressOfEntryPoint;
}

VOID CXB32DbgDlg::ReadMemoryBytes(DWORD nAddress, LPBYTE nValue, DWORD nLen)
{
	DWORD nReadSize;
	ReadProcessMemory(m_ProcessHandle, (LPCVOID)nAddress, nValue, nLen, &nReadSize);
}

VOID CXB32DbgDlg::PrintBreakFlag(DWORD nBreakAddress, DWORD nItem)
{
	CString nAddr;
	//检测是不是软件断点
	if (IsBreakPoint(nBreakAddress) != -1)
	{
		nAddr = TEXT("●");
		SetItemColor(nItem, m_Flag_Break, COLOR_WHITE, COLOR_RED,AsmBlock);
		SetItemColor(nItem, m_Flag_Address, COLOR_WHITE, COLOR_RED,AsmBlock);
		//SetItemColor(nItem, m_Flag_Address_Show, COLOR_WHITE, COLOR_RED);
	}
	//检测是不是硬件断点
	else if (IsHardBreakPoint(nBreakAddress) != -1)
	{
		nAddr = TEXT("■");
		SetItemColor(nItem, m_Flag_Break, COLOR_WHITE, COLOR_BLACK,AsmBlock);
		SetItemColor(nItem, m_Flag_Address, COLOR_WHITE, COLOR_BLACK,AsmBlock);
		//SetItemColor(nItem, m_Flag_Address_Show, COLOR_WHITE, COLOR_BLACK);
	}
	//检测是不是内存断点
	else if (IsMemoryBreakPoint(nBreakAddress))
	{
		nAddr = TEXT("▲");
		SetItemColor(nItem, m_Flag_Break, COLOR_WHITE, COLOR_ORANGE,AsmBlock);
		SetItemColor(nItem, m_Flag_Address, COLOR_WHITE, COLOR_ORANGE,AsmBlock);
		//SetItemColor(nItem, m_Flag_Address_Show, COLOR_WHITE, COLOR_ORANGE);
	}
	else nAddr = TEXT("");
	m_list_asm.SetItemText(nItem, m_Flag_Break, nAddr);
}

DWORD CXB32DbgDlg::IsBreakPoint(DWORD nBreakAddress)
{
	for (DWORD i = 0; i < m_BreakPoint.size(); i++)
	{
		if (m_BreakPoint[i].nAddress == nBreakAddress)
		{
			return i;
		}
	}
	return -1;
}

VOID CXB32DbgDlg::SetItemColor(DWORD Item, DWORD SubItem, COLORREF TextColor, COLORREF BkColor,DWORD Flag)
{
	BOOL nHava = FALSE;
	//如果是反汇编区
	if (Flag == AsmBlock)
	{
		//如果数组存在该项，进行颜色更新
		for (auto &value : m_AsmColor)
		{
			if (value.Item == Item && value.SubItem == SubItem)
			{
				value.TextColor = TextColor;
				value.BkColor = BkColor;
				nHava = TRUE;
				break;
			}
		}

		//如果数组不存在，加入数组
		if (!nHava)m_AsmColor.push_back({ Item ,SubItem ,TextColor ,BkColor });
	}
	//如果是寄存器区
	else if (Flag == RegBlock)
	{
		//如果数组存在该项，进行颜色更新
		for (auto &value : m_RegColor)
		{
			if (value.Item == Item && value.SubItem == SubItem)
			{
				value.TextColor = TextColor;
				value.BkColor = BkColor;
				nHava = TRUE;
				break;
			}
		}

		//如果数组不存在，加入数组
		if (!nHava)m_RegColor.push_back({ Item ,SubItem ,TextColor ,BkColor });
	}

}

CString CXB32DbgDlg::ShowExportName(CString szAddress)
{
	DWORD nAddress = 0;
	DWORD nMinAddress = 0;
	DWORD nMaxAddress = 0;

	_stscanf_s(szAddress, TEXT("%x"), &nAddress);

	//循环遍历所有模块
	for (DWORD i = 0; i < m_Table.size(); i++)
	{
		//计算出模块的起始地址和结束
		nMinAddress = m_Table[i].ModuleAddress;
		nMaxAddress = nMinAddress + m_Table[i].ModuleSize;

		//判断传入的地址是否属于该模块
		if (nAddress >= nMinAddress && nAddress <= nMaxAddress)
		{
			//确定模块后，遍历此模块通过函数名导出的导出函数地址
			for (DWORD x = 0; x < m_Table[i].ExportFunctionAddress.size(); x++)
			{
				//如果导出函数的地址与传入的地址相等
				if (szAddress == m_Table[i].ExportFunctionAddress[x])
				{
					//返回该导出函数的地址所对应的模块名和导出函数名字
					return CString(szAddress + CString("  ")+ m_Table[i].ExportModuleName + CString(".") + m_Table[i].ExportFunctionName[x]);
				}
			}

			break;
		}

	}
	return szAddress;
}

CString CXB32DbgDlg::ShowAsmName(CString szAddress, CString szAsm)
{

	if (szAsm.GetLength() < 12)return szAsm;

	DWORD nAddress = 0;
	DWORD nMinAddress = 0;
	DWORD nMaxAddress = 0;

	//为TRUE时表示要跳转的地址内嵌在汇编指令中
	//为FALSE时表示要跳转的地址在内存中
	BOOL BeAanlyAddressFlag = FALSE;

	_stscanf_s(szAddress, TEXT("%x"), &nAddress);

	//如果要跳转的地址是内嵌在汇编指令中，标志为设置为TRUE
	if (szAsm.Find(TEXT("[")) == -1)
	{
		BeAanlyAddressFlag = TRUE;
	}

	//循环遍历所有模块
	for (DWORD i = 0; i < m_Table.size(); i++)
	{
		//计算出模块的起始地址和结束
		nMinAddress = m_Table[i].ModuleAddress;
		nMaxAddress = nMinAddress + m_Table[i].ModuleSize;

		//判断传入的地址是否属于该模块
		if (nAddress >= nMinAddress && nAddress <= nMaxAddress)
		{
			//确定模块后，遍历此模块通过函数名导出的导出函数地址
			for (DWORD x = 0; x < m_Table[i].ExportFunctionAddress.size(); x++)
			{
				//在汇编代码里查找是否有导出函数的地址
				INT nIndex = szAsm.Find(m_Table[i].ExportFunctionAddress[x]);
				//查找到了导出函数的地址
				if (nIndex != -1)
				{
					//直接将汇编代码内的地址替换为导入模块名与导出函数名
					szAsm.Replace(m_Table[i].ExportFunctionAddress[x] + TEXT("h"), m_Table[i].ExportFunctionName[x]);

					m_BeAanlyAddress.push_back({ szAddress, m_Table[i].ExportFunctionAddress[x]
						,BeAanlyAddressFlag ,m_Table[i].ExportFunctionName[x] });

					return szAsm;
				}
			}

			//遍历导入表所有信息
			for (DWORD x = 0; x < m_Table[i].ImportFunctionAddress.size(); x++)
			{
				//在汇编代码里查找是否有导入函数的地址
				INT nIndex = szAsm.Find(m_Table[i].ImportFunctionAddress[x]);
				//查找到了导入函数的地址
				if (nIndex != -1)
				{
					//直接将汇编代码内的地址替换为导入模块名与导入函数名
					szAsm.Replace(m_Table[i].ImportFunctionAddress[x] + TEXT("h"), 
						m_Table[i].ImportModuleName[x] + CString(".") + m_Table[i].ImportFunctionName[x]);

					m_BeAanlyAddress.push_back({ szAddress, m_Table[i].ImportFunctionAddress[x],BeAanlyAddressFlag ,
						m_Table[i].ImportModuleName[x] + CString(".") + m_Table[i].ImportFunctionName[x] });

					return szAsm;
				}

				//以免数组越界
				if (m_Table[i].ThunkAdress.size() >= x)
				{
					//在汇编代码里查找是否有 FirstThunk 指向的结构体数组中每个结构体的内存地址
					nIndex = szAsm.Find(m_Table[i].ThunkAdress[x]);
					//如果有
					if (nIndex != -1)
					{
						szAsm.Replace(m_Table[i].ThunkAdress[x] + TEXT("h"), 
							m_Table[i].ImportModuleName[x] + CString(".") + m_Table[i].ImportFunctionName[x]);

						m_BeAanlyAddress.push_back({ szAddress, m_Table[i].ThunkAdress[x],BeAanlyAddressFlag ,
							m_Table[i].ImportModuleName[x] + CString(".") + m_Table[i].ImportFunctionName[x] });

						return szAsm;
					}
				}

			}

			break;
		}
	}

	return szAsm;
}

DWORD CXB32DbgDlg::IsHardBreakPoint(DWORD nBreakAddress)
{
	if (m_HardBreakPoint[0].nAddress == nBreakAddress)return 0;
	else if (m_HardBreakPoint[1].nAddress == nBreakAddress)return 1;
	else if (m_HardBreakPoint[2].nAddress == nBreakAddress)return 2;
	else if (m_HardBreakPoint[3].nAddress == nBreakAddress)return 3;
	return -1;
}

DWORD CXB32DbgDlg::IsHardBreakPointEx()
{
	CONTEXT nContext{};
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);

	DR6 *pDr6 = nullptr;
	pDr6 = (DR6*)&nContext.Dr6;

	if (pDr6->Dr0)
	{
		if (m_HardBreakPoint[0].nAddress == nContext.Dr0 && m_HardBreakPoint[0].nType)return 0;
		else if (m_HardBreakPoint[1].nAddress == nContext.Dr0 && m_HardBreakPoint[1].nType)return 1;
		else if (m_HardBreakPoint[2].nAddress == nContext.Dr0 && m_HardBreakPoint[2].nType)return 2;
		else if (m_HardBreakPoint[3].nAddress == nContext.Dr0 && m_HardBreakPoint[3].nType)return 3;
	}
	else if (pDr6->Dr1)
	{
		if (m_HardBreakPoint[0].nAddress == nContext.Dr1 && m_HardBreakPoint[0].nType)return 0;
		else if (m_HardBreakPoint[1].nAddress == nContext.Dr1 && m_HardBreakPoint[1].nType)return 1;
		else if (m_HardBreakPoint[2].nAddress == nContext.Dr1 && m_HardBreakPoint[2].nType)return 2;
		else if (m_HardBreakPoint[3].nAddress == nContext.Dr1 && m_HardBreakPoint[3].nType)return 3;

	}
	else if (pDr6->Dr2)
	{
		if (m_HardBreakPoint[0].nAddress == nContext.Dr2 && m_HardBreakPoint[0].nType)return 0;
		else if (m_HardBreakPoint[1].nAddress == nContext.Dr2 && m_HardBreakPoint[1].nType)return 1;
		else if (m_HardBreakPoint[2].nAddress == nContext.Dr2 && m_HardBreakPoint[2].nType)return 2;
		else if (m_HardBreakPoint[3].nAddress == nContext.Dr2 && m_HardBreakPoint[3].nType)return 3;

	}
	else if (pDr6->Dr3)
	{
		if (m_HardBreakPoint[0].nAddress == nContext.Dr3 && m_HardBreakPoint[0].nType)return 0;
		else if (m_HardBreakPoint[1].nAddress == nContext.Dr3 && m_HardBreakPoint[1].nType)return 1;
		else if (m_HardBreakPoint[2].nAddress == nContext.Dr3 && m_HardBreakPoint[2].nType)return 2;
		else if (m_HardBreakPoint[3].nAddress == nContext.Dr3 && m_HardBreakPoint[3].nType)return 3;
	}

	return -1;
}

DWORD CXB32DbgDlg::IsTempHardBreakPoint(DWORD nBreakAddress)
{
	for (DWORD i = 0; i < 4; i++)
	{
		if (m_TempHardBreakPoint[i] == nBreakAddress)
		{
			return i;
		}
	}
	return -1;
}

BOOL CXB32DbgDlg::IsConditionHardBreakPoint(DWORD nBreakPointIndex)
{
	if (nBreakPointIndex == -1)return FALSE;
	return m_HardBreakPoint[nBreakPointIndex].nIsCondition;
}

BOOL CXB32DbgDlg::IsConditionSucessHardBreakPoint(DWORD nBreakPointIndex, CONTEXT nContext,BOOL nIsCondition)
{
	
	if (nBreakPointIndex == -1 || !nIsCondition)return FALSE;

	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_EAX)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.EAX != nContext.Eax)return FALSE;
	}
	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_EBX)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.EBX != nContext.Ebx)return FALSE;
	}
	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_ECX)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.ECX != nContext.Ecx)return FALSE;
	}
	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_EDX)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.EDX != nContext.Edx)return FALSE;
	}
	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_ESI)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.ESI != nContext.Esi)return FALSE;
	}
	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_EDI)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.EDI != nContext.Edi)return FALSE;
	}
	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_EBP)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.EBP != nContext.Ebp)return FALSE;
	}
	if (m_HardBreakPoint[nBreakPointIndex].nCondition.OPEN_ESP)
	{
		if (m_HardBreakPoint[nBreakPointIndex].nCondition.ESP != nContext.Esp)return FALSE;
	}
	return TRUE;
}

BOOL CXB32DbgDlg::IsMemoryBreakPoint(DWORD nBreakAddress)
{
	if (m_MemoryBreakPoint.nAddress == nBreakAddress) return TRUE;
	else return FALSE;
}

VOID CXB32DbgDlg::ShowOpcode(BYTE* pOpcode, DWORD nSize, DWORD nItem)
{
	CString nTemp;
	CString nOpcode;
	for (DWORD i = 0; i < nSize; ++i)
	{
		nTemp.Format(TEXT("%02X "), pOpcode[i]);
		nOpcode += nTemp;
	}

	m_list_asm.SetItemText(nItem, m_Flag_Opcode, nOpcode);
}

BOOL CXB32DbgDlg::IsColorWord(CString nStr, LPDWORD nTextColor, LPDWORD nBkColor)
{
	if (nStr.Left(4) == "call")
	{
		*nTextColor = COLOR_RED;
		*nBkColor = COLOR_WHITE;
		return TRUE;
	}
	else if (nStr.Left(1) == "j")
	{
		*nTextColor = COLOR_BLUE;
		*nBkColor = COLOR_WHITE;
		return TRUE;
	}
	else if (nStr.Left(3) == "ret")
	{
		*nTextColor = COLOR_GREEN;
		*nBkColor = COLOR_WHITE;
		return TRUE;
	}
	return FALSE;
}

//处理反汇编框高亮
void CXB32DbgDlg::OnNMCustomdrawListAsm(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	LPNMCUSTOMDRAW nmCustomDraw = &pNMCD->nmcd;

	switch (nmCustomDraw->dwDrawStage)
	{
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
	{
		//如果当前项被选中
		if (m_list_asm.IsSelectionItem(nmCustomDraw->dwItemSpec))
		{
			pNMCD->clrText = COLOR_WHITE;
			pNMCD->clrTextBk = COLOR_BLUE;
			nmCustomDraw->uItemState = 0;
			*pResult = CDRF_NEWFONT;
			return;
		}

		//循环记录着需要高亮的反汇编指令动态数组
		for (auto &value : m_AsmColor)
		{
			if (value.Item == nmCustomDraw->dwItemSpec && value.SubItem == pNMCD->iSubItem)
			{
				pNMCD->clrText = value.TextColor;
				pNMCD->clrTextBk = value.BkColor;
				break;
			}
			else
			{
				pNMCD->clrText = COLOR_BLACK;
				pNMCD->clrTextBk = COLOR_WHITE;
			}
		}
		break;
	}
	}

	*pResult = 0;
	*pResult |= CDRF_NOTIFYPOSTPAINT;		//绘制项目后通知父项，必须有，不然就没有效果
	*pResult |= CDRF_NOTIFYITEMDRAW;		//通知父项与项目相关的任何绘图操作，必须有，不然就没有效果}
}

VOID CXB32DbgDlg::ShowMemory(DWORD nAddress)
{
	BYTE nValue[16]{};
	BYTE nStrValue[17]{};

	CString nTemp;

	for (DWORD i = 0; i < m_Num_Memory; i++)
	{
		//地址
		nTemp.Format(TEXT("%08X"), nAddress + (i * _countof(nValue)));
		m_list_mem.SetItemText(i, m_Flag_MemAddress, nTemp);

		//每次读取16个字节数据
		ReadMemoryBytes(nAddress + (i * _countof(nValue)), nValue, _countof(nValue));
		//数据
		for (DWORD x = 0; x < _countof(nValue); x++)
		{
			nTemp.Format(TEXT("%02X "), nValue[x]);
			m_list_mem.SetItemText(i, x + 1, nTemp);
		}


		for (DWORD x = 0; x < _countof(nValue); x++)
		{
			if (!nValue[x])nValue[x] = '.';
		}
		memcpy_s(nStrValue, _countof(nStrValue), nValue, _countof(nValue));
		//文本
		m_list_mem.SetItemText(i, m_Flag_MemStr, CString(nStrValue));
		ZeroMemory(nValue, _countof(nValue));
		ZeroMemory(nStrValue, _countof(nStrValue));
	}
}

DWORD CXB32DbgDlg::IsTempBreakPoint(DWORD nBreakAddress)
{
	for (DWORD i = 0; i < m_TempBreakPoint.size(); i++)
	{
		if (m_TempBreakPoint[i].nAddress == nBreakAddress)
		{
			return i;
		}
	}
	return -1;
}

BOOL CXB32DbgDlg::IsConditionBreakPoint(DWORD nBreakPointIndex)
{
	if (nBreakPointIndex == -1)return FALSE;
	return m_BreakPoint[nBreakPointIndex].nIsCondition;
}

BOOL CXB32DbgDlg::IsConditionSucessBreakPoint(DWORD nBreakPointIndex, CONTEXT nContext, BOOL nIsCondition)
{
	if (nBreakPointIndex == -1 || !nIsCondition)return FALSE;

	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_EAX)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.EAX != nContext.Eax)return FALSE;
	}
	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_EBX)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.EBX != nContext.Ebx)return FALSE;
	}
	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_ECX)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.ECX != nContext.Ecx)return FALSE;
	}
	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_EDX)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.EDX != nContext.Edx)return FALSE;
	}
	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_ESI)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.ESI != nContext.Esi)return FALSE;
	}
	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_EDI)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.EDI != nContext.Edi)return FALSE;
	}
	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_EBP)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.EBP != nContext.Ebp)return FALSE;
	}
	if (m_BreakPoint[nBreakPointIndex].nCondition.OPEN_ESP)
	{
		if (m_BreakPoint[nBreakPointIndex].nCondition.ESP != nContext.Esp)return FALSE;
	}
	return TRUE;
}

VOID CXB32DbgDlg::DelTempBreakPoint(DWORD nBreakAddress, DWORD nTempBreakPointIndex)
{
	BYTE nWriteByte;
	nWriteByte = m_TempBreakPoint[nTempBreakPointIndex].nData;
	//恢复断点处数据
	WriteMemoryByte(nBreakAddress, nWriteByte);
	//将断点信息从数组中删除
	m_TempBreakPoint.erase(m_TempBreakPoint.begin() + nTempBreakPointIndex);
}

VOID CXB32DbgDlg::WriteMemoryByte(DWORD nAddress, BYTE nValue)
{
	DWORD nOldProtect;
	DWORD nWriteSize;
	//修改页属性为可读可写
	VirtualProtectEx(m_ProcessHandle, (LPVOID)nAddress, (SIZE_T)1, PAGE_READWRITE, &nOldProtect);
	//修改数据
	WriteProcessMemory(m_ProcessHandle, (LPVOID)nAddress, &nValue, 1, &nWriteSize);
	//恢复页属性
	VirtualProtectEx(m_ProcessHandle, (LPVOID)nAddress, 1, nOldProtect, &nOldProtect);
}

VOID CXB32DbgDlg::WriteMemoryBytes(DWORD nAddress, LPBYTE nValue, DWORD nLen)
{
	DWORD nOldProtect;
	DWORD nWriteSize;
	//修改页属性为可读可写可执行
	VirtualProtectEx(m_ProcessHandle, (LPVOID)nAddress, nLen, PAGE_EXECUTE_READWRITE, &nOldProtect);
	//修改数据
	WriteProcessMemory(m_ProcessHandle, (LPVOID)nAddress, nValue, nLen, &nWriteSize);
	//恢复页属性
	VirtualProtectEx(m_ProcessHandle, (LPVOID)nAddress, nLen, nOldProtect, &nOldProtect);
}

//处理寄存器框高亮
void CXB32DbgDlg::OnNMCustomdrawListReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	LPNMCUSTOMDRAW nmCustomDraw = &pNMCD->nmcd;

	switch (nmCustomDraw->dwDrawStage)
	{
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
	{
		//如果当前项被选中
		if (m_list_reg.IsSelectionItem(nmCustomDraw->dwItemSpec))
		{
			pNMCD->clrText = COLOR_WHITE;
			pNMCD->clrTextBk = COLOR_BLUE;
			nmCustomDraw->uItemState = 0;
			*pResult = CDRF_NEWFONT;
			return;
		}

		//循环记录着需要高亮的反汇编指令动态数组
		for (auto &value : m_RegColor)
		{
			if (value.Item == nmCustomDraw->dwItemSpec && value.SubItem == pNMCD->iSubItem)
			{
				pNMCD->clrText = value.TextColor;
				pNMCD->clrTextBk = value.BkColor;
				break;
			}
			else
			{
				pNMCD->clrText = COLOR_BLACK;
				pNMCD->clrTextBk = COLOR_WHITE;
			}
		}
		break;
	}
	}

	*pResult |= CDRF_NOTIFYPOSTPAINT;		//绘制项目后通知父项，必须有，不然就没有效果
	*pResult |= CDRF_NOTIFYITEMDRAW;		//通知父项与项目相关的任何绘图操作，必须有，不然就没有效果}
}

VOID CXB32DbgDlg::SetAllWindowsLong()
{
	g_AsmListProc = SetWindowLong(m_list_asm.m_hWnd, GWL_WNDPROC, (LONG)ListAsmProc);
	g_MemoryListProc = SetWindowLong(m_list_mem.m_hWnd, GWL_WNDPROC, (LONG)ListMemoryProc);
}

VOID CXB32DbgDlg::PrintOnceAsm(DWORD nReadAddress)
{
	BYTE nReadBuff[1024]{};
	CString nAddr;
	DISASM disAsm = { 0 };
	COLORREF nTextColor;
	COLORREF nBkColor;

	ReadMemoryBytes(nReadAddress, nReadBuff, 1024);

	disAsm.EIP = (UIntPtr)nReadBuff;
	disAsm.VirtualAddr = nReadAddress;
	disAsm.Archi = 0;
	disAsm.Options = 0x000;
	DWORD nOpcodeSize = 1024;

	INT nLen = 0;

	ClearItemColor();
	for (DWORD i = 0; i < m_Num_Asm; i++)
	{
		nLen = Disasm(&disAsm);
		if (nLen == -1)
		{
			disAsm.EIP += 1;
			disAsm.VirtualAddr += 1;
			continue;
		}

		PrintBreakFlag((DWORD)disAsm.VirtualAddr, i);
		nAddr.Format(TEXT("%08X"), disAsm.VirtualAddr);
		m_list_asm.SetItemText(i, m_Flag_Address, ShowExportName(nAddr));

		ShowOpcode((BYTE*)disAsm.EIP, nLen, i);
		m_list_asm.SetItemText(i, m_Flag_Asm, ShowAsmName(nAddr, CString(disAsm.CompleteInstr)));


		disAsm.EIP += nLen;
		disAsm.VirtualAddr += nLen;

		//修复int3断点处的数据
		RepairAsm(nReadAddress, (DWORD)disAsm.VirtualAddr, nReadBuff);

		//如果是高亮指令，将指令信息存入动态数组中，以便后面绘画使用
		if (IsColorWord(CString(disAsm.CompleteInstr), &nTextColor, &nBkColor))
		{
			SetItemColor(i, m_Flag_Asm, nTextColor, nBkColor, AsmBlock);
		}
	}

	_stscanf_s(m_list_asm.GetItemText(0, m_Flag_Address), TEXT("%x"), &nReadAddressFirst);
	_stscanf_s(m_list_asm.GetItemText(1, m_Flag_Address), TEXT("%x"), &nReadAddressSecond);
	//_stscanf_s(m_List_Asm.GetItemText(m_Num_Asm - 1, m_Flag_Address), TEXT("%x"), &nReadAddressEnd);

}

VOID CXB32DbgDlg::RepairAsm(DWORD nReadAddress, DWORD nVirtualAddress, BYTE *nRepairBytes)
{
	DWORD nIsBreakPoint;
	nIsBreakPoint = IsBreakPoint(nVirtualAddress);
	if (nIsBreakPoint != -1)
	{
		nRepairBytes[nVirtualAddress - nReadAddress] = m_BreakPoint[nIsBreakPoint].nData;
	}

	DWORD nIsTempBreakPoint;
	nIsTempBreakPoint = IsTempBreakPoint(nVirtualAddress);
	if (nIsTempBreakPoint != -1)
	{
		nRepairBytes[nVirtualAddress - nReadAddress] = m_TempBreakPoint[nIsTempBreakPoint].nData;
	}
}

DWORD CXB32DbgDlg::GetBeforeAddress(DWORD nReadAddress)
{

	BYTE nReadBuff[128]{};
	CString nAddr;
	DISASM disAsm = { 0 };

	ReadMemoryBytes((nReadAddress - 30), nReadBuff, 128);

	disAsm.EIP = (UIntPtr)nReadBuff;
	disAsm.VirtualAddr = nReadAddress - 30;
	disAsm.Archi = 0;
	disAsm.Options = 0x000;
	DWORD nOpcodeSize = 128;

	INT nLen = 0;

	for (DWORD i = 0; i < 128; i++)
	{
		nLen = Disasm(&disAsm);
		if (nLen == -1)
		{
			disAsm.EIP += 1;
			disAsm.VirtualAddr += 1;
			continue;
		}
		disAsm.EIP += nLen;
		disAsm.VirtualAddr += nLen;

		if (disAsm.VirtualAddr == nReadAddress)
		{
			return nReadAddress -= nLen;
		}
	}

	return nReadAddress - 1;
}

VOID CXB32DbgDlg::SetBreakPoint(DWORD nBreakAddress, BOOL nIsCondotion, CONDITION nCondition)
{
	BYTE nReadByte;
	BYTE nWriteByte = 0xcc;

	ReadMemoryBytes(nBreakAddress, &nReadByte,1);
	WriteMemoryByte(nBreakAddress, nWriteByte);

	if (nIsCondotion)	m_BreakPoint.push_back({ nBreakAddress ,nReadByte,TRUE,nCondition });
	else 	m_BreakPoint.push_back({ nBreakAddress ,nReadByte,FALSE,NULL });
}

VOID CXB32DbgDlg::DelBreakPoint(DWORD nBreakAddress)
{
	BYTE nWriteByte;
	DWORD nIndex;

	nIndex = IsBreakPoint(nBreakAddress);
	if (nIndex == -1)return;

	nWriteByte = m_BreakPoint[nIndex].nData;
	WriteMemoryByte(nBreakAddress, nWriteByte);
	m_BreakPoint.erase(m_BreakPoint.begin() + nIndex);
}

VOID CXB32DbgDlg::CloseAllBreakPoint()
{
	BYTE nWriteByte;

	for (DWORD i = 0; i < m_BreakPoint.size(); i++)
	{
		nWriteByte = m_BreakPoint[i].nData;
		WriteMemoryByte(m_BreakPoint[i].nAddress, nWriteByte);
	}
}

BOOL CXB32DbgDlg::CloseOneBreakPoint(DWORD nAddress)
{
	BYTE nWriteByte;

	for (DWORD i = 0; i < m_BreakPoint.size(); i++)
	{
		if (m_BreakPoint[i].nAddress == nAddress)
		{
			nWriteByte = m_BreakPoint[i].nData;
			WriteMemoryByte(m_BreakPoint[i].nAddress, nWriteByte);
			return TRUE;
		}
	}
	return FALSE;
}

VOID CXB32DbgDlg::OpenAllBreakPoint()
{
	BYTE nWriteByte = 0xcc;

	for (DWORD i = 0; i < m_BreakPoint.size(); i++)
	{
		WriteMemoryByte(m_BreakPoint[i].nAddress, nWriteByte);
	}
}

VOID CXB32DbgDlg::OpenAllHardBreakPoint()
{
	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	if (m_HardBreakPoint[0].nAddress != 0)
	{
		nContext.Dr0 = m_HardBreakPoint[0].nAddress;
		nDr7->Dr0_L = 1;
		nDr7->Dr0_LEN = m_HardBreakPoint[0].nSize;
		nDr7->Dr0_RWE = m_HardBreakPoint[0].nType;
	}
	if (m_HardBreakPoint[1].nAddress != 0)
	{
		nContext.Dr1 = m_HardBreakPoint[1].nAddress;
		nDr7->Dr1_L = 1;
		nDr7->Dr1_LEN = m_HardBreakPoint[1].nSize;
		nDr7->Dr1_RWE = m_HardBreakPoint[1].nType;
	}
	if (m_HardBreakPoint[2].nAddress != 0)
	{
		nContext.Dr2 = m_HardBreakPoint[2].nAddress;
		nDr7->Dr2_L = 1;
		nDr7->Dr2_LEN = m_HardBreakPoint[2].nSize;
		nDr7->Dr2_RWE = m_HardBreakPoint[2].nType;
	}
	if (m_HardBreakPoint[3].nAddress != 0)
	{
		nContext.Dr3 = m_HardBreakPoint[3].nAddress;
		nDr7->Dr3_L = 1;
		nDr7->Dr3_LEN = m_HardBreakPoint[3].nSize;
		nDr7->Dr3_RWE = m_HardBreakPoint[3].nType;
	}
	SetThreadContext(m_ThreadHandle, &nContext);
}

VOID CXB32DbgDlg::ShowSingleAsm(DWORD nShowAddress)
{

	if (m_list_asm.GetSelectionItem() == m_list_asm.GetItemCount() - 2)
	{
		g_pDlg->PrintOnceAsm(nReadAddressSecond);

		INT nIndex = FindAddress(nShowAddress);
		if (nIndex != -1)m_list_asm.SetSelectionEx(nIndex);
		else
		{
			PrintOnceAsm(nShowAddress);
			m_list_asm.SetSelectionEx(0);
		}
	}
	else
	{

		INT nIndex = FindAddress(nShowAddress);
		if (nIndex != -1)m_list_asm.SetSelectionEx(nIndex);
		else
		{
			PrintOnceAsm(nShowAddress);
			//选中列表第一项
			m_list_asm.SetSelectionEx(0);
		}
	}
}

DWORD CXB32DbgDlg::FindAddress(DWORD nShowAddress)
{
	DWORD nShow;
	CString nFind;
	for (INT i = 0; i < m_list_asm.GetItemCount(); i++)
	{
		nFind = m_list_asm.GetItemText(i, m_Flag_Address);
		_stscanf_s(nFind, TEXT("%08X"), &nShow);
		if (nShowAddress == nShow)return i;
	}
	return -1;
}

VOID CXB32DbgDlg::ShowStack(DWORD nShowAddress)
{

	DWORD nReadData;
	CString nShowData;
	for (DWORD i = 0; i < m_Num_Stack; i++)
	{
		nShowData.Format(TEXT("%08X+%X"), nShowAddress, (i * 4));
		m_list_stack.SetItemText(i, m_Flag_StackAddress, nShowData);
		ReadMemoryBytes(nShowAddress + (i * 4), (LPBYTE)&nReadData,4);
		nShowData.Format(TEXT("%08X"), nReadData);
		m_list_stack.SetItemText(i, m_Flag_StackData, nShowData);
	}

}

BOOL CXB32DbgDlg::IsCallRepAsm(DWORD nAddress)
{
	BYTE nReadBuff[64]{};
	CString nAddr;
	DISASM disAsm = { 0 };

	ReadMemoryBytes(nAddress, nReadBuff, 64);

	disAsm.EIP = (UIntPtr)nReadBuff;
	disAsm.VirtualAddr = nAddress;
	disAsm.Archi = 0;
	disAsm.Options = 0x000;
	DWORD nOpcodeSize = 64;

	INT nLen = 0;

	for (DWORD i = 0; i < 64; i++)
	{
		nLen = Disasm(&disAsm);
		if (nLen == -1)
		{
			disAsm.EIP += 1;
			disAsm.VirtualAddr += 1;
			continue;
		}
		break;
	}

	CString nTempAsm(disAsm.CompleteInstr);
	if (nTempAsm.Left(4) == TEXT("call"))return TRUE;
	else if (nTempAsm.Left(3) == TEXT("rep"))return TRUE;
	return FALSE;
}

DWORD CXB32DbgDlg::GetNextAddress(DWORD nReadAddress)
{
	BYTE nReadBuff[128]{};
	CString nAddr;
	DISASM disAsm = { 0 };

	ReadMemoryBytes(nReadAddress, nReadBuff, 128);

	disAsm.EIP = (UIntPtr)nReadBuff;
	disAsm.VirtualAddr = nReadAddress;
	disAsm.Archi = 0;
	disAsm.Options = 0x000;
	DWORD nOpcodeSize = 128;

	INT nLen = 0;

	for (DWORD i = 0; i < 128; i++)
	{
		nLen = Disasm(&disAsm);
		if (nLen == -1)
		{
			disAsm.EIP += 1;
			disAsm.VirtualAddr += 1;
			continue;
		}

		disAsm.EIP += nLen;
		disAsm.VirtualAddr += nLen;

		return (DWORD)disAsm.VirtualAddr;
	}
	return nReadAddress + 1;
}

VOID CXB32DbgDlg::SetTempBreakPoint(DWORD nBreakAddress)
{
	BYTE nReadByte;
	BYTE nWriteByte = 0xcc;

	ReadMemoryBytes(nBreakAddress, &nReadByte, 1);
	WriteMemoryByte(nBreakAddress, nWriteByte);

	m_TempBreakPoint.push_back({ nBreakAddress ,nReadByte });
}

VOID CXB32DbgDlg::SetTempHardBreakPoint(DWORD nBreakAddress)
{
	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	nContext.Dr0 = nBreakAddress;
	nDr7->Dr0_L = 1;
	nDr7->Dr0_LEN = 0;
	nDr7->Dr0_RWE = 0;
	SetThreadContext(m_ThreadHandle, &nContext);
	m_TempHardBreakPoint[0] = nBreakAddress;

}

BOOL CXB32DbgDlg::SetHardBreakPoint(DWORD nBreakAddress, BOOL nIsCondotion, CONDITION nCondition)
{
	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	//如果Dr0没有保存断点地址信息
	if (nContext.Dr0 == 0)
	{
		m_HardBreakPoint[0].nAddress = nBreakAddress;
		m_HardBreakPoint[0].nType = 0;
		m_HardBreakPoint[0].nSize = 0;
		m_HardBreakPoint[0].nIsCondition = nIsCondotion;
		m_HardBreakPoint[0].nCondition = nCondition;

		nContext.Dr0 = nBreakAddress;
		nDr7->Dr0_L = 1;
		nDr7->Dr0_LEN = 0;
		nDr7->Dr0_RWE = 0;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	else if (nContext.Dr1 == 0)
	{
		m_HardBreakPoint[1].nAddress = nBreakAddress;
		m_HardBreakPoint[1].nType = 0;
		m_HardBreakPoint[1].nSize = 0;
		m_HardBreakPoint[1].nIsCondition = nIsCondotion;
		m_HardBreakPoint[1].nCondition = nCondition;

		nContext.Dr1 = nBreakAddress;
		nDr7->Dr1_L = 1;
		nDr7->Dr1_LEN = 0;
		nDr7->Dr1_RWE = 0;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	else if (nContext.Dr2 == 0)
	{
		m_HardBreakPoint[2].nAddress = nBreakAddress;
		m_HardBreakPoint[2].nType = 0;
		m_HardBreakPoint[2].nSize = 0;
		m_HardBreakPoint[2].nIsCondition = nIsCondotion;
		m_HardBreakPoint[2].nCondition = nCondition;

		nContext.Dr2 = nBreakAddress;
		nDr7->Dr2_L = 1;
		nDr7->Dr2_LEN = 0;
		nDr7->Dr2_RWE = 0;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	else if (nContext.Dr3 == 0)
	{
		m_HardBreakPoint[3].nAddress = nBreakAddress;
		m_HardBreakPoint[3].nType = 0;
		m_HardBreakPoint[3].nSize = 0;
		m_HardBreakPoint[3].nIsCondition = nIsCondotion;
		m_HardBreakPoint[3].nCondition = nCondition;

		nContext.Dr3 = nBreakAddress;
		nDr7->Dr3_L = 1;
		nDr7->Dr3_LEN = 0;
		nDr7->Dr3_RWE = 0;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	return FALSE;
}

BOOL CXB32DbgDlg::SetHardBreakPointEx(DWORD nBreakAddress, DWORD nType, DWORD nSize)
{
	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	if (nContext.Dr0 == 0)
	{
		m_HardBreakPoint[0].nAddress = nBreakAddress;
		m_HardBreakPoint[0].nType = nType;
		m_HardBreakPoint[0].nSize = nSize;
		nContext.Dr0 = nBreakAddress;
		nDr7->Dr0_L = 1;
		nDr7->Dr0_LEN = nSize;
		nDr7->Dr0_RWE = nType;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	else if (nContext.Dr1 == 0)
	{
		m_HardBreakPoint[1].nAddress = nBreakAddress;
		m_HardBreakPoint[1].nType = nType;
		m_HardBreakPoint[1].nSize = nSize;
		nContext.Dr1 = nBreakAddress;
		nDr7->Dr1_L = 1;
		nDr7->Dr1_LEN = nSize;
		nDr7->Dr1_RWE = nType;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	else if (nContext.Dr2 == 0)
	{
		m_HardBreakPoint[2].nAddress = nBreakAddress;
		m_HardBreakPoint[2].nType = nType;
		m_HardBreakPoint[2].nSize = nSize;
		nContext.Dr2 = nBreakAddress;
		nDr7->Dr2_L = 1;
		nDr7->Dr2_LEN = nSize;
		nDr7->Dr2_RWE = nType;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	else if (nContext.Dr3 == 0)
	{
		m_HardBreakPoint[3].nAddress = nBreakAddress;
		m_HardBreakPoint[3].nType = nType;
		m_HardBreakPoint[3].nSize = nSize;
		nContext.Dr3 = nBreakAddress;
		nDr7->Dr3_L = 1;
		nDr7->Dr3_LEN = nSize;
		nDr7->Dr3_RWE = nType;
		SetThreadContext(m_ThreadHandle, &nContext);
		return TRUE;
	}
	return FALSE;
}

VOID CXB32DbgDlg::DelHardBreakPoint(DWORD nBreakAddress)
{
	DWORD nIndex;
	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	nIndex = IsHardBreakPoint(nBreakAddress);
	if (nIndex == -1)return;

	if (nIndex == 0)
	{
		nContext.Dr0 = 0;
		nDr7->Dr0_L = 0;
		nDr7->Dr0_LEN = 0;
		nDr7->Dr0_RWE = 0;
		m_HardBreakPoint[0].nAddress = 0;
		m_HardBreakPoint[0].nType = 0;
	}
	else if (nIndex == 1)
	{
		nContext.Dr1 = 0;
		nDr7->Dr1_L = 0;
		nDr7->Dr1_LEN = 0;
		nDr7->Dr1_RWE = 0;
		m_HardBreakPoint[1].nAddress = 0;
		m_HardBreakPoint[1].nType = 0;
	}
	else if (nIndex == 2)
	{
		nContext.Dr2 = 0;
		nDr7->Dr2_L = 0;
		nDr7->Dr2_LEN = 0;
		nDr7->Dr2_RWE = 0;
		m_HardBreakPoint[2].nAddress = 0;
		m_HardBreakPoint[2].nType = 0;
	}
	else if (nIndex == 3)
	{
		nContext.Dr3 = 0;
		nDr7->Dr3_L = 0;
		nDr7->Dr3_LEN = 0;
		nDr7->Dr3_RWE = 0;
		m_HardBreakPoint[3].nAddress = 0;
		m_HardBreakPoint[3].nType = 0;
	}
	SetThreadContext(m_ThreadHandle, &nContext);
}

BOOL CXB32DbgDlg::HaveMemoryBreakPoint()
{
	if (m_MemoryBreakPoint.nAddress == 0) return FALSE;
	else return TRUE;
}

VOID CXB32DbgDlg::SetMemoryBreakPoint(DWORD nBreakAddress, DWORD nType, BOOL nIsCondotion, CONDITION nCondotion)
{
	//修改页属性为禁用对页面提交区域的所有访问。尝试读取，写入或执行提交的区域会导致访问冲突
	VirtualProtectEx(m_ProcessHandle, (LPVOID)nBreakAddress, 1, PAGE_NOACCESS, &m_MemoryBreakPoint.nOldPageProtect);

	m_MemoryBreakPoint.nAddress = nBreakAddress;
	m_MemoryBreakPoint.nType = nType;
	m_MemoryBreakPoint.nIsCondition = nIsCondotion;
	m_MemoryBreakPoint.nCondition = nCondotion;
}

VOID CXB32DbgDlg::DelMemoryBreakPoint(DWORD nBreakAddress)
{
	DWORD nOldProtece;

	//恢复页保护属性
	VirtualProtectEx(m_ProcessHandle, (LPVOID)nBreakAddress, 1, m_MemoryBreakPoint.nOldPageProtect, &nOldProtece);

	ZeroMemory(&m_MemoryBreakPoint, sizeof(MEMORYBREAKPOINT));
}

CString CXB32DbgDlg::GetOnceAsm(DWORD nReadAddress)
{
	BYTE nReadBuff[128]{};
	CString nAddr;
	DISASM disAsm = { 0 };

	ReadMemoryBytes(nReadAddress, nReadBuff, 128);

	disAsm.EIP = (UIntPtr)nReadBuff;
	disAsm.VirtualAddr = nReadAddress;
	disAsm.Archi = 0;
	disAsm.Options = 0x000;
	DWORD nOpcodeSize = 128;

	Disasm(&disAsm);

	return CString(disAsm.CompleteInstr);

}

VOID CXB32DbgDlg::AddBreakReadWrite(DWORD nBreakAddress, CString nAsm,DWORD nFlags)
{
	BOOL nHave = FALSE;
	if (m_BreakReadWrite.size() == 0)
	{
		m_BreakReadWrite.push_back({ nBreakAddress ,nAsm,nFlags });
		return;
	}

	for (DWORD i = 0; i < m_BreakReadWrite.size(); i++)
	{
		if (m_BreakReadWrite[i].nAddress == nBreakAddress)
		{
			nHave = TRUE;
			break;
		}
	}

	if (!nHave)	m_BreakReadWrite.push_back({ nBreakAddress ,nAsm,nFlags });

}

VOID CXB32DbgDlg::CloseAllHardBreakPoint()
{
	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	nContext.Dr0 = 0;
	nContext.Dr1 = 0;
	nContext.Dr2 = 0;
	nContext.Dr3 = 0;

	nDr7->Dr0_L = 0;
	nDr7->Dr0_LEN = 0;
	nDr7->Dr0_RWE = 0;
	nDr7->Dr1_L = 0;
	nDr7->Dr1_LEN = 0;
	nDr7->Dr1_RWE = 0;
	nDr7->Dr2_L = 0;
	nDr7->Dr2_LEN = 0;
	nDr7->Dr2_RWE = 0;
	nDr7->Dr3_L = 0;
	nDr7->Dr3_LEN = 0;
	nDr7->Dr3_RWE = 0;
	SetThreadContext(m_ThreadHandle, &nContext);
}

BOOL CXB32DbgDlg::CloseOneHardBreakPoint(DWORD nAddress)
{
	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	BOOL bRet = FALSE;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	DWORD i = 0;
	for (; i < 4; i++)
	{
		if (m_HardBreakPoint[i].nAddress == nAddress)
		{
			bRet = TRUE;
			break;
		}
	}
	switch (i)
	{
	case 0:
		nContext.Dr0 = 0;
		nDr7->Dr0_L = 0;
		nDr7->Dr0_LEN = 0;
		nDr7->Dr0_RWE = 0;
		break;
	case 1:
		nContext.Dr1 = 0;
		nDr7->Dr1_L = 0;
		nDr7->Dr1_LEN = 0;
		nDr7->Dr1_RWE = 0;
		break;
	case 2:
		nContext.Dr2 = 0;
		nDr7->Dr2_L = 0;
		nDr7->Dr2_LEN = 0;
		nDr7->Dr2_RWE = 0;
		break;
	case 3:
		nContext.Dr3 = 0;
		nDr7->Dr3_L = 0;
		nDr7->Dr3_LEN = 0;
		nDr7->Dr3_RWE = 0;
		break;
	default:
		break;
	}

	SetThreadContext(m_ThreadHandle, &nContext);
	return bRet;
}

BOOL CXB32DbgDlg::CloseOneMemBreakPoint(DWORD nAddress)
{
	DWORD nOldProtect;

	for (DWORD i = 0; i < m_BreakReadWrite.size(); i++)
	{
		if (m_BreakReadWrite[i].nAddress == nAddress&&m_BreakReadWrite[i].nIsHardOrMem == 2)
		{
			//恢复页保护属性
			VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, m_MemoryBreakPoint.nOldPageProtect, &nOldProtect);
			return TRUE;
		}
	}
	return FALSE;

}

VOID CXB32DbgDlg::DelTempHardBreakPoint(DWORD nBreakAddress)
{
	DWORD nIndex;

	nIndex = IsTempHardBreakPoint(nBreakAddress);
	if (nIndex == -1)return;

	CONTEXT nContext{};
	PDR7 nDr7 = nullptr;
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);
	nDr7 = (PDR7)&nContext.Dr7;

	nContext.Dr0 = 0;
	nDr7->Dr0_L = 0;
	nDr7->Dr0_LEN = 0;
	nDr7->Dr0_RWE = 0;
	SetThreadContext(m_ThreadHandle, &nContext);

	m_TempHardBreakPoint[nIndex] = 0;
}

DWORD CXB32DbgDlg::GetMemoryBreakPointType()
{
	if (!HaveMemoryBreakPoint())return -1;
	return m_MemoryBreakPoint.nType;
}

VOID CXB32DbgDlg::SetHardReadWriteProc(DWORD nType, DWORD nSize)
{
	DWORD nBreakAddress;
	CString nTempAddress;
	DWORD nIndex;
	BOOL nRet;

	//获取选中的项的内存地址
	nTempAddress = m_list_mem.GetItemText(m_nTempItem, m_Flag_MemAddress);
	_stscanf_s(nTempAddress, TEXT("%x"), &nBreakAddress);
	nBreakAddress = nBreakAddress + m_nTempSubItem - 1;

	//检测是否已经是断点
	nIndex = IsHardBreakPoint(nBreakAddress);
	//如果不是,就将其设置为断点，信息加入断点数组中
	if (nIndex == -1)
	{
		//设置硬件读写断点
		nRet = SetHardBreakPointEx(nBreakAddress, nType, nSize);
		if (!nRet)
		{
			MessageBox(TEXT("硬件断点失败！"), TEXT("失败！"), MB_ICONERROR);
			return;
		}
		//ClearBreakReadWrite();
		//ReadWriteDlg * pReadWriteDlg = new ReadWriteDlg;
	}
	//如果是，就删除该地址处断点
	else
	{
		DelHardBreakPoint(nBreakAddress);
	}

}

VOID CXB32DbgDlg::SetMemoryReadWriteProc(DWORD nType)
{
	DWORD nBreakAddress;
	CString nTempAddress;
	DWORD nIndex;

	//获取选中的项的内存地址
	nTempAddress = m_list_mem.GetItemText(m_nTempItem, m_Flag_MemAddress);
	_stscanf_s(nTempAddress, TEXT("%x"), &nBreakAddress);
	nBreakAddress = nBreakAddress + m_nTempSubItem - 1;

	//检测是否已经是断点
	nIndex = IsMemoryBreakPoint(nBreakAddress);
	//如果已经存在内存断点，且该断点不是内存断点
	if (HaveMemoryBreakPoint() && nIndex == FALSE)
	{
		MessageBox(TEXT("已存在内存断点！断点失败！"), TEXT("失败！"), MB_ICONERROR);
		return;
	}
	//如果不是,就将其设置为断点，信息加入断点数组中
	if (nIndex == FALSE)
	{
		SetMemoryBreakPoint(nBreakAddress, nType, FALSE, {});

		//ClearBreakReadWrite();
		//ReadWriteDlg * pReadWriteDlg = new ReadWriteDlg;
	}
	//如果是，就删除该地址处断点
	else
	{
		DelMemoryBreakPoint(nBreakAddress);
	}

}

void CXB32DbgDlg::OnChangeasm()
{
	//获取选中的行数
	DWORD nItem = m_list_asm.GetSelectionItem();
	CString nAddress;
	CString nOpcode;
	//根据行数获取对应的地址与十六进制数据
	nOpcode = m_list_asm.GetItemText(nItem, m_Flag_Opcode);
	nAddress = m_list_asm.GetItemText(nItem, m_Flag_Address);
	//过滤掉地址后面可能出现的符号
	nAddress = nAddress.Left(8);
	//地址进行转换
	_stscanf_s(nAddress, TEXT("%X"), &g_SendAddress_Asm);

	//获取对应的汇编指令
	g_SendAsm_Asm = m_list_asm.GetItemText(nItem, m_Flag_Asm);

	//看看汇编指令是否已经被解析成符号
	for (DWORD i = 0; i < m_BeAanlyAddress.size(); i++)
	{
		if (nAddress == m_BeAanlyAddress[i].nAddress)
		{
			//在汇编代码里查找是否有符号
			INT nIndex = g_SendAsm_Asm.Find(m_BeAanlyAddress[i].Symbol);
			//查找到了符号
			if (nIndex != -1)
			{
				//直接将汇编代码内的符号替换为之前的地址
				g_SendAsm_Asm.Replace(m_BeAanlyAddress[i].Symbol, m_BeAanlyAddress[i].nBeAnalyzedAddress+_T("h"));

			}
			break;
		}
	}


	//获取汇编指令对应十六进制的长度
	g_SendBytesNum_Asm = nOpcode.Replace(TEXT(" "), TEXT(" "));

	g_Ok_Asm = FALSE;
	EditAsmDlg nEditAsmDlg;
	nEditAsmDlg.DoModal();
	//如果用户没有点击确定按钮，就直接返回
	if (!g_Ok_Asm)return;

	if (g_RecvBytesNum_Asm)
	{
		CString nTemp;
		CString nTempBytes;

		for (DWORD i = 0; i < g_RecvBytesNum_Asm; i++)
		{
			nTemp.Format(TEXT("%02X "), g_RecvBytes_Asm[i]);
			nTempBytes = nTempBytes + nTemp;
		}

		//将要修改的指令写入到目标进程中
		WriteMemoryBytes(g_SendAddress_Asm, g_RecvBytes_Asm, g_RecvBytesNum_Asm);

		InitAsm(nReadAddressFirst);

	}
}

VOID CXB32DbgDlg::OnChangeReg()
{
	//获取选中的行数
	DWORD nItem = m_list_reg.GetSelectionItem();
	CString regName;
	CString regValue;

	TCHAR* regNames[17] = { _T("EAX"),_T("EBX"),_T("ECX"),_T("EDX"),_T("EBP"),_T("ESP"),_T("ESI"),_T("EDI"),
		_T("CF"),_T("PF"),_T("AF"),_T("ZF"),_T("SF"),_T("TF"),_T("DF"),_T("OF") ,_T("Eflags") };

	//根据行数获取对应的寄存器名与值
	regName = m_list_reg.GetItemText(nItem, 0);
	regValue = m_list_reg.GetItemText(nItem, 1);

	_stscanf_s(regValue, TEXT("%X"), &g_SendValue_Reg);

	//获取线程上下文环境
	CONTEXT nContext{};
	nContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(m_ThreadHandle, &nContext);

	PEFLAGS eflags = (PEFLAGS)&nContext.EFlags;

	for (DWORD i = 0; i < 17; i++)
	{
		//如果是可以修改的寄存器
		if (!regName.Compare(regNames[i]))
		{
			switch (i)
			{
			case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 16:
			{
				g_Ok_Reg = FALSE;
				EditRegDlg nEditRegDlg;
				nEditRegDlg.DoModal();
				if (!g_Ok_Reg)return;

				switch (i)
				{
				case 0:
					nContext.Eax = g_RecvValue_Reg;
					break;
				case 1:
					nContext.Ebx = g_RecvValue_Reg;
					break;
				case 2:
					nContext.Ecx = g_RecvValue_Reg;
					break;
				case 3:
					nContext.Edx = g_RecvValue_Reg;
					break;
				case 4:
					nContext.Ebp = g_RecvValue_Reg;
					break;
				case 5:
					nContext.Esp = g_RecvValue_Reg;
					break;
				case 6:
					nContext.Esi = g_RecvValue_Reg;
					break;
				case 7:
					nContext.Edi = g_RecvValue_Reg;
					break;
				case 16:
					nContext.EFlags = g_RecvValue_Reg;
					break;
				}
				regValue.Format(_T("%08X"), g_RecvValue_Reg);
				m_list_reg.SetItemText(nItem, 1, regValue);

				break;
			}

				//CF
			case 8:
				eflags->CF = !eflags->CF;
				regValue.Format(_T("%d"), eflags->CF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
				//PF
			case 9:
				eflags->PF = !eflags->PF;
				regValue.Format(_T("%d"), eflags->PF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
				//AF
			case 10:
				eflags->AF = !eflags->AF;
				regValue.Format(_T("%d"), eflags->AF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
				//ZF
			case 11:
				eflags->ZF = !eflags->ZF;
				regValue.Format(_T("%d"), eflags->ZF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
				//SF
			case 12:
				eflags->SF = !eflags->SF;
				regValue.Format(_T("%d"), eflags->SF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
				//TF
			case 13:
				eflags->TF = !eflags->TF;
				regValue.Format(_T("%d"), eflags->TF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
				//DF
			case 14:
				eflags->DF = !eflags->DF;
				regValue.Format(_T("%d"), eflags->DF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
				//OF
			case 15:
				eflags->OF = !eflags->OF;
				regValue.Format(_T("%d"), eflags->OF);
				m_list_reg.SetItemText(nItem, 1, regValue);
				regValue.Format(_T("%08X"), nContext.EFlags);
				m_list_reg.SetItemText(26, 1, regValue);
				break;
			}
			break;
		}
	}

	//设置线程上下文环境
	SetThreadContext(m_ThreadHandle, &nContext);
}

VOID CXB32DbgDlg::EditMemoryProc(DWORD nItem, DWORD nSubItem)
{

	CString szAddress;
	DWORD dwAddress;
	DWORD dwByte;
	//获取选定要修改的地址
	szAddress = m_list_mem.GetItemText(nItem, m_Flag_MemAddress);
	_stscanf_s(szAddress, TEXT("%X"), &g_SendAddress_Memory);
	g_SendAddress_Memory = g_SendAddress_Memory + nSubItem - 1;

	//获取要修改的数据
	g_SendBytes_Memory = m_list_mem.GetItemText(nItem, nSubItem);
	//g_SendBytes_Memory.Replace(TEXT(" "), TEXT(""));

	g_Ok_Memory = FALSE;
	EditMemoryDlg nEditMemoryDlg;
	nEditMemoryDlg.DoModal();
	if (!g_Ok_Memory)return;

	_stscanf_s(g_SendBytes_Memory, TEXT("%02X"), &dwByte);

	//如果修改前和修改后一样，直接返回
	if (dwByte == g_RecvBytes_Memory)return;

	WriteMemoryByte(g_SendAddress_Memory, (BYTE)g_RecvBytes_Memory);

	szAddress = m_list_mem.GetItemText(0, m_Flag_MemAddress);
	_stscanf_s(szAddress, TEXT("%X"), &dwAddress);

	//重新显示内存数据
	ShowMemory(dwAddress);
}

BOOL CXB32DbgDlg::IsJumpWord(CString nStr, CString nAddress, LPDWORD pAddress)
{
	DWORD nTempAddress = 0;

	//如果是call指令或j开头的指令
	if (nStr.Left(4) == CString("call") || nStr.Left(1) == CString("j"))
	{
		//看看汇编指令是否已经被解析成符号
		for (DWORD i = 0; i < m_BeAanlyAddress.size(); i++)
		{
			if (nAddress == m_BeAanlyAddress[i].nAddress)
			{		
				if (m_BeAanlyAddress[i].bFlag)
				{
					_stscanf_s(m_BeAanlyAddress[i].nBeAnalyzedAddress, TEXT("%x"), pAddress);
				}
				else
				{
					_stscanf_s(m_BeAanlyAddress[i].nBeAnalyzedAddress, TEXT("%x"), &nTempAddress);
					//从内存中获取要跳转的地址
					ReadMemoryBytes(nTempAddress, (LPBYTE)pAddress, 4);
				}

				break;
			}
		}
		//如果没有被解析成符号
		if (*pAddress == 0)
		{
			//去除指令中的h(为了去掉地址后面的h,如果自身指令带h,不好意思，那么此处就是个bug)
			nStr.Replace(TEXT("h"), TEXT(""));

			//如果要跳转的地址是直接的(不需要再从内存中读取)
			if (nStr.Find(TEXT("[")) == -1)
			{
				//获取指令的后8位，将其作为要跳转的地址
				nStr = nStr.Right(8);
				_stscanf_s(nStr, TEXT("%x"), pAddress);
			}
			//如果要跳转的地址存放在内存中
			else
			{
				//去除指令中的[与]
				nStr.Replace(TEXT("["), TEXT(""));
				nStr.Replace(TEXT("]"), TEXT(""));
				//获取指令的后8位，将其视为存放要跳转地址的地址
				nStr = nStr.Right(8);
				_stscanf_s(nStr, TEXT("%x"), &nTempAddress);
				//从内存中获取要跳转的地址
				ReadMemoryBytes(nTempAddress, (LPBYTE)pAddress, 4);
			}
		}

		return TRUE;
	}

	return FALSE;
}

void CXB32DbgDlg::OnInaddress()
{
	CString nTempAddress;
	CString nRecordAddress;
	DWORD nReadAddress = 0;
	
	//获取选定的项的地址与汇编指令
	nRecordAddress = m_list_asm.GetItemText(m_list_asm.GetSelectionItem(), m_Flag_Address);
	//过滤掉地址后面可能出现的符号
	nRecordAddress = nRecordAddress.Left(8);

	nTempAddress = m_list_asm.GetItemText(m_list_asm.GetSelectionItem(), m_Flag_Asm);

	//如果是call指令或者j开头的指令，就提取其中的地址
	if (IsJumpWord(nTempAddress, nRecordAddress, &nReadAddress))
	{
		//根据提取出的地址显示在列表上
		InitAsm(nReadAddress);
		m_list_asm.SetSelectionEx(0);
	}

}

void CXB32DbgDlg::FunDbg()
{
	if (m_bChecked_AntiPeb)
	{
		//获取PEB地址
		DWORD nPebAddress = GetPebAddress(m_ProcessHandle);

		//调试标记位BeingDebugged,可用IsDebuggerPresent()检测，其值为1则处于调试状态
		//将其改为0,过反调试
		WriteMemoryByte(nPebAddress + 0x02, '\0');

		//内核全局标记，当进程处于调试状态时值为0x70,非调试状态为0x0
		//将其改为0，过反调试
		WriteMemoryByte(nPebAddress + 0x068, '\0');
	}

	if (m_bChecked_AntiNtQInforPro)
	{

		DWORD nApiAddress;
		DWORD nHookCodeAddress;

		BYTE nJmpCode[5] = { 0xe9,0x0,0x0 ,0x0 ,0x0 };

		//00C71816 | 8B4424 08		| mov eax, dword ptr ss : [esp + 8] |	获取NtQueryInformationProcess第二个参数
		//00C7181A | 83F8 07		| cmp eax, 7						|	判断是不是获取调试端口
		//00C7181D | 75 0D			| jne demo.C7182C					|
		//00C7181F | 8B4424 0C		| mov eax, dword ptr ss : [esp + C] |	如果是获取调试端口
		//00C71823 | C700 00000000	| mov dword ptr ds : [eax], 0		|	将其设为0
		//00C71829 | C2 1400		| ret 14							|	返回
		//00C7182C | 83F8 1E		| cmp eax, 1E						|	判断是不是获取调试句柄
		//00C7182F | 75 0D			| jne demo.C7183E					|
		//00C71831 | 8B4424 0C		| mov eax, dword ptr ss : [esp + C] |
		//00C71835 | C700 00000000	| mov dword ptr ds : [eax], 0		|
		//00C7183B | C2 1400		| ret 14							|
		//00C7183E | 83F8 1F		| cmp eax, 1F						|	判断是不是获取调试标记
		//00C71841 | 8B4424 0C		| mov eax, dword ptr ss : [esp + C] |
		//00C71845 | C700 01000000	| mov dword ptr ds : [eax], 1		|
		//00C7184B | C2 1400		| ret 14							|

		BYTE nHookCode[] = {0x8B ,0x44,0x24,0x08,0x83,0xF8,0x07,0x75,0x0D,0x8B,
			0x44,0x24,0x0C,0xC7,0x00,0x00,0x00,0x00,0x00,0xC2,0x14,0x00,0x83,
			0xF8,0x1E,0x75,0x0D,0x8B,0x44,0x24,0x0C,0xC7,0x00,0x00,0x00,0x00,
			0x00,0xC2,0x14,0x00,0x83,0xF8,0x1F,0x8B,0x44,0x24,0x0C,0xC7,0x00,
			0x01,0x00,0x00,0x00,0xC2,0x14,0x00 };

		//获取函数地址
		nApiAddress = GetApiAddress(TEXT("NtQueryInformationProcess"));

		//在被调试进程申请一块空间并将shellcode写进去
		nHookCodeAddress = (DWORD)VirtualAllocEx(m_ProcessHandle, NULL, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		WriteMemoryBytes(nHookCodeAddress, nHookCode, _countof(nHookCode));

		//获取跳转偏移构造跳转指令   跳转偏移  = 目标地址 - 指令所在- 指令长度
		*(DWORD*)(nJmpCode + 1) = nHookCodeAddress - nApiAddress - 5;

		WriteMemoryBytes(nApiAddress, nJmpCode, _countof(nJmpCode));
	}
}

DWORD CXB32DbgDlg::GetPebAddress(HANDLE nHandle)
{
	PROCESS_BASIC_INFORMATION32 pbi = { 0 };
	NTSTATUS Status = NtQueryInformationProcess(
		nHandle,
		ProcessBasicInformation,
		&pbi,
		sizeof(pbi),
		NULL);

	if (NT_SUCCESS(Status))
	{
		return pbi.PebBaseAddress;
	}
	return 0;
}

DWORD CXB32DbgDlg::GetApiAddress(CString nApi)
{
	HMODULE nHandle = nullptr;
	DWORD nAddress = 0;

	nHandle = GetModuleHandle(TEXT("kernel32.dll"));
	nAddress = (DWORD)GetProcAddress(nHandle, CStringA(nApi));
	if (nAddress)return nAddress;
	FreeLibrary(nHandle);

	nHandle = GetModuleHandle(TEXT("user32.dll"));
	nAddress = (DWORD)GetProcAddress(nHandle, CStringA(nApi));
	if (nAddress)return nAddress;
	FreeLibrary(nHandle);

	nHandle = GetModuleHandle(TEXT("gdi32.dll"));
	nAddress = (DWORD)GetProcAddress(nHandle, CStringA(nApi));
	if (nAddress)return nAddress;
	FreeLibrary(nHandle);

	nHandle = GetModuleHandle(TEXT("ntdll.dll"));
	nAddress = (DWORD)GetProcAddress(nHandle, CStringA(nApi));
	if (nAddress)return nAddress;
	FreeLibrary(nHandle);

	return 0;
}

//步入
void CXB32DbgDlg::OnIn()
{
	// TODO: 在此添加命令处理程序代码

	//1.获取线程环境块
	CONTEXT context = { CONTEXT_FULL };
	GetThreadContext(m_ThreadHandle, &context);

	DWORD nOldProtect;
	//恢复页保护属性
	VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, m_MemoryBreakPoint.nOldPageProtect, &nOldProtect);

	//设置TF断点
	context.EFlags |= 0x100;
	SetThreadContext(m_ThreadHandle, &context);

	ResumeThread(m_ThreadHandle);
}

//步过
void CXB32DbgDlg::OnJump()
{
	// TODO: 在此添加命令处理程序代码

	//1.获取线程环境块
	CONTEXT context = { CONTEXT_FULL };
	GetThreadContext(m_ThreadHandle, &context);

	DWORD nOldProtect;
	//恢复页保护属性
	VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, m_MemoryBreakPoint.nOldPageProtect, &nOldProtect);

	DWORD nNextAddress;

	//如果此时eip处为软件断点处将此处数据恢复
	//因为软件断点会将第一个数据改为0xCC,影响后面
	//指令判断，所以需要先恢复
	CloseOneBreakPoint(context.Eip);

	//如果是call或者rep指令，就满足步过条件
	if (IsCallRepAsm(context.Eip))
	{
		//获取下一条指令的地址
		nNextAddress = GetNextAddress(context.Eip);

		//设置断点
		if (g_BreakType == BREAK_SOFT)
		{
			SetTempBreakPoint(nNextAddress);
		}
		else if (g_BreakType == BREAK_HARD)
		{
			SetTempHardBreakPoint(nNextAddress);
		}

	}
	//如果不是call或rep指令，不满足步过条件
	else
	{
		// 单步断点: 通过 CPU 中 efalgs 提供的 TF 标志位
		// 完成的。当CPU在执行指令之后，会检查当前的 TF 位
		// 是否开启，如果开启了，就会触发一个单步异常，并且
		// 会将 TF 标志位重新置 0。

		//设置TF断点
		context.EFlags |= 0x100;
		SetThreadContext(m_ThreadHandle, &context);

	}

	ResumeThread(m_ThreadHandle);
}

//运行
void CXB32DbgDlg::OnRun()
{
	// TODO: 在此添加命令处理程序代码

	//1.获取线程环境块
	CONTEXT context = { CONTEXT_FULL };
	GetThreadContext(m_ThreadHandle, &context);

	//将所有软件/硬件断点重新打开
	OpenAllBreakPoint();
	OpenAllHardBreakPoint();

	//将内存执行断点重启
	DWORD nOldProtect;
	VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, PAGE_NOACCESS, &nOldProtect);

	//如果此时eip处为软件断点处将此处数据恢复
	if (CloseOneBreakPoint(context.Eip))
	{
		//设置TF断点
		context.EFlags |= 0x100;
		SetThreadContext(m_ThreadHandle, &context);
		g_OpenBreak = TRUE;
	}
	//如果此时eip处为硬件断点处将此处数据恢复
	else if (CloseOneHardBreakPoint(context.Eip))
	{
		//设置TF断点
		context.EFlags |= 0x100;
		SetThreadContext(m_ThreadHandle, &context);
		g_OpenBreak = TRUE;
	}
	//如果此时eip处为内存执行断点处
	else if (context.Eip == m_MemoryBreakPoint.nAddress && m_MemoryBreakPoint.nType == 8)
	{
		//恢复页保护属性
		VirtualProtectEx(m_ProcessHandle, (LPVOID)m_MemoryBreakPoint.nAddress, 1, m_MemoryBreakPoint.nOldPageProtect, &nOldProtect);
		//设置TF断点
		context.EFlags |= 0x100;
		SetThreadContext(m_ThreadHandle, &context);
		g_OpenBreak = TRUE;
	}
	//如果此时eip处为触发内存读写断点指令地址处
	else if (CloseOneMemBreakPoint(context.Eip))
	{
		//设置TF断点
		context.EFlags |= 0x100;
		SetThreadContext(m_ThreadHandle, &context);
		g_OpenBreak = TRUE;
	}

	ResumeThread(m_ThreadHandle);
}

//设置软件断点
void CXB32DbgDlg::OnSoftbreak()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nBreakAddress;
	CString nTempAddress;
	DWORD nIndex;
	DWORD nSelect;

	//获取选中的项所在列表的行数
	nSelect = m_list_asm.GetSelectionItem();
	//获取断点地址
	nTempAddress = m_list_asm.GetItemText(nSelect, m_Flag_Address);
	_stscanf_s(nTempAddress, TEXT("%08X"), &nBreakAddress);

	//检测是否已经是断点
	nIndex = IsBreakPoint(nBreakAddress);
	//如果不是,就将其设置为断点，信息加入断点数组中
	if (nIndex == -1)
	{
		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT("●"));
		SetBreakPoint(nBreakAddress, FALSE, {});
		SetItemColor(nSelect, m_Flag_Break, COLOR_WHITE, COLOR_RED,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_WHITE, COLOR_RED,AsmBlock);
	}
	//如果是，就删除该地址处断点
	else
	{
		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT(""));
		DelBreakPoint(nBreakAddress);
		SetItemColor(nSelect, m_Flag_Break, COLOR_BLACK, COLOR_WHITE,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_BLACK, COLOR_WHITE,AsmBlock);
	}

}

// 设置软件条件断点
void CXB32DbgDlg::OnConditionsoftbreak()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nBreakAddress;
	CString nTempAddress;
	DWORD nIndex;
	DWORD nSelect;

	//获取选中的项所在列表的行数
	nSelect = m_list_asm.GetSelectionItem();
	//获取断点地址
	nTempAddress = m_list_asm.GetItemText(nSelect, m_Flag_Address);
	//过滤掉地址后面可能出现的符号
	nTempAddress = nTempAddress.Left(8);
	_stscanf_s(nTempAddress, TEXT("%08X"), &nBreakAddress);

	//检测是否已经是断点
	nIndex = IsBreakPoint(nBreakAddress);
	//如果不是,就将其设置为断点，信息加入断点数组中
	if (nIndex == -1)
	{
		g_OK_ConditionBreakPoint = FALSE;
		ZeroMemory(&g_RecvCondition, sizeof(CONDITION));
		ConditionBreakDlg nConditionBreakDlg;
		nConditionBreakDlg.DoModal();

		//如果点的取消按钮就返回
		if (!g_OK_ConditionBreakPoint)return;

		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT("●"));
		SetBreakPoint(nBreakAddress, TRUE, g_RecvCondition);
		SetItemColor(nSelect, m_Flag_Break, COLOR_WHITE, COLOR_RED,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_WHITE, COLOR_RED,AsmBlock);
	}
	//如果是，就删除该地址处断点
	else
	{
		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT(""));
		DelBreakPoint(nBreakAddress);
		SetItemColor(nSelect, m_Flag_Break, COLOR_BLACK, COLOR_WHITE,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_BLACK, COLOR_WHITE,AsmBlock);
	}
}

// 设置硬件断点
void CXB32DbgDlg::OnHardbreak()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nBreakAddress;
	CString nTempAddress;
	DWORD nIndex;
	DWORD nSelect;
	BOOL nRet;

	//获取选中的项所在列表的行数
	nSelect = m_list_asm.GetSelectionItem();
	//获取断点地址
	nTempAddress = m_list_asm.GetItemText(nSelect, m_Flag_Address);
	//过滤掉地址后面可能出现的符号
	nTempAddress = nTempAddress.Left(8);
	_stscanf_s(nTempAddress, TEXT("%08X"), &nBreakAddress);

	//检测是否已经是断点
	nIndex = IsHardBreakPoint(nBreakAddress);
	//如果不是,就将其设置为断点，信息加入断点数组中
	if (nIndex == -1)
	{
		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT("■"));
		nRet = SetHardBreakPoint(nBreakAddress, FALSE, {});
		if (!nRet)
		{
			MessageBox(TEXT("硬件断点失败！"), TEXT("失败！"), MB_ICONERROR);
			return;
		}
		SetItemColor(nSelect, m_Flag_Break, COLOR_WHITE, COLOR_BLACK,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_WHITE, COLOR_BLACK,AsmBlock);

	}
	//如果是，就删除该地址处断点
	else
	{
		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT(""));
		DelHardBreakPoint(nBreakAddress);
		SetItemColor(nSelect, m_Flag_Break, COLOR_BLACK, COLOR_WHITE,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_BLACK, COLOR_WHITE,AsmBlock);
	}
}

// 设置硬件条件断点
void CXB32DbgDlg::OnConditionhardbreak()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nBreakAddress;
	CString nTempAddress;
	DWORD nIndex;
	DWORD nSelect;
	BOOL nRet;

	//获取选中的项所在列表的行数
	nSelect = m_list_asm.GetSelectionItem();
	//获取断点地址
	nTempAddress = m_list_asm.GetItemText(nSelect, m_Flag_Address);
	//过滤掉地址后面可能出现的符号
	nTempAddress = nTempAddress.Left(8);
	_stscanf_s(nTempAddress, TEXT("%08X"), &nBreakAddress);

	//检测是否已经是断点
	nIndex = IsHardBreakPoint(nBreakAddress);
	//如果不是,就将其设置为断点，信息加入断点数组中
	if (nIndex == -1)
	{
		g_OK_ConditionBreakPoint = FALSE;
		ZeroMemory(&g_RecvCondition, sizeof(CONDITION));
		ConditionBreakDlg nConditionBreakDlg;
		nConditionBreakDlg.DoModal();

		if (!g_OK_ConditionBreakPoint)return;

		nRet = SetHardBreakPoint(nBreakAddress, TRUE, g_RecvCondition);
		if (!nRet)
		{
			MessageBox(TEXT("硬件断点失败！"), TEXT("失败！"), MB_ICONERROR);
			return;
		}
		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT("■"));
		SetItemColor(nSelect, m_Flag_Break, COLOR_WHITE, COLOR_BLACK,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_WHITE, COLOR_BLACK,AsmBlock);
	}
	else
	{
		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT(""));
		DelHardBreakPoint(nBreakAddress);
		SetItemColor(nSelect, m_Flag_Break, COLOR_BLACK, COLOR_WHITE,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_BLACK, COLOR_WHITE,AsmBlock);
	}
}

// 设置内存执行断点
void CXB32DbgDlg::OnMemoryBreak()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nBreakAddress;
	CString nTempAddress;
	BOOL nIndex;
	DWORD nSelect;

	//获取选中的项所在列表的行数
	nSelect = m_list_asm.GetSelectionItem();
	//获取断点地址
	nTempAddress = m_list_asm.GetItemText(nSelect, m_Flag_Address);
	//过滤掉地址后面可能出现的符号
	nTempAddress = nTempAddress.Left(8);
	_stscanf_s(nTempAddress, TEXT("%08X"), &nBreakAddress);

	//检测是否已经是断点
	nIndex = IsMemoryBreakPoint(nBreakAddress);
	//如果已经存在内存断点
	if (HaveMemoryBreakPoint() && nIndex == FALSE)
	{
		MessageBox(TEXT("已存在内存断点！断点失败！"), TEXT("失败！"), MB_ICONERROR);
		return;
	}
	//如果不存在内存断点
	if (!HaveMemoryBreakPoint())
	{
		//设置内存断点
		SetMemoryBreakPoint(nBreakAddress, 8, FALSE, {});

		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT("▲"));
		SetItemColor(nSelect, m_Flag_Break, COLOR_WHITE, COLOR_ORANGE,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_WHITE, COLOR_ORANGE,AsmBlock);

	}
	else
	{
		DelMemoryBreakPoint(nBreakAddress);

		m_list_asm.SetItemText(nSelect, m_Flag_Break, TEXT(""));
		SetItemColor(nSelect, m_Flag_Break, COLOR_BLACK, COLOR_WHITE,AsmBlock);
		SetItemColor(nSelect, m_Flag_Address, COLOR_BLACK, COLOR_WHITE,AsmBlock);
	}
}

// 设置硬件写入断点 (1字节)
void CXB32DbgDlg::OnHardwrite1()
{
	// TODO: 在此添加命令处理程序代码
	SetHardReadWriteProc(1, 0);
}

// 设置硬件写入断点 (2字节)
void CXB32DbgDlg::OnHardwrite2()
{
	// TODO: 在此添加命令处理程序代码
	SetHardReadWriteProc(1, 1);
}

// 设置硬件写入断点 (4字节)
void CXB32DbgDlg::OnHardwrite4()
{
	// TODO: 在此添加命令处理程序代码
	SetHardReadWriteProc(1, 3);
}

// 设置硬件读写断点 (1字节)
void CXB32DbgDlg::OnHardread1()
{
	// TODO: 在此添加命令处理程序代码
	SetHardReadWriteProc(3, 0);
}

// 设置硬件读写断点 (2字节)
void CXB32DbgDlg::OnHardread2()
{
	// TODO: 在此添加命令处理程序代码
	SetHardReadWriteProc(3, 1);
}

// 设置硬件读写断点 (4字节)
void CXB32DbgDlg::OnHardread4()
{
	// TODO: 在此添加命令处理程序代码
	SetHardReadWriteProc(3, 3);
}

// 设置内存写入断点
void CXB32DbgDlg::OnMemoryWrite()
{
	// TODO: 在此添加命令处理程序代码
	SetMemoryReadWriteProc(1);
}

// 设置内存读取断点
void CXB32DbgDlg::OnMemoryRead()
{
	// TODO: 在此添加命令处理程序代码
	SetMemoryReadWriteProc(0);
}

//跳转到指定地址(内存窗口)
void CXB32DbgDlg::OnGotomemory()
{
	// TODO: 在此添加命令处理程序代码

	CString szAddress;
	szAddress = m_list_mem.GetItemText(m_nTempItem, m_Flag_MemAddress);
	_stscanf_s(szAddress, TEXT("%X"), &g_SendAddress_Memory);
	g_SendAddress_Memory = g_SendAddress_Memory + m_nTempSubItem - 1;

	g_Ok_Memory = FALSE;
	GotoMemDlg nGotoMemoryDlg;
	nGotoMemoryDlg.DoModal();
	if (!g_Ok_Memory)return;

	ShowMemory(g_RecvAddress_Memory);
	m_list_mem.SetFocus();
	m_list_mem.SetSelectionEx(0);
}

//内存框单击右键
void CXB32DbgDlg::OnNMRClickListMemory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取单击选择的行数列数
	DWORD nItem = pNMItemActivate->iItem;
	DWORD nSubItem = pNMItemActivate->iSubItem;

	if (nItem == -1)return;

	CMenu *nMenu = m_Menu_Main.GetSubMenu(2);
	POINT pos;
	GetCursorPos(&pos);


	if (nSubItem == m_Flag_MemAddress || nSubItem == m_Flag_MemStr)
	{
		//nMenu->EnableMenuItem(ID_EditMemory, TRUE);
		nMenu->EnableMenuItem(ID_32795, TRUE);
		nMenu->EnableMenuItem(ID_32796, TRUE);
		nMenu->EnableMenuItem(ID_32797, TRUE);
		nMenu->EnableMenuItem(ID_32798, TRUE);
		nMenu->EnableMenuItem(ID_32799, TRUE);
		nMenu->EnableMenuItem(ID_32800, TRUE);
	}
	else
	{
		//nMenu->EnableMenuItem(ID_EditMemory, FALSE);
		nMenu->EnableMenuItem(ID_32795, FALSE);
		nMenu->EnableMenuItem(ID_32796, FALSE);
		nMenu->EnableMenuItem(ID_32797, FALSE);
		nMenu->EnableMenuItem(ID_32798, FALSE);
		nMenu->EnableMenuItem(ID_32799, FALSE);
		nMenu->EnableMenuItem(ID_32800, FALSE);
	}

	//保存选中的行数列数
	m_nTempItem = nItem;
	m_nTempSubItem = nSubItem;
	nMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
}

// 处理内存框高亮
void CXB32DbgDlg::OnNMCustomdrawListMemory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	LPNMCUSTOMDRAW nmCustomDraw = &pNMCD->nmcd;

	switch (nmCustomDraw->dwDrawStage)
	{
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
	{

		if (m_list_mem.IsSelectionItem(nmCustomDraw->dwItemSpec))
		{
			if (pNMCD->iSubItem == m_nTempSubItem)
			{
				pNMCD->clrText = COLOR_WHITE;
				pNMCD->clrTextBk = COLOR_BLUE;
				nmCustomDraw->uItemState = 0;
				*pResult = CDRF_NEWFONT;
				return;
			}

			pNMCD->clrText = COLOR_BLACK;
			pNMCD->clrTextBk = COLOR_WHITE;
			nmCustomDraw->uItemState = 0;
			*pResult = CDRF_NEWFONT;
			return;
		}

		break;
	}
	}

	*pResult = 0;
	*pResult |= CDRF_NOTIFYPOSTPAINT;		//必须有，不然就没有效果
	*pResult |= CDRF_NOTIFYITEMDRAW;		//必须有，不然就没有效果}
}

//汇编框单击右键
void CXB32DbgDlg::OnNMRClickListAsm(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取单击选择的行数列数
	DWORD nItem = pNMItemActivate->iItem;
	DWORD nSubItem = pNMItemActivate->iSubItem;

	if (nItem == -1)return;

	CMenu *nMenu = m_Menu_Main.GetSubMenu(1);
	POINT pos;
	GetCursorPos(&pos);
	nMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
}

//汇编框双击左键
void CXB32DbgDlg::OnNMDblclkListAsm(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	if (pNMItemActivate->iItem == -1 ||
		(pNMItemActivate->iSubItem != m_Flag_Asm))return;

	//修改ASM代码
	OnChangeasm();
}

//寄存器框双击左键
void CXB32DbgDlg::OnNMDblclkListReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	if (pNMItemActivate->iItem == -1 ||
		(pNMItemActivate->iSubItem != 1))return;


	//修改寄存器的值
	OnChangeReg();

}

//内存框双击左键
void CXB32DbgDlg::OnNMDblclkListMemory(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取选定的数据所在列表的行和列
	DWORD nItem = pNMItemActivate->iItem;
	DWORD nSubItem = pNMItemActivate->iSubItem;

	if (nItem == -1 || nSubItem == m_Flag_MemAddress || nSubItem == m_Flag_MemStr)return;

	//修改数据
	EditMemoryProc(nItem, nSubItem);
}

//堆栈框双击左键
void CXB32DbgDlg::OnNMDblclkListStack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取选定的数据所在列表的行和列
	DWORD nItem = pNMItemActivate->iItem;
	DWORD nSubItem = pNMItemActivate->iSubItem;

	//选中无效区域直接返回
	if (nItem == -1)return;

	CString nTemp;
	DWORD nReadAddress;
	DWORD nReadOffset;

	//如果选中的是地址栏
	if (nSubItem == m_Flag_StackAddress)
	{
		//获取地址
		nTemp = m_list_stack.GetItemText(nItem, m_Flag_StackAddress);
		_stscanf_s(nTemp, TEXT("%X+%X"), &nReadAddress, &nReadOffset);

		//根据地址重新显示内存数据
		ShowMemory(nReadAddress + nReadOffset);
		m_list_mem.SetFocus();
		m_list_mem.SetSelectionEx(0);
	}
	//如果选中的是数据栏
	else if (nSubItem == m_Flag_StackData)
	{
		//获取数据
		nTemp = m_list_stack.GetItemText(nItem, m_Flag_StackData);
		//如果数据为空，直接返回
		if (nTemp.IsEmpty())return;

		//将数据转换为十六进制
		_stscanf_s(nTemp, TEXT("%X"), &nReadAddress);

		//将数据作为地址重新显示汇编框数据
		InitAsm(nReadAddress);
		m_list_asm.SetFocus();
		m_list_asm.SetSelectionEx(0);
	}
}

//寄存器框单击右键
void CXB32DbgDlg::OnNMRClickListReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取选定的数据所在列表的行和列
	DWORD nItem = pNMItemActivate->iItem;

	//如果不是调用寄存器或者EIP就直接返回
	if (nItem == -1||nItem>8)return;

	CMenu *nMenu = m_Menu_Secondary.GetSubMenu(0);
	POINT pos;
	GetCursorPos(&pos);
	nMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);

}

//在内存窗口中转到
void CXB32DbgDlg::OnJumpToMemShow()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nAddress;
	CString regValue;
	DWORD nSelect;

	//获取选中的项所在列表的行数
	nSelect = m_list_reg.GetSelectionItem();
	//获取寄存器的值
	regValue = m_list_reg.GetItemText(nSelect, 1);
	//将寄存器的值转换为十六进制，作为要转到的地址
	_stscanf_s(regValue, TEXT("%08X"), &nAddress);

	//根据地址重新显示内存数据
	ShowMemory(nAddress);
	m_list_mem.SetFocus();
	m_list_mem.SetSelectionEx(0);
}

//在栈中转到
void CXB32DbgDlg::OnJumpToStackShow()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nAddress;
	CString regValue;
	DWORD nSelect;

	//获取选中的项所在列表的行数
	nSelect = m_list_reg.GetSelectionItem();
	//获取寄存器的值
	regValue = m_list_reg.GetItemText(nSelect, 1);
	//将寄存器的值转换为十六进制，作为要转到的地址
	_stscanf_s(regValue, TEXT("%08X"), &nAddress);

	//显示栈区信息
	ShowStack(nAddress);
	m_list_stack.SetFocus();
	m_list_stack.SetSelectionEx(0);
}

//在反汇编窗口中转到
void CXB32DbgDlg::OnJumpToAsmShow()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nAddress;
	CString regValue;
	DWORD nSelect;

	//获取选中的项所在列表的行数
	nSelect = m_list_reg.GetSelectionItem();
	//获取寄存器的值
	regValue = m_list_reg.GetItemText(nSelect, 1);
	//将寄存器的值转换为十六进制，作为要转到的地址
	_stscanf_s(regValue, TEXT("%08X"), &nAddress);

	//将数据作为地址重新显示汇编框数据
	InitAsm(nAddress);
	m_list_asm.SetFocus();
	m_list_asm.SetSelectionEx(0);
}

//跳转到指定地址(返汇编窗口)
void CXB32DbgDlg::OnGotoAsm()
{
	// TODO: 在此添加命令处理程序代码

	DWORD nSelect;

	//获取选中的项所在列表的行数
	nSelect = m_list_asm.GetSelectionItem();

	//获取当前选定项的地址
	CString szAddress;
	szAddress = m_list_asm.GetItemText(nSelect, m_Flag_Address);
	//过滤掉地址后面可能出现的符号
	szAddress = szAddress.Left(8);
	_stscanf_s(szAddress, TEXT("%X"), &g_SendAddress_Asm);

	g_Ok_Asm = FALSE;
	GoToAsmDlg nGotoAsmDlg;
	nGotoAsmDlg.DoModal();
	if (!g_Ok_Asm)return;

	InitAsm(g_RecvAddress_Asm);
	m_list_asm.SetFocus();
	m_list_asm.SetSelectionEx(0);
}

//查看模块按钮
void CXB32DbgDlg::OnShowModule()
{
	// TODO: 在此添加命令处理程序代码

	ModuleDlg nModuleDlg;
	nModuleDlg.DoModal();

	if (!g_RecvAddress_Module)return;

	InitAsm(g_RecvAddress_Module);
	m_list_asm.SetFocus();
	m_list_asm.SetSelectionEx(0);
}

//导入导出表按钮
void CXB32DbgDlg::OnShowImportExportedTable()
{
	// TODO: 在此添加命令处理程序代码

	TableDlg *nTableDlg = TableDlg::GetTableDlg();
	nTableDlg->ShowWindow(TRUE);
}

void CXB32DbgDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (nIDEvent == TABLECLOCK)
	{
		SetTimer(nIDEvent, -1, NULL);
		TableDlg *nTableDlg = TableDlg::GetTableDlg();
	}

	CDialogEx::OnTimer(nIDEvent);
}

//拦截消息
BOOL CXB32DbgDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	//如果按下回车键
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (pMsg->hwnd == m_list_asm.m_hWnd)OnInaddress();
		return FALSE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CXB32DbgDlg::OnAntiNtQueryInformationProcess()
{
	// TODO: 在此添加命令处理程序代码
	m_bChecked_AntiNtQInforPro = !m_bChecked_AntiNtQInforPro;
}

void CXB32DbgDlg::OnAntiPeb()
{
	// TODO: 在此添加命令处理程序代码
	m_bChecked_AntiPeb = !m_bChecked_AntiPeb;
}

void CXB32DbgDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: 在此处添加消息处理程序代码

	if (!bSysMenu) {
		//查看弹出菜单中是否包含 选中的项。
		int nCount = pPopupMenu->GetMenuItemCount();
		for (int i = 0; i < nCount; i++)
		{
			if (pPopupMenu->GetMenuItemID(i) == ID_32809)
			{
				pPopupMenu->CheckMenuItem(ID_32809, MF_BYCOMMAND | (m_bChecked_AntiPeb ? MF_CHECKED : MF_UNCHECKED));
			}
			if (pPopupMenu->GetMenuItemID(i) == ID_32810)
			{
				pPopupMenu->CheckMenuItem(ID_32810, MF_BYCOMMAND | (m_bChecked_AntiNtQInforPro ? MF_CHECKED : MF_UNCHECKED));
			}
		}
	}
}

