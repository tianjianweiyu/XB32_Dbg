
// XB32_DbgDlg.h: 头文件
//

#pragma once

//反汇编引擎头文件，静态库
#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine_4.1//Win32//headers//BeaEngine.h"
#pragma comment (lib , "BeaEngine_4.1/Win32/Win32/Lib/BeaEngine.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"crt.lib\"")

#include "ClistCtrlEx.h"
#include "MyHead.h"
#include "LoadPlugin.h"

//附加进程的ID消息
#define WM_AttachProcessId WM_USER+100
//区别反汇编区与寄存器区
#define AsmBlock 0
#define RegBlock 1

//标志寄存器
typedef struct _EFLAGS
{
	BYTE CF : 1;
	BYTE NULL1 : 1;
	BYTE PF : 1;
	BYTE NULL2 : 1;
	BYTE AF : 1;
	BYTE NULL3 : 1;
	BYTE ZF : 1;
	BYTE SF : 1;
	BYTE TF : 1;
	BYTE IF : 1;
	BYTE DF : 1;
	BYTE OF : 1;
	BYTE NULL4 : 4;
}EFLAGS, *PEFLAGS;

//软件断点信息
typedef struct _BREAKPOINT
{
	DWORD nAddress;			//断点地址
	BYTE nData;				//原数据
	BOOL nIsCondition;		//条件断点标志
	CONDITION nCondition;	//条件断点使用的寄存器与值
}BREAKPOINT, *PBREAKPOINT;

//硬件断点信息
typedef struct _HARDBREAKPOINT
{
	DWORD nAddress;			//断点地址
	DWORD nType;			//断点类型
	DWORD nSize;			//断点长度
	BOOL nIsCondition;		//条件断点标志
	CONDITION nCondition;	//条件断点使用的寄存器与值
}HARDBREAKPOINT, *PHARDBREAKPOINT;

//内存断点信息
typedef struct _MEMORYBREAKPOINT
{
	DWORD nAddress;			//断点地址
	DWORD nOldPageProtect;	//原来页保护属性
	DWORD nType;			//断点类型
	BOOL nIsCondition;		//条件断点标志
	CONDITION nCondition;	//条件断点使用的寄存器与值
}MEMORYBREAKPOINT, *PMEMORYBREAKPOINT;

typedef struct _LISTCOLOR
{
	DWORD Item;
	DWORD SubItem;
	COLORREF TextColor;
	COLORREF BkColor;
}LISTCOLOR, *PLISTCOLOR;

typedef struct _DR6 {
	/*
	//     断点命中标志位，如果位于DR0~3的某个断点被命中，则进行异常处理前，对应
	// 的B0~3就会被置为1。
	*/
	unsigned Dr0 : 1;  // Dr0断点触发置位
	unsigned Dr1 : 1;  // Dr1断点触发置位
	unsigned Dr2 : 1;  // Dr2断点触发置位
	unsigned Dr3 : 1;  // Dr3断点触发置位
					  /*
					  // 保留字段
					  */
	unsigned Reserve1 : 9;
	/*
	// 其它状态字段
	*/
	unsigned BD : 1;  // 调制寄存器本身触发断点后，此位被置为1
	unsigned BS : 1;  // 单步异常被触发，需要与寄存器EFLAGS的TF联合使用
	unsigned BT : 1;  // 此位与TSS的T标志联合使用，用于接收CPU任务切换异常
					  /*
					  // 保留字段
					  */
	unsigned Reserve2 : 16;
}DR6, *PDR6;

typedef struct _DR7
{
	unsigned Dr0_L : 1;		//1
	unsigned Dr0_G : 1;
	unsigned Dr1_L : 1;
	unsigned Dr1_G : 1;
	unsigned Dr2_L : 1;
	unsigned Dr2_G : 1;
	unsigned Dr3_L : 1;
	unsigned Dr3_G : 1;

	unsigned NULL1 : 8;		//保留

	unsigned Dr0_RWE : 2;	//00:只执行 01:写入数据断点 10:I/O端口断点 11:读或写数据断点
	unsigned Dr0_LEN : 2;	//00:1字节 01:2字节 10:保留 11:4字节
	unsigned Dr1_RWE : 2;
	unsigned Dr1_LEN : 2;
	unsigned Dr2_RWE : 2;
	unsigned Dr2_LEN : 2;
	unsigned Dr3_RWE : 2;
	unsigned Dr3_LEN : 2;

}DR7, *PDR7;

typedef struct _BREAKREADWRITE
{
	DWORD nAddress;
	CString nAsm;
	DWORD nIsHardOrMem;	//硬件断点与内存断点区别标志，为1是硬件,2是内存
}BREAKREADWRITE, *PBREAKREADWRITE;

typedef struct _BEANALYZEDADDRESSINFO
{
	CString nAddress;	//指令地址
	CString nBeAnalyzedAddress;	//被解析的地址
	BOOL bFlag;		//为TRUE时表示被解析的地址就是要跳转的地址，
					//为FALSE时表示被解析的地址处保存着要跳转的地址
	CString Symbol;	//被解析成的符号
}BEANALYZEDADDRESSINFO, *PBEANALYZEDADDRESSINFO;

// CXB32DbgDlg 对话框
class CXB32DbgDlg : public CDialogEx
{
// 构造
public:
	CXB32DbgDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_XB32_DBG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	//保存被调试进程的进程句柄和线程句柄
	HANDLE m_ProcessHandle = nullptr;
	HANDLE m_ThreadHandle = nullptr;
	//保存被调试进程的路径
	CString m_strFilePath;
	//保存被调试进程的ID(用于区别是附加进程还是创建进程)
	DWORD m_nPid = 0;
	//保存被调试进程的PID
	DWORD m_Pid = 0;
	//用于保存调试事件的结构体
	DEBUG_EVENT m_dbgEvent = { 0 };

	//列表控件变量
	ClistCtrlEx m_list_reg;
	ClistCtrlEx m_list_asm;
	ClistCtrlEx m_list_mem;
	ClistCtrlEx m_list_stack;

	//一次显示添加多少列
	DWORD m_Num_Asm = 27;
	DWORD m_Num_Stack = 128;
	DWORD m_Num_Memory = 20;

	//记录永久软件断点动态数组
	vector<BREAKPOINT> m_BreakPoint;
	//记录临时(一次性)软件断点,用来单步步过的
	vector<BREAKPOINT> m_TempBreakPoint;
	//记录永久硬件断点数组
	HARDBREAKPOINT m_HardBreakPoint[4]{};
	//记录读写断点
	vector<BREAKREADWRITE>m_BreakReadWrite;
	//记录一次性硬件断点数组
	DWORD m_TempHardBreakPoint[4]{};
	//记录内存断点，内存断点只允许存在一个
	MEMORYBREAKPOINT m_MemoryBreakPoint = {};

	//记录需要高亮的反汇编指令信息的动态数组
	vector<LISTCOLOR> m_AsmColor;
	//记录需要高亮的寄存器信息的动态数组
	vector<LISTCOLOR> m_RegColor;

	//颜色
	COLORREF COLOR_WHITE = RGB(255, 255, 255);
	COLORREF COLOR_BLACK = RGB(0, 0, 0);
	COLORREF COLOR_RED = RGB(255, 0, 0);
	COLORREF COLOR_ORANGE = RGB(255, 153, 0);
	COLORREF COLOR_GREEN = RGB(0, 178, 102);
	COLORREF COLOR_BLUE = RGB(51, 153, 255);

	//反汇编窗口列数
	DWORD m_Flag_Break = 0;
	DWORD m_Flag_Address = 1;
	DWORD m_Flag_Opcode = 2;
	DWORD m_Flag_Asm = 3;
	DWORD m_Flag_Mark = 4;
	//堆栈窗口列数
	DWORD m_Flag_StackAddress = 0;
	DWORD m_Flag_StackData = 1;
	//内存数据窗口列数
	DWORD m_Flag_MemAddress = 0;
	DWORD m_Flag_MemStr = 17;

	//保存右键单击内存框中内容所在行数/列数
	DWORD m_nTempItem = 0;
	DWORD m_nTempSubItem = 0;

	//菜单变量
	CMenu m_Menu_Main;
	CMenu m_Menu_Secondary;

	vector<TABLE> m_Table;
	//计时器标识符
	DWORD TABLECLOCK = 0x999;

	//反反调试菜单下的按键是否被选定的标志
	BOOL m_bChecked_AntiPeb = FALSE;
	BOOL m_bChecked_AntiNtQInforPro = FALSE;

