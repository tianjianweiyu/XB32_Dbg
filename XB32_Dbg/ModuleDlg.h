#pragma once
#include <vector>
#include <TlHelp32.h>

// ModuleDlg 对话框

class ModuleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ModuleDlg)

public:
	ModuleDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ModuleDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MODULE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_List_Module;
	//获取模块列表
	std::vector<MODULEENTRY32> m_modList;

	/*!
	*  函 数 名： EnumModuleList
	*  日    期： 2020/06/14
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
	afx_msg void OnNMDblclkListModule(NMHDR *pNMHDR, LRESULT *pResult);
};
