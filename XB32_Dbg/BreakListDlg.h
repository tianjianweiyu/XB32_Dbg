#pragma once


// BreakListDlg 对话框

class BreakListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(BreakListDlg)

public:
	BreakListDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~BreakListDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BREAKLIST_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_List_Break;
	afx_msg void OnNMDblclkListBreaklist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListBreaklist(NMHDR *pNMHDR, LRESULT *pResult);
};