	//插件
	LoadPlugin m_LoadPlugin;

	//保存被解析成符号的地址
	vector<BEANALYZEDADDRESSINFO> m_BeAanlyAddress;

	afx_msg void OnCreateProcess();
	afx_msg void OnDebugActiveProcess();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMCustomdrawListAsm(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawListReg(NMHDR *pNMHDR, LRESULT *pResult);

	//自定义消息，用于接收附加进程的id和路径
	afx_msg LRESULT OnAttachprocessid(WPARAM wParam, LPARAM lParam);

public:

	/*!
	*  函 数 名： MyRtlAdjustPrivilege
	*  日    期： 2020/06/14
	*  返回类型： VOID
	*  功    能： 提升权限
	*/
	VOID MyRtlAdjustPrivilege();

	/*!
	*  函 数 名： Open
	*  日    期： 2020/06/06
	*  返回类型： BOOL
	*  参    数： const TCHAR * pszFile 进程路径
	*  功    能： 以调试的方式创建进程
	*/
	BOOL Open(const TCHAR* pszFile);

	/*!
	*  函 数 名： Open
	*  日    期： 2020/06/06
	*  返回类型： BOOL
	*  参    数： DWORD nPid 进程ID
	*  功    能： 附加进程
	*/
	BOOL Open(DWORD nPid);

	/*!
	*  函 数 名： GetProcessThreadId
	*  日    期： 2020/06/06
	*  返回类型： DWORD
	*  参    数： DWORD nPid 进程ID
	*  功    能： 获取指定进程的线程ID
	*/
	DWORD GetProcessThreadId(DWORD nPid);

	/*!
	*  函 数 名： SetListAttr
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  功    能： 设置List控件属性
	*/
	VOID SetListAttr();

	/*!
	*  函 数 名： SetPluginMenu
	*  日    期： 2020/06/17
	*  返回类型： VOID
	*  功    能： 设置插件菜单名字
	*/
	VOID SetPluginMenu();

	/*!
	*  函 数 名： LoadSymbol
	*  日    期： 2020/06/17
	*  返回类型： BOOL
	*  参    数： CREATE_PROCESS_DEBUG_INFO * pInfo 创建进程调试结构体
	*  功    能： 载入符号文件
	*/
	BOOL LoadSymbol(CREATE_PROCESS_DEBUG_INFO* pInfo);

	/*!
	*  函 数 名： DispatchEvent
	*  日    期： 2020/06/06
	*  返回类型： DWORD
	*  功    能： 对调试事件进行分类，异常事件继续派发，其余事件自己处理
	*/
	DWORD DispatchEvent();

	/*!
	*  函 数 名： DispatchException
	*  日    期： 2020/06/06
	*  返回类型： DWORD
	*  功    能： 处理异常事件
	*/
	DWORD DispatchException();

	/*!
	*  函 数 名： ShowContext
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： CONTEXT ct
	*  功    能： 显示被调试进程当前寄存器的值
	*/
	VOID ShowContext(CONTEXT ct);

	/*!
	*  函 数 名： InitInformationEx
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： DWORD nImageBase 加载基址
	*  功    能： 初始化寄存器,内存信息
	*/
	VOID InitInformationEx(DWORD nImageBase);

	/*!
	*  函 数 名： InitAsm
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： DWORD nReadAddress 要反汇编的数据的首地址
	*  功    能： 初始化反汇编指令
	*/
	VOID InitAsm(DWORD nReadAddress);

	/*!
	*  函 数 名： ClearItemColor
	*  日    期： 2020/06/07
	*  返回类型： VOID
	*  功    能： 清除反汇编区高亮颜色
	*/
	VOID ClearItemColor();

	/*!
	*  函 数 名： GetOepAddressEx
	*  日    期： 2020/06/06
	*  返回类型： DWORD
	*  参    数： DWORD nImageBase 模块加载基址
	*  功    能： 取出指定模块的OEP地址
	*/
	DWORD GetOepAddressEx(DWORD nImageBase);

	/*!
	*  函 数 名： ReadMemoryBytes
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： DWORD nAddress 要读取数据的首地址
	*  参    数： LPBYTE nValue 接收读取的数据
	*  参    数： DWORD nLen 要读取数据的大小，单位字节
	*  功    能： 读取被被调试进程指定地址指定字节的数据
	*/
	VOID ReadMemoryBytes(DWORD nAddress, LPBYTE nValue, DWORD nLen);

	/*!
	*  函 数 名： PrintBreakFlag
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  参    数： DWORD nItem 所在列表控件行数
	*  功    能： 刷新显示断点的高亮标志
	*/
	VOID PrintBreakFlag(DWORD nBreakAddress, DWORD nItem);

	/*!
	*  函 数 名： IsBreakPoint
	*  日    期： 2020/06/06
	*  返回类型： DWORD	不是返回-1，是返回数组下标
	*  参    数： DWORD nBreakAddress 待检测的地址
	*  功    能： 检测是否为永久软件断点地址
	*/
	DWORD IsBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： SetItemColor
	*  日    期： 2020/06/07
	*  返回类型： VOID
	*  参    数： DWORD Item 行
	*  参    数： DWORD SubItem 列
	*  参    数： COLORREF TextColor 字体颜色
	*  参    数： COLORREF BkColor 背景颜色
	*  参    数： DWORD Flag 标志 区别反汇编区与寄存器区 AsmBlock为反汇编区 RegBlock为寄存器区
	*  功    能：  将需要高亮的数据信息存入动态数组中
	*/
	VOID SetItemColor(DWORD Item, DWORD SubItem, COLORREF TextColor, COLORREF BkColor, DWORD Flag);

	/*!
	*  函 数 名： ShowExportName
	*  日    期： 2020/06/17
	*  返回类型： CString
	*  参    数： CString szAddress 地址
	*  功    能： 遍历地址中的符号
	*/
	CString ShowExportName(CString szAddress);

	/*!
	*  函 数 名： ShowAsmName
	*  日    期： 2020/06/17
	*  返回类型： CString
	*  参    数： CString szAddress Asm代码所在地址
	*  参    数： CString szAsm Asm代码
	*  功    能： 遍历显示Asm代码中的符号
	*/
	CString ShowAsmName(CString szAddress, CString szAsm);

	/*!
	*  函 数 名： IsHardBreakPoint
	*  日    期： 2020/06/06
	*  返回类型： DWORD	不是返回-1，是返回数组下标
	*  参    数： DWORD nBreakAddress 待检测的地址
	*  功    能： 检测是否为永久硬件断点地址
	*/
	DWORD IsHardBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： IsHardBreakPointEx
	*  日    期： 2020/06/12
	*  返回类型： DWORD 不是返回-1，是返回数组下标
	*  功    能： 检测是否在读写硬件读写断点地址上的内容
	*/
	DWORD IsHardBreakPointEx();

	/*!
	*  函 数 名： IsTempHardBreakPoint
	*  日    期： 2020/06/08
	*  返回类型： DWORD	不是返回-1，是返回数组下标
	*  参    数： DWORD nBreakAddress 待检测的地址
	*  功    能： 检测是否为一次性硬件断点地址
	*/
	DWORD IsTempHardBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： IsConditionHardBreakPoint
	*  日    期： 2020/06/08
	*  返回类型： DWORD
	*  参    数： DWORD nBreakPointIndex 断点在永久断点数组中的下标
	*  功    能： 检测是否为条件断点(硬件)
	*/
	BOOL IsConditionHardBreakPoint(DWORD nBreakPointIndex);

	/*!
	*  函 数 名： IsConditionSucessHardBreakPoint
	*  日    期： 2020/06/08
	*  返回类型： BOOL
	*  参    数： DWORD nBreakPointIndex 断点在永久断点数组中的下标 
	*  参    数： CONTEXT nContext 断点时候的线程上下文
	*  参    数： BOOL nIsCondition 条件断点的标志,由IsConditionHardBreakPoint获得
	*  功    能： 检测条件断点是否执行到条件成立时(硬件)
	*/
	BOOL IsConditionSucessHardBreakPoint(DWORD nBreakPointIndex, CONTEXT nContext, BOOL nIsCondition);

	/*!
	*  函 数 名： IsMemoryBreakPoint
	*  日    期： 2020/06/06
	*  返回类型： BOOL
	*  参    数： DWORD nBreakAddress 待检测的地址
	*  功    能： 检测是否为内存断点地址
	*/
	BOOL IsMemoryBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： PrintOpcode
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： BYTE * pOpcode opcode地址
	*  参    数： DWORD nSize opcode数据大小，单位字节
	*  参    数： DWORD nItem 第几行
	*  功    能： 显示Opcode
	*/
	VOID ShowOpcode(BYTE* pOpcode, DWORD nSize, DWORD nItem);

	/*!
	*  函 数 名： IsColorWord
	*  日    期： 2020/06/06
	*  返回类型： BOOL
	*  参    数： CString nStr 指令
	*  参    数： LPDWORD nTextColor 返回字体颜色
	*  参    数： LPDWORD nBkColor 返回背景颜色
	*  功    能： 是否为高亮的指令
	*/
	BOOL IsColorWord(CString nStr, LPDWORD nTextColor, LPDWORD nBkColor);

	/*!
	*  函 数 名： ShowMemory
	*  日    期： 2020/06/06
	*  返回类型： VOID
	*  参    数： DWORD nAddress 要显示的内存数据的首地址
	*  功    能： 显示内存数据
	*/
	VOID ShowMemory(DWORD nAddress);

	/*!
	*  函 数 名： IsTempBreakPoint
	*  日    期： 2020/06/07
	*  返回类型： DWORD	不是返回-1，是返回数组下标
	*  参    数： DWORD nBreakAddress 待检测的地址
	*  功    能： 检测是否为临时(一次性)软件断点地址
	*/
	DWORD IsTempBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： IsConditionBreakPoint
	*  日    期： 2020/06/07
	*  返回类型： BOOL
	*  参    数： DWORD nBreakPointIndex 断点在永久断点数组中的下标
	*  功    能： 检测是否为条件断点(软件)
	*/
	BOOL IsConditionBreakPoint(DWORD nBreakPointIndex);

	/*!
	*  函 数 名： IsConditionSucessBreakPoint
	*  日    期： 2020/06/07
	*  返回类型： BOOL
	*  参    数： DWORD nBreakPointIndex 断点在永久断点数组中的下标 
	*  参    数： CONTEXT nContext 断点时候的线程上下文
	*  参    数： BOOL nIsCondition 条件断点的标志,由IsConditionBreakPoint获得
	*  功    能： 检测条件断点是否执行到条件成立时(软件)
	*/
	BOOL IsConditionSucessBreakPoint(DWORD nBreakPointIndex, CONTEXT nContext, BOOL nIsCondition);

	/*!
	*  函 数 名： DelTempBreakPoint
	*  日    期： 2020/06/07
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  参    数： DWORD nTempBreakPointIndex 断点在一次性断点数组中的下标
	*  功    能： 恢复一次性断点并将断点信息从数组中删除
	*/
	VOID DelTempBreakPoint(DWORD nBreakAddress, DWORD nTempBreakPointIndex);

	/*!
	*  函 数 名： WriteMemoryByte
	*  日    期： 2020/06/07
	*  返回类型： VOID
	*  参    数： DWORD nAddress 要修改数据的地址
	*  参    数： BYTE nValue 修改后的值
	*  功    能： 修改被被调试进程指定地址1字节的数据
	*/
	VOID WriteMemoryByte(DWORD nAddress, BYTE nValue);

	/*!
	*  函 数 名： WriteMemoryBytes
	*  日    期： 2020/06/13
	*  返回类型： VOID
	*  参    数： DWORD nAddress 要修改数据的地址
	*  参    数： LPBYTE nValue 修改后的值
	*  参    数： DWORD nLen 修改数据的长度
	*  功    能： 修改被被调试进程指定地址指定长度的数据
	*/
	VOID WriteMemoryBytes(DWORD nAddress, LPBYTE nValue, DWORD nLen);

	/*!
	*  函 数 名： SetAllWindowsLong
	*  日    期： 2020/06/07
	*  返回类型： VOID
	*  功    能： 设置控件钩子
	*/
	VOID SetAllWindowsLong();

	/*!
	*  函 数 名： PrintOnceAsm
	*  日    期： 2020/06/07
	*  返回类型： VOID
	*  参    数： DWORD nReadAddress 要反汇编的数据的首地址
	*  功    能： 显示反汇编指令
	*/
	VOID PrintOnceAsm(DWORD nReadAddress);

	/*!
	*  函 数 名： RepairAsm
	*  日    期： 2020/06/08
	*  返回类型： VOID
	*  参    数： DWORD nReadAddress 要修复的地址
	*  参    数： DWORD nVirtualAddress 反汇编的虚拟地址
	*  参    数： BYTE * nRepairBytes 待修复opcode
	*  功    能： 修复int3断点的opcode
	*/
	VOID RepairAsm(DWORD nReadAddress, DWORD nVirtualAddress, BYTE *nRepairBytes);

	/*!
	*  函 数 名： GetBeforeAddress
	*  日    期： 2020/06/07
	*  返回类型： DWORD
	*  参    数： DWORD nReadAddress 当前地址
	*  功    能： 取出上一条地址
	*/
	DWORD GetBeforeAddress(DWORD nReadAddress);

	/*!
	*  函 数 名： SetBreakPoint
	*  日    期： 2020/06/08
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  参    数： BOOL nIsCondotion 条件断点标志，TRUE为条件断点，FALSE不是条件断点，为FALSE是参数nCondition必须为{}
	*  参    数： CONDITION nCondition 条件断点的条件信息
	*  功    能： 设置软件断点
	*/
	VOID SetBreakPoint(DWORD nBreakAddress, BOOL nIsCondotion, CONDITION nCondition);

	/*!
	*  函 数 名： DelBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 要删除的断点地址
	*  功    能： 删除指定断点(软件)
	*/
	VOID DelBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： CloseAllBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  功    能： 将所有永久软件断点关闭，即断点处的数据修复
	*/
	VOID CloseAllBreakPoint();

	/*!
	*  函 数 名： CloseOneBreakPoint
	*  日    期： 2020/06/10
	*  返回类型： BOOL	如果运行处是软件断点地址返回TRUE,否则返回FALSE
	*  参    数： DWORD nAddress 断点地址
	*  功    能： 将指定永久软件断点关闭，即断点处的数据修复
	*/
	BOOL CloseOneBreakPoint(DWORD nAddress);

	/*!
	*  函 数 名： OpenAllBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  功    能： 将所有永久软件断点重新打开，即断点处的数据写为0xCC
	*/
	VOID OpenAllBreakPoint();

	/*!
	*  函 数 名： OpenAllHardBreakPoint
	*  日    期： 2020/06/10
	*  返回类型： VOID
	*  功    能： 将所有硬件断点重新打开
	*/
	VOID OpenAllHardBreakPoint();

	/*!
	*  函 数 名： ShowSingleAsm
	*  日    期： 2020/06/08
	*  返回类型： VOID
	*  参    数： DWORD nShowAddress 待显示的地址
	*  功    能： 显示一行Asm
	*/
	VOID ShowSingleAsm(DWORD nShowAddress);

	/*!
	*  函 数 名： FindAddress
	*  日    期： 2020/06/08
	*  返回类型： DWORD
	*  参    数： DWORD nShowAddress 地址
	*  功    能： 查找地址所在列表框的位置
	*/
	DWORD FindAddress(DWORD nShowAddress);

	/*!
	*  函 数 名： ShowStack
	*  日    期： 2020/06/14
	*  返回类型： VOID
	*  参    数： DWORD nShowAddress 首地址
	*  功    能： 显示栈区数据
	*/
	VOID ShowStack(DWORD nShowAddress);

	/*!
	*  函 数 名： IsCallRepAsm
	*  日    期： 2020/06/09
	*  返回类型： BOOL 是就返回TRUE 不是返回FASLE
	*  参    数： DWORD nAddress 内存地址
	*  功    能： 判断是不是call与rep指令
	*/
	BOOL IsCallRepAsm(DWORD nAddress);

	/*!
	*  函 数 名： GetNextAddress
	*  日    期： 2020/06/09
	*  返回类型： DWORD
	*  参    数： DWORD nReadAddress 当前地址
	*  功    能： 获取下一条指令地址
	*/
	DWORD GetNextAddress(DWORD nReadAddress);

	/*!
	*  函 数 名： SetTempBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  功    能： 设置一次性软件断点，单步步过
	*/
	VOID SetTempBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： SetTempHardBreakPoint
	*  日    期： 2020/06/10
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  功    能： 设置一次性硬件断点，单步步过
	*/
	VOID SetTempHardBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： SetHardBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： BOOL
	*  参    数： DWORD nBreakAddress 断点地址
	*  参    数： BOOL nIsCondotion 条件断点标志，TRUE为条件断点，FALSE不是条件断点，为FALSE是参数nCondition必须为{}
	*  参    数： CONDITION nCondition 条件断点的条件信息
	*  功    能： 设置硬件执行断点或硬件条件执行断点
	*/
	BOOL SetHardBreakPoint(DWORD nBreakAddress, BOOL nIsCondotion, CONDITION nCondition);

	/*!
	*  函 数 名： SetHardBreakPointEx
	*  日    期： 2020/06/11
	*  返回类型： BOOL
	*  参    数： DWORD nBreakAddress 断点地址
	*  参    数： DWORD nType 断点类型
	*  参    数： DWORD nSize 断点长度
	*  功    能： 设置硬件读写断点
	*/
	BOOL SetHardBreakPointEx(DWORD nBreakAddress, DWORD nType, DWORD nSize);

	/*!
	*  函 数 名： DelHardBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  功    能： 删除指定断点(硬件)
	*/
	VOID DelHardBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： HaveMemoryBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： BOOL	FALSE为不存在，TRUE为存在
	*  功    能： 判断是否已经存在内存断点
	*/
	BOOL HaveMemoryBreakPoint();

	/*!
	*  函 数 名： SetMemoryBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  参    数： DWORD nType 断点类型，8为执行断点，0为读取断点,1为写入断点
	*  参    数： BOOL nIsCondotion 条件断点标志，TRUE为条件断点，FALSE不是条件断点，为FALSE是参数nCondition必须为{}
	*  参    数： CONDITION nCondotion 条件断点的条件信息
	*  功    能： 设置内存断点
	*/
	VOID SetMemoryBreakPoint(DWORD nBreakAddress, DWORD nType, BOOL nIsCondotion, CONDITION nCondotion);

	/*!
	*  函 数 名： DelMemoryBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 
	*  功    能： 删除内存断点
	*/
	VOID DelMemoryBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： GetOnceAsm
	*  日    期： 2020/06/09
	*  返回类型： CString
	*  参    数： DWORD nReadAddress 地址
	*  功    能： 获得一条反汇编指令
	*/
	CString GetOnceAsm(DWORD nReadAddress);

	/*!
	*  函 数 名： AddBreakReadWrite
	*  日    期： 2020/06/13
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 断点地址
	*  参    数： CString nAsm 反汇编
	*  参    数： DWORD nFlags 硬件读写与内存读写标志，为1是硬件，为2是内存
	*  功    能： 将读写断点信息写入到数组中
	*/
	VOID AddBreakReadWrite(DWORD nBreakAddress, CString nAsm, DWORD nFlags);

	/*!
	*  函 数 名： CloseAllHardBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  功    能： 关闭硬件断点
	*/
	VOID CloseAllHardBreakPoint();

	/*!
	*  函 数 名： CloseOneHardBreakPoint
	*  日    期： 2020/06/10
	*  返回类型： BOOL	如果运行处是硬件断点地址返回TRUE,否则返回FALSE
	*  参    数： DWORD nAddress 断点地址
	*  功    能： 关闭指定硬件断点
	*/
	BOOL CloseOneHardBreakPoint(DWORD nAddress);

	/*!
	*  函 数 名： CloseOneMemBreakPoint
	*  日    期： 2020/06/13
	*  返回类型： BOOL
	*  参    数： DWORD nAddress 触发内存读写指令地址处
	*  功    能： 关闭指定内存读写断点
	*/
	BOOL CloseOneMemBreakPoint(DWORD nAddress);

	/*!
	*  函 数 名： DelTempHardBreakPoint
	*  日    期： 2020/06/09
	*  返回类型： VOID
	*  参    数： DWORD nBreakAddress 
	*  功    能： 将断点信息从数组中删除
	*/
	VOID DelTempHardBreakPoint(DWORD nBreakAddress);

	/*!
	*  函 数 名： GetMemoryBreakPointType
	*  日    期： 2020/06/10
	*  返回类型： DWORD	如果没有设置内存断点返回-1，否则返回断点类型，8为执行，0为读取，1为写入
	*  功    能： 获取设置的内存断点类型
	*/
	DWORD GetMemoryBreakPointType();

	/*!
	*  函 数 名： SetHardReadWriteProc
	*  日    期： 2020/06/11
	*  返回类型： VOID
	*  参    数： DWORD nType 断点类型
	*  参    数： DWORD nSize 大小
	*  功    能： 设置硬件读写断点
	*/
	VOID SetHardReadWriteProc(DWORD nType, DWORD nSize);

	/*!
	*  函 数 名： SetMemoryReadWriteProc
	*  日    期： 2020/06/12
	*  返回类型： VOID
	*  参    数： DWORD nType 断点类型,0为读取断点,1为写入断点,100为读写断点
	*  功    能： 设置内存读写断点
	*/
	VOID SetMemoryReadWriteProc(DWORD nType);

	/*!
	*  函 数 名： OnChangeasm
	*  日    期： 2020/06/13
	*  返回类型： void
	*  功    能： 修改汇编指令
	*/
	void OnChangeasm();

	/*!
	*  函 数 名： OnChangeReg
	*  日    期： 2020/06/14
	*  返回类型： VOID
	*  功    能： 修改寄存器值
	*/
	VOID OnChangeReg();

	/*!
	*  函 数 名： EditMemoryProc
	*  日    期： 2020/06/14
	*  返回类型： VOID
	*  参    数： DWORD nItem 要修改的数据所在列表中的行
	*  参    数： DWORD nSubItem 要修改的数据所在列表中的列
	*  功    能： 修改内存数据
	*/
	VOID EditMemoryProc(DWORD nItem, DWORD nSubItem);

	/*!
	*  函 数 名： IsJumpWord
	*  日    期： 2020/06/17
	*  返回类型： BOOL	如果是返回TRUE,并且将要跳转到的地址赋给 pAddress ,如果不是返回FALSE
	*  参    数： CString nStr 汇编指令
	*  参    数： CString nAddress 汇编指令所在的地址
	*  参    数： LPDWORD pAddress 用于接收要跳转到的地址
	*  功    能： 检测是否为跳转指令(检测指令前四位是否为call或前一位是否为j)
	*/
	BOOL IsJumpWord(CString nStr, CString nAddress ,LPDWORD pAddress);

	/*!
	*  函 数 名： OnInaddress
	*  日    期： 2020/06/15
	*  返回类型： void
	*  功    能： 如果是带地址的跳转指令，就跳转到地址处
	*/
	void OnInaddress();

	/*!
	*  函 数 名： FunDbg
	*  日    期： 2020/06/16
	*  返回类型： void
	*  功    能： 反反调试
	*/
	void FunDbg();

	/*!
	*  函 数 名： GetPebAddress
	*  日    期： 2020/06/16
	*  返回类型： DWORD
	*  参    数： HANDLE nHandle 目标进程句柄
	*  功    能： 获取指定进程的PEB地址
	*/
	DWORD GetPebAddress(HANDLE nHandle);

	/*!
	*  函 数 名： GetApiAddress
	*  日    期： 2020/06/16
	*  返回类型： DWORD
	*  参    数： CString nApi Api名字
	*  功    能： 取出指定Api地址
	*/
	DWORD GetApiAddress(CString nApi);

	afx_msg void OnIn();
	afx_msg void OnJump();
	afx_msg void OnRun();
	afx_msg void OnSoftbreak();
	afx_msg void OnConditionsoftbreak();
	afx_msg void OnHardbreak();
	afx_msg void OnConditionhardbreak();
	afx_msg void OnMemoryBreak();
	afx_msg void OnHardwrite1();
	afx_msg void OnHardwrite2();
	afx_msg void OnHardwrite4();
	afx_msg void OnHardread1();
	afx_msg void OnHardread2();
	afx_msg void OnHardread4();
	afx_msg void OnMemoryWrite();
	afx_msg void OnMemoryRead();
	afx_msg void OnGotomemory();
	afx_msg void OnNMRClickListMemory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawListMemory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListAsm(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListAsm(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListReg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListMemory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListStack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListReg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnJumpToMemShow();
	afx_msg void OnJumpToStackShow();
	afx_msg void OnJumpToAsmShow();
	afx_msg void OnGotoAsm();
	afx_msg void OnShowModule();
	afx_msg void OnShowImportExportedTable();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnAntiNtQueryInformationProcess();
	afx_msg void OnAntiPeb();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};
