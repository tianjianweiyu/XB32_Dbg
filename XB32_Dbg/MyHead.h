#pragma once
#include <minwindef.h>

#include <vector>

using std::vector;

//�����ϵ���Ϣ ʹ���ĸ��Ĵ��� ���Ĵ�����ֵ
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

//һ��ģ��ĵ��뵼���������¼�ṹ
typedef struct _TABLE
{
	CString ModuleName;							//ģ�������
	DWORD ModuleAddress;						//ģ��ļ��ص�ַ
	DWORD ModuleSize;							//ģ��ĳߴ�

	CString ExportModuleName;					//������ģ�������
	vector<CString>ExportFunctionAddress;		//����������ַ(ʮ������),ֻ��ͨ�������������ĵ�ַ
	vector<CString>ExportFunctionName;			//�����ĺ�������

	vector<CString>ImportModuleName;			//�����ģ������
	vector<CString>ImportFunctionName;			//����ĺ�������
	vector<CString>ImportFunctionAddress;		//����ĺ�����ַ(ʮ������)
	vector<CString>ThunkAdress;					//FirstThunkָ��Ľṹ��������ÿ���ṹ����ڴ��ַ(ʮ������)
}TABLE, *PTABLE;