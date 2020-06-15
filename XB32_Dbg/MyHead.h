#pragma once
#include <minwindef.h>

#include <vector>

using std::vector;

//条件断点信息 使用哪个寄存器 ，寄存器的值
typedef struct _CONDITION
{
	BOOL OPEN_EAX;
	DWORD EAX;

	BOOL OPEN_EBX;
	DWORD EBX;

	BOOL OPEN_ECX;
	DWORD ECX;

	BOOL OPEN_EDX;
	DWORD EDX;

	BOOL OPEN_ESI;
	DWORD ESI;

	BOOL OPEN_EDI;
	DWORD EDI;

	BOOL OPEN_EBP;
	DWORD EBP;

	BOOL OPEN_ESP;
	DWORD ESP;

}CONDITION, *PCONDITION;

//一个模块的导入导出函数表记录结构
typedef struct _TABLE
{
	CString ModuleName;							//模块的名字
	DWORD ModuleAddress;						//模块的加载地址
	DWORD ModuleSize;							//模块的尺寸

	CString ExportModuleName;					//导出表模块的名字
	vector<CString>ExportFunctionAddress;		//导出函数地址(十六进制),只有通过函数名导出的地址
	vector<CString>ExportFunctionName;			//导出的函数名字

	vector<CString>ImportModuleName;			//导入的模块名字
	vector<CString>ImportFunctionName;			//导入的函数名字
	vector<CString>ImportFunctionAddress;		//导入的函数地址(十六进制)
	vector<CString>ThunkAdress;					//FirstThunk指向的结构体数组中每个结构体的内存地址(十六进制)
}TABLE, *PTABLE;