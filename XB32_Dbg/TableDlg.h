#pragma once
#include <TlHelp32.h>
#include <vector>


// TableDlg 对话框

class TableDlg : public CDialogEx
{
	DECLARE_DYNAMIC(TableDlg)

public:
	TableDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~TableDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TABLE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_List_Module;
	CListCtrl m_List_Table;

	CProgressCtrl m_Progress_Table;
	CStatic m_Static_Table;

	//获取模块列表
	std::vector<MODULEENTRY32> m_modList;

	virtual BOOL OnInitDialog();

	/*!
	*  函 数 名： EnumModuleList
	*  日    期： 2020/06/15
	*  返回类型： BOOL
	*  参    数： DWORD nPid 目标进程ID
	*  功    能： 遍历指定进程的模块
	*/
	BOOL EnumModuleList(DWORD nPid);

	/*!
	*  函 数 名： ShowModuleList
	*  日    期： 2020/06/15
	*  返回类型： VOID
	*  功    能： 显示模块信息到列表
	*/
	VOID ShowModuleList();

	/*!
	*  函 数 名： GetTable
	*  日    期： 2020/06/15
	*  返回类型： VOID
	*  参    数： CString nModuleName 模块名
	*  参    数： DWORD nImageBass 模块加载基址
	*  参    数： DWORD nSize 模块大小
	*  功    能： 获取指定模块导入导出函数信息
	*/
	VOID GetTable(CString nModuleName, DWORD nImageBass, DWORD nSize);


	afx_msg void OnNMClickListModule(NMHDR *pNMHDR, LRESULT *pResult);

	//获取单例，只允许当前创建一个对象
	static TableDlg* GetTableDlg();

private:
	//只能实例化一个类
	static TableDlg* m_pTableDlg;
};
