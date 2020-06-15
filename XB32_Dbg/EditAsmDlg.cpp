// EditAsmDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "EditAsmDlg.h"
#include "afxdialogex.h"

extern DWORD g_SendAddress_Asm;
extern BYTE g_RecvBytes_Asm[256];
extern DWORD g_RecvBytesNum_Asm;
extern CString g_SendAsm_Asm;
extern DWORD g_SendBytesNum_Asm;
extern BOOL g_Ok_Asm;


// EditAsmDlg 对话框

IMPLEMENT_DYNAMIC(EditAsmDlg, CDialogEx)

EditAsmDlg::EditAsmDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDITASM_DIALOG, pParent)
{

}

EditAsmDlg::~EditAsmDlg()
{
}

void EditAsmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit_Asm);
}


BEGIN_MESSAGE_MAP(EditAsmDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &EditAsmDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// EditAsmDlg 消息处理程序


BOOL EditAsmDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	g_RecvBytesNum_Asm = 0;
	ZeroMemory(g_RecvBytes_Asm, _countof(g_RecvBytes_Asm));

	m_Edit_Asm.SetWindowText(g_SendAsm_Asm);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//点击确定按钮 
void EditAsmDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

	CHAR nAsm[250]{};
	CString nTemp1;
	CStringA nTemp2;
	//获取编辑框的内容
	m_Edit_Asm.GetWindowText(nTemp1);
	nTemp2 = nTemp1;

	strcpy_s(nAsm, 250, nTemp2);

	//将汇编指令转换为Opcode
	if (!AsmToOpcode(g_SendAddress_Asm, nAsm, g_RecvBytes_Asm, g_RecvBytesNum_Asm))
	{
		MessageBox(TEXT("指令错误！"), TEXT("提示"), MB_ICONERROR);
		return;
	}

	//如果修改后还有多余的用0x90填充
	if (g_RecvBytesNum_Asm < g_SendBytesNum_Asm)
	{
		//DWORD nPathNum = g_SendBytesNum - g_RecvBytesNum;
		for (DWORD i = g_RecvBytesNum_Asm; i < g_SendBytesNum_Asm; i++)
		{
			g_RecvBytes_Asm[i] = 144;
		}
		g_RecvBytesNum_Asm = g_SendBytesNum_Asm;
	}

	g_SendAsm_Asm = nTemp1;

	//标志按下了确定按钮
	g_Ok_Asm = TRUE;
	EndDialog(0);
}


BOOL EditAsmDlg::AsmToOpcode(DWORD nAddress, CHAR *nAsm, LPBYTE nBytes, DWORD &nByteNum)
{
	XEDPARSE nXedparse = { 0 };

	nXedparse.cip = (ULONGLONG)nAddress;
	strcpy_s(nXedparse.instr, XEDPARSE_MAXBUFSIZE, nAsm);

	if (XEDParseAssemble(&nXedparse) != XEDPARSE_OK)return FALSE;

	nByteNum = nXedparse.dest_size;

	memcpy_s(nBytes, nXedparse.dest_size, nXedparse.dest, nXedparse.dest_size);

	return TRUE;
}

//截获回车键消息，使按下回车键与点击确定按钮功能一样
BOOL EditAsmDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	//如果按下回车键
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (pMsg->hwnd == m_Edit_Asm.m_hWnd)OnBnClickedOk();
		return FALSE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
