#pragma once


// ConditionBreakDlg 对话框

class ConditionBreakDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ConditionBreakDlg)

public:
	ConditionBreakDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ConditionBreakDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONDITIONBREAK_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
