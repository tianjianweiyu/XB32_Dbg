#pragma once


// GotoMemDlg 对话框

class GotoMemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(GotoMemDlg)

public:
	GotoMemDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~GotoMemDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GOTOMEMORY_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_Edit_Address;
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
