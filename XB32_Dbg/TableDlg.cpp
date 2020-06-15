// TableDlg.cpp: 实现文件
//

#include "pch.h"
#include "XB32_Dbg.h"
#include "TableDlg.h"
#include "afxdialogex.h"
#include "XB32_DbgDlg.h"
#include "Resource.h"

TableDlg* TableDlg::m_pTableDlg = nullptr;

extern CXB32DbgDlg* g_pDlg;
extern DWORD nReadAddressFirst;

// TableDlg 对话框

IMPLEMENT_DYNAMIC(TableDlg, CDialogEx)

TableDlg::TableDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TABLE_DIALOG, pParent)
{
	Create(IDD_TABLE_DIALOG);
}

TableDlg::~TableDlg()
{
}

void TableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List_Module);
	DDX_Control(pDX, IDC_LIST2, m_List_Table);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress_Table);
	DDX_Control(pDX, IDC_STATIC_TABLE, m_Static_Table);
}


BEGIN_MESSAGE_MAP(TableDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &TableDlg::OnNMClickListModule)
END_MESSAGE_MAP()


// TableDlg 消息处理程序
//线程回调函数
DWORD WINAPI RunThread(LPVOID lpThreadParameter)
{
	TableDlg *nDlg = (TableDlg*)lpThreadParameter;

	//枚举被调试进程的模块
	nDlg->EnumModuleList(g_pDlg->m_Pid);
	//显示模块信息到列表
	nDlg->ShowModuleList();

	return 0;
}

