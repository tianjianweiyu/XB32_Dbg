#pragma once
#include <minwindef.h>

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