#include "pch.h"
#include <tchar.h>

//�����
#define PLUGINNAME "XBhelloworld"

//��ʼ������ӿڣ���ѡ������ֵΪ�������
CHAR* WINAPI InitPlugin()
{
	CHAR * nRet = PLUGINNAME;
	MessageBox(NULL, _T("����Ѽ���"), _T("Hello,XiaoBai"), NULL);
	return nRet;
}

//�����Գ��򴥷����̴����¼�������(�����������������᷵�ؽ���id���߳�id)
VOID WINAPI CreateProcessEvent(DWORD m_Pid, DWORD m_Tid)
{

}

//�����̴̳߳����¼�������
VOID WINAPI CreateThreadEvent(HANDLE nHandle, DWORD nThreadLocalBase)
{

}