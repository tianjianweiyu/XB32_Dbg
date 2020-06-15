#pragma once


// GoToAsmDlg 对话框

class GoToAsmDlg : public CDialogEx
{
	DECLARE_DYNAMIC(GoToAsmDlg)

public:
	GoToAsmDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~GoToAsmDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GOTOASM_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_Edit_Address;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOk();
};
