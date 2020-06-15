#pragma once


// EditMemoryDlg 对话框

class EditMemoryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(EditMemoryDlg)

public:
	EditMemoryDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~EditMemoryDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDITMEM_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_Edit_Address;
	CEdit m_Edit_Data;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
