#include "pch.h"
#include <tchar.h>

//插件名
#define PLUGINNAME "XBhelloworld"

//初始化插件接口，必选！返回值为插件名字
CHAR* WINAPI InitPlugin()
{
	CHAR * nRet = PLUGINNAME;
	MessageBox(NULL, _T("插件已加载"), _T("Hello,XiaoBai"), NULL);
	return nRet;
}

//被调试程序触发进程创建事件被调用(两个参数，调试器会返回进程id和线程id)
VOID WINAPI CreateProcessEvent(DWORD m_Pid, DWORD m_Tid)
{

}

//触发线程程创建事件被调用
VOID WINAPI CreateThreadEvent(HANDLE nHandle, DWORD nThreadLocalBase)
{

}