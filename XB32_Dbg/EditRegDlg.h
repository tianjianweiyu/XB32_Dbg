#pragma once


// EditRegDlg 对话框

class EditRegDlg : public CDialogEx
{
	DECLARE_DYNAMIC(EditRegDlg)

public:
	EditRegDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~EditRegDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDITREG_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_Edit_Reg;
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
