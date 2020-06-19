#pragma once
#include "vector"
using std::vector;

// 1. 插件的存在形式
//	- 插件通常是以 dll 的形式存在的，但是一些
//	- 应用程序，为了区分自己的插件，通常会修改
//	- 它的后缀名为其他更好辨别的形式  .plg

// 2. 插件的查找方式
//	- 通常一个应用程序都会有默认的插件存放目录，
//	- 所有的插件都会被保存到这个目录中，应用程序
//	- 通过遍历文件，判断后缀，确认内容的方式来确
//	- 保找到的是一个有效的插件文件


class LoadPlugin
{
public:
	LoadPlugin();
	~LoadPlugin();

	//插件句柄
	vector <DWORD>m_Handle;
	//插件初始化函数地址
	vector <DWORD>m_InitPlugin_Address;
	//插件名
	vector <CString>m_PluginName;
	//接口CreateProcessEvent的函数地址
	vector <DWORD>m_CreateProcessEvent_Address;
	//接口CreateThreadEvent的函数地址
	vector <DWORD>m_CreateThreadEvent_Address;

	//定义初始化插件的函数指针
	typedef CHAR*(WINAPI *pInitPlugin)(VOID);
	pInitPlugin InitPlugin;

	typedef VOID(WINAPI *pCreateProcessEvent)(DWORD, DWORD);
	pCreateProcessEvent CreateProcessEvent;

	typedef VOID(WINAPI *pCreateThreadEvent)(DWORD, DWORD);
	pCreateThreadEvent CreateThreadEvent;

	/*!
	*  函 数 名： LoadPluginFile
	*  日    期： 2020/06/17
	*  返回类型： VOID
	*  功    能： 载入插件
	*/
	VOID LoadPluginFile();

	/*!
	*  函 数 名： GetProgramDir
	*  日    期： 2020/06/17
	*  返回类型： CString
	*  功    能： 获取程序所在目录
	*/
	CString GetProgramDir();

	/*!
	*  函 数 名： CallCreateProcessEvent
	*  日    期： 2020/06/17
	*  返回类型： VOID
	*  参    数： DWORD nPid 被调试进程ID
	*  参    数： DWORD nTid 被调试线程ID
	*  功    能： 调试全部插件的CreateProcessEvent函数,触发进程创建事件被调用
	*/
	VOID CallCreateProcessEvent(DWORD nPid, DWORD nTid);

	/*!
	*  函 数 名： CallCreateThreadEvent
	*  日    期： 2020/06/17
	*  返回类型： VOID
	*  参    数： DWORD nHandle 导致调试事件的线程的句柄  _CREATE_THREAD_DEBUG_INFO结构体中 hThread 字段值
	*  参    数： DWORD nThreadLocalBase 指向数据块的指针 _CREATE_THREAD_DEBUG_INFO结构体中 lpThreadLocalBase 字段值;
	*  功    能： 调试全部插件的CreateThreadEvent函数，触发线程创建事件时被调用 
	*/
	VOID CallCreateThreadEvent(DWORD nHandle, DWORD nThreadLocalBase);

};

