#pragma once

#include "afxwin.h"
//汇编引擎头文件，静态库
#include "XEDParse//XEDParse.h"
#pragma comment(lib,"XEDParse/x86/XEDParse_x86.lib")


// EditAsmDlg 对话框

class EditAsmDlg : public CDialogEx
{
	DECLARE_DYNAMIC(EditAsmDlg)

public:
	EditAsmDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~EditAsmDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDITASM_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_Edit_Asm;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();

	/*!
	*  函 数 名： AsmToOpcode
	*  日    期： 2020/06/13
	*  返回类型： BOOL
	*  参    数： DWORD nAddress 要修改指令的首地址
	*  参    数： CHAR * nAsm 修改后的指令(编辑框输入的指令)
	*  参    数： LPBYTE nBytes 转换成的Opcode
	*  参    数： DWORD & nByteNum 转换成的Opcode的大小(字节)
	*  功    能： 将汇编指令转换为Opcode
	*/
	BOOL AsmToOpcode(DWORD nAddress, CHAR *nAsm, LPBYTE nBytes, DWORD &nByteNum);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