BOOL TableDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CRect rect;
	m_List_Module.GetClientRect(rect);

	m_List_Module.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List_Module.InsertColumn(0, TEXT("模块名称"), 0, rect.right / 2);
	m_List_Module.InsertColumn(1, TEXT("模块地址"), 0, rect.right / 2);

	m_List_Table.GetClientRect(rect);

	m_List_Table.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List_Table.InsertColumn(0, TEXT("所属模块"), 0, rect.right / 7);
	m_List_Table.InsertColumn(1, TEXT("函数名字"), 0, rect.right * 3 / 7);
	m_List_Table.InsertColumn(2, TEXT("函数地址"), 0, rect.right / 7);
	m_List_Table.InsertColumn(3, TEXT("Thunk地址"), 0, rect.right  / 7);
	m_List_Table.InsertColumn(4, TEXT("类型"), 0, rect.right / 7);

	//刚开始调试目标进程时
	if (g_pDlg->m_Table.size() == 0)
	{
		CreateThread(NULL, NULL, RunThread, (LPVOID)this, NULL, NULL);
	}
	//点
	else
	{
		ShowModuleList();
		for (DWORD i = 0; i < g_pDlg->m_Table.size(); i++)
		{

			//CString str;
			//m_List_Module.InsertItem(i, g_pDlg->m_Table[i].ModuleName);
			//str.Format(L"%08X", g_pDlg->m_Table[i].ModuleAddress);
			//m_List_Module.SetItemText(i, 1, str);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL TableDlg::EnumModuleList(DWORD nPid)
{
	//1.先给当前进程的模块拍一个快照
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, nPid);
	//判断模块快照是否创建成功
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	//2.从快照中提取出模块信息
	MODULEENTRY32 stcmod = { sizeof(MODULEENTRY32) };
	Module32First(hSnapShot, &stcmod);
	do
	{
		//将模块的信息结构体保存在模块信息结构体数组的尾部
		m_modList.push_back(stcmod);

	} while (Module32Next(hSnapShot, &stcmod));

	//关闭模块快照
	CloseHandle(hSnapShot);

	return true;
}

VOID TableDlg::ShowModuleList()
{
	m_Progress_Table.SetStep(m_modList.size());

	for (auto&i : m_modList)
	{
		//获取列表行数
		DWORD index = m_List_Module.GetItemCount();
		//插入到列表底部
		CString buffer;
		m_List_Module.InsertItem(index, _T(""));
		//模块名
		m_List_Module.SetItemText(index, 0, i.szModule);
		//模块地址
		buffer.Format(_T("%p"), i.modBaseAddr);
		m_List_Module.SetItemText(index, 1, buffer);

		m_Static_Table.SetWindowText(TEXT("正在分析符号:") + CString(i.szModule));

		m_Static_Table.GetWindowText(buffer);

		GetTable(i.szModule, (DWORD)i.modBaseAddr, i.modBaseSize);

		m_Progress_Table.StepIt();

	}
	m_Progress_Table.ShowWindow(FALSE);
	m_Static_Table.ShowWindow(FALSE);
	g_pDlg->InitAsm(nReadAddressFirst);
}

VOID TableDlg::GetTable(CString nModuleName, DWORD nImageBass, DWORD nSize)
{

	vector<CString>ImportModuleName;
	vector<CString>ImportFunctionName;
	vector<CString>ImportFunctionAddress;
	vector<CString>ThunkAddress;

	CString ExportModuleName;
	vector<CString>ExportFunctionAddress;
	vector<CString>ExportFunctionName;

	//读取被调试进程的前4096字节的数据
	BYTE nData[4096]{};
	g_pDlg->ReadMemoryBytes(nImageBass, nData, _countof(nData));

	//获取模块的DOS头
	PIMAGE_DOS_HEADER nDosHeade = (PIMAGE_DOS_HEADER)nData;
	//获取模块的NT头
	PIMAGE_NT_HEADERS32 nNtHead = (PIMAGE_NT_HEADERS32)((DWORD)nData + nDosHeade->e_lfanew);
	//获取模块的文件头
	PIMAGE_FILE_HEADER nFileHead = (PIMAGE_FILE_HEADER)&nNtHead->FileHeader;
	//获取模块的扩展头
	PIMAGE_OPTIONAL_HEADER32 nOptionalHead = (PIMAGE_OPTIONAL_HEADER32)&nNtHead->OptionalHeader;

	//获取导入表的相对虚拟地址
	DWORD nImportHeadRva = nOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	//获取导入表的内存虚拟地址
	DWORD nImportAddress = nImportHeadRva + nImageBass;
	//获取导入表的大小
	DWORD nImportSize = nOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

	//获取导出表的相对虚拟地址
	DWORD nExportHeadRva = nOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	//获取导出表的内存虚拟地址
	DWORD nExportAddress = nExportHeadRva + nImageBass;
	//获取导出表的大小
	DWORD nExportSize = nOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	//清空数据
	ZeroMemory(nData, _countof(nData));
	//获取导入表结构数组中的第一个结构体信息
	g_pDlg->ReadMemoryBytes(nImportAddress, nData, _countof(nData));
	PIMAGE_IMPORT_DESCRIPTOR nImportHead = (PIMAGE_IMPORT_DESCRIPTOR)nData;

	//遍历导入表结构体数组
	while (nImportHead->Name != 0 && nImportHead->OriginalFirstThunk)
	{
		DWORD nIndex;

		//获取导入文件的名字的虚拟地址
		DWORD ImportDllNameVa = nImportHead->Name + (DWORD)nImageBass;

		//获取导入文件的名字
		BYTE nDllName[256]{};
		g_pDlg->ReadMemoryBytes(ImportDllNameVa, nDllName, _countof(nDllName));

		//获取导入名称表(INT)的虚拟地址
		DWORD nIntAddress = nImportHead->OriginalFirstThunk + nImageBass;
		//获取导入地址表(IAT)的虚拟地址
		DWORD nIatAddress = nImportHead->FirstThunk + nImageBass;

		//获取导入名称表与导入地址表指向的 IMAGE_THUNK_DATA32 结构数据
		IMAGE_THUNK_DATA32 nThunkDataName{};
		IMAGE_THUNK_DATA32 nThunkDataAddress{};
		g_pDlg->ReadMemoryBytes(nIntAddress, (LPBYTE)&nThunkDataName, sizeof(IMAGE_THUNK_DATA32));
		g_pDlg->ReadMemoryBytes(nIatAddress, (LPBYTE)&nThunkDataAddress, sizeof(IMAGE_THUNK_DATA32));

		//遍历模块导入的所有函数
		while (nThunkDataName.u1.AddressOfData != 0)
		{
			nIndex = m_List_Table.GetItemCount();
			//将导入函数所属模块名添加到数组中
			ImportModuleName.push_back(CString(nDllName));

			//函数名导入
			if (!IMAGE_SNAP_BY_ORDINAL(nThunkDataName.u1.AddressOfData))
			{
				//获取结构 IMAGE_IMPORT_BY_NAME 的虚拟地址
				DWORD nImportNameAddress = nThunkDataName.u1.Function + nImageBass;
				//申请一块内存并清空，用于存放读取到的函数名
				LPBYTE nImportName = (LPBYTE)malloc(sizeof(IMAGE_IMPORT_BY_NAME) + 256);
				ZeroMemory(nImportName, sizeof(IMAGE_IMPORT_BY_NAME) + 256);
				g_pDlg->ReadMemoryBytes(nImportNameAddress, nImportName, sizeof(IMAGE_IMPORT_BY_NAME) + 256);

				//将导入函数名添加到数组中
				CString nImportFunName(nImportName + 2);
				ImportFunctionName.push_back(nImportFunName);

				//释放内存
				free(nImportName);
			}
			//序号导入,低16位就是导入序号
			else
			{
				//获取导入函数的序号并将序号添加到数组中(和导入函数名同一个数组)
				CString nImportOrdinal;
				nImportOrdinal.Format(TEXT("0x%04X"), nThunkDataName.u1.Ordinal & 0xffff);
				ImportFunctionName.push_back(TEXT("序号：") + nImportOrdinal);
			}

			//将导入函数的地址添加到数组中
			CString nTemp;
			nTemp.Format(TEXT("%08X"), nThunkDataAddress.u1.Function);
			ImportFunctionAddress.push_back(nTemp);

			m_List_Table.SetItemText(nIndex, 4, TEXT("导入"));

			//INT与IAT都指向 IMAGE_THUNK_DATA32 结构体数组，该数组以全零结尾
			//将每个 IMAGE_THUNK_DATA32 结构的地址保存到数组中
			nTemp.Format(TEXT("%08X"), nIatAddress);
			ThunkAddress.push_back(nTemp);

			//读取下一个 IMAGE_THUNK_DATA32 结构数据
			nIntAddress = nIntAddress + sizeof(IMAGE_THUNK_DATA32);
			nIatAddress = nIatAddress + sizeof(IMAGE_THUNK_DATA32);
			g_pDlg->ReadMemoryBytes(nIntAddress, (LPBYTE)&nThunkDataName, sizeof(IMAGE_THUNK_DATA32));
			g_pDlg->ReadMemoryBytes(nIatAddress, (LPBYTE)&nThunkDataAddress, sizeof(IMAGE_THUNK_DATA32));

		}

		//获取导入表结构体数组中下一个结构体
		nImportHead++;
	}
	
	//清空数据
	ZeroMemory(nData, _countof(nData));
	//获取导出表结构的数据
	g_pDlg->ReadMemoryBytes(nExportAddress, nData, _countof(nData));
	PIMAGE_EXPORT_DIRECTORY nExportHead = (PIMAGE_EXPORT_DIRECTORY)nData;

	//如果没有导出表
	if (nExportAddress == nImageBass)
	{
		g_pDlg->m_Table.push_back({ nModuleName,nImageBass,nSize, ExportModuleName ,ExportFunctionAddress ,ExportFunctionName,
			ImportModuleName,ImportFunctionName,ImportFunctionAddress,ThunkAddress });
		return;
	}

	//获取函数地址表的内存虚拟地址
	PDWORD nExportFunctionAddress = (PDWORD)(nExportHead->AddressOfFunctions + nImageBass);
	//获取函数名称表的内存虚拟地址
	PDWORD nExportFunctionNameAddress = (PDWORD)(nExportHead->AddressOfNames + nImageBass);
	//获取序号表的内存虚拟地址
	PWORD nExportFunctionOrdinalsAddress = (PWORD)(nExportHead->AddressOfNameOrdinals + nImageBass);
	//获取导出模块名的内存虚拟地址
	PDWORD nExportModuleNameAddress = (PDWORD)(nExportHead->Name + nImageBass);

	//获取导出模块的名称
	BYTE nExportModuleName[256]{};
	g_pDlg->ReadMemoryBytes((DWORD)nExportModuleNameAddress, nExportModuleName, _countof(nExportModuleName));

	//保存导出模块名
	ExportModuleName = nExportModuleName;

	//申请一块内存用来存放导出的函数地址
	DWORD * ExportAddress = new DWORD[nExportHead->AddressOfFunctions]{};

	for (DWORD i = 0; i < nExportHead->AddressOfFunctions; i++)
	{
		//获取导出函数的地址，函数地址表中存储的是函数地址的相对虚拟地址，
		//需要加上加载基址得到真正的函数地址
		DWORD nAddress;
		g_pDlg->ReadMemoryBytes((DWORD)nExportFunctionAddress, (LPBYTE)&nAddress, sizeof(DWORD));
		nAddress = nAddress + nImageBass;

		//将获得的导出函数的地址放入刚申请的内存中
		ExportAddress[i] = nAddress;

		//获取下一个函数地址
		nExportFunctionAddress++;
	}

	for (DWORD i = 0; i < nExportHead->NumberOfNames; i++)
	{
		BYTE nExportFunctionName[256]{};
		DWORD nAddress;
		WORD nOrdinals;
		//获取导出函数的名称的相对虚拟地址
		g_pDlg->ReadMemoryBytes((DWORD)nExportFunctionNameAddress, (LPBYTE)&nAddress, sizeof(DWORD));
		//获取导出函数的名称的内存虚拟地址
		nAddress = nAddress + nImageBass;
		//获取导出函数的名称
		g_pDlg->ReadMemoryBytes(nAddress, nExportFunctionName, _countof(nExportFunctionName));
		//获取导出函数地址在函数地址表的位置
		g_pDlg->ReadMemoryBytes((DWORD)nExportFunctionOrdinalsAddress, (LPBYTE)&nOrdinals, sizeof(WORD));

		CString nTemp;
		nTemp.Format(TEXT("%08X"), ExportAddress[nOrdinals]);
		ExportFunctionAddress.push_back(nTemp);
		ExportFunctionName.push_back(CString(nExportFunctionName));

		//获取下一个函数名称与序号
		nExportFunctionNameAddress++;
		nExportFunctionOrdinalsAddress++;
	}

	//释放内存
	delete[] ExportAddress;

	g_pDlg->m_Table.push_back({ nModuleName,nImageBass,nSize, ExportModuleName ,ExportFunctionAddress ,ExportFunctionName,
	ImportModuleName,ImportFunctionName,ImportFunctionAddress,ThunkAddress });

	return;
}


//模块列表单击左键
void TableDlg::OnNMClickListModule(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取选中的行数
	DWORD nIndex = pNMItemActivate->iItem;

	//如果无效直接返回
	if (nIndex == -1)return;

	DWORD nCount;

	//清空导入导出函数列表
	m_List_Table.DeleteAllItems();

	//显示选中模块的所有导出函数信息(以函数名导出的)
	for (DWORD i = 0; i < g_pDlg->m_Table[nIndex].ExportFunctionName.size(); i++)
	{
		nCount = m_List_Table.GetItemCount();
		m_List_Table.InsertItem(nCount, _T(""));
		//导出函数所在模块名
		m_List_Table.SetItemText(nCount, 0, g_pDlg->m_Table[nIndex].ExportModuleName);
		//导出函数名
		m_List_Table.SetItemText(nCount, 1, g_pDlg->m_Table[nIndex].ExportFunctionName[i]);
		//导出函数地址
		m_List_Table.SetItemText(nCount, 2, g_pDlg->m_Table[nIndex].ExportFunctionAddress[i]);
		//此项无效
		m_List_Table.SetItemText(nCount, 3, _T(" - "));
		//函数类型(导出)
		m_List_Table.SetItemText(nCount, 4, _T("导出"));
	}

	//显示选中模块的所有导入函数信息
	for (DWORD i = 0; i < g_pDlg->m_Table[nIndex].ImportModuleName.size(); i++)
	{
		nCount = m_List_Table.GetItemCount();
		m_List_Table.InsertItem(nCount, _T(""));
		//导入函数所在的模块名
		m_List_Table.SetItemText(nCount, 0, g_pDlg->m_Table[nIndex].ImportModuleName[i]);
		//导入函数的名称或序号
		m_List_Table.SetItemText(nCount, 1, g_pDlg->m_Table[nIndex].ImportFunctionName[i]);
		//导入函数的地址
		m_List_Table.SetItemText(nCount, 2, g_pDlg->m_Table[nIndex].ImportFunctionAddress[i]);
		//FirstThunk指向的结构体数组中每个结构体的内存地址(十六进制)
		m_List_Table.SetItemText(nCount, 3, g_pDlg->m_Table[nIndex].ThunkAdress[i]);
		//函数类型(导入)
		m_List_Table.SetItemText(nCount, 4, TEXT("导入"));
	}
}

TableDlg* TableDlg::GetTableDlg()
{
	if (!m_pTableDlg)
	{
		m_pTableDlg = new TableDlg;
	}
	return m_pTableDlg;
}
