#include "pch.h"
#include "LoadPlugin.h"

LoadPlugin::LoadPlugin()
{
	//初始化，调用LoadPluginFile遍历目录下的所有文件，并载入插件
	LoadPluginFile();
}

LoadPlugin::~LoadPlugin()
{
	//析构函数
	//退出之前，将所有载入的插件卸载，释放资源
	for (DWORD i = 0; i < m_Handle.size(); i++)
	{
		FreeLibrary((HMODULE)m_Handle[i]);
	}
	//清除所有保存在动态数组里面的函数指针
	m_PluginName.swap(vector <CString>());
	m_Handle.swap(vector <DWORD>());
	m_InitPlugin_Address.swap(vector <DWORD>());
	m_CreateProcessEvent_Address.swap(vector <DWORD>());
	m_CreateThreadEvent_Address.swap(vector <DWORD>());

}

VOID LoadPlugin::LoadPluginFile()
{

	HMODULE Handle = nullptr;
	DWORD nAddress = 0;


	//设置目录，在程序目录下的Plugin文件夹
	CString PluginPath(GetProgramDir() + _T("Plugin"));

	//保存遍历到的文件信息
	WIN32_FIND_DATA FileInfo = { 0 };

	//遍历插件的路径，可以设置后缀名
	//c_str()将字符串的内容转换为以 null 结尾的 C 样式字符串
	HANDLE FindHandle = FindFirstFile(PluginPath+_T("\\*"), &FileInfo);
	if (FindHandle == INVALID_HANDLE_VALUE)return;

	//一个一个遍历
	do {

		//获取后缀名
		LPTSTR pszExtension = PathFindExtension(FileInfo.cFileName);

		//如果这个是一个文件夹或者后缀名不为.xbplugin，则跳过
		if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY || lstrcmp(pszExtension, L".xbplugin") != 0)
		{
			continue;
		}
		//如果这是插件文件
		else
		{
			//调用LoadLibrary加载文件
			Handle = LoadLibrary(PluginPath + _T("\\") + FileInfo.cFileName);

			//如果模块加载成功，需要插件提供自己的信息，形式就是导出一个特定的函数
			if (Handle)
			{
				//调用GetProcAddress取出指定的插件初始化函数地址
				nAddress = (DWORD)GetProcAddress(Handle, "InitPlugin");

				//如果函数获取成功
				if (nAddress)
				{
					//将该插件的加载句柄保存到动态数组内
					m_Handle.push_back((DWORD)Handle);
					//将初始化函数地址保存在动态数组内
					m_InitPlugin_Address.push_back((DWORD)nAddress);
					//将初始化函数地址强制转换位函数指针
					InitPlugin = (pInitPlugin)nAddress;
					//通过函数指针调用初始化函数，并把调用的返回结果保存到动态数组内，返回的是插件的名字
					m_PluginName.push_back(CString(InitPlugin()));

					//取出指定接口CreateProcessEvent的函数地址
					nAddress = (DWORD)GetProcAddress(Handle, "CreateProcessEvent");
					//如果函数地址不为0，则存在此函数，将函数地址保存到动态数组内
					if (nAddress)m_CreateProcessEvent_Address.push_back((DWORD)nAddress);

					//取出指定接口CreateThreadEvent的函数地址
					nAddress = (DWORD)GetProcAddress(Handle, "CreateThreadEvent");
					//如果函数地址不为0，则存在此函数，将函数地址保存到动态数组内
					if (nAddress)m_CreateThreadEvent_Address.push_back((DWORD)nAddress);

				}
				//如果函数地址获取失败，则释放该文件
				else
				{
					FreeLibrary(Handle);
					Handle = nullptr;
				}
			}
		}

		//继续遍历下一个文件
	} while (FindNextFile(FindHandle, &FileInfo));
}


CString LoadPlugin::GetProgramDir()
{
	TCHAR exeFullPath[MAX_PATH]; // Full path
	TCHAR *nWeak;

	//获取当前进程的可执行文件的路径。
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	//将路径变为目录
	nWeak = _tcsrchr(exeFullPath, '\\');
	nWeak[1] = 0;

	return CString(exeFullPath);

}

VOID LoadPlugin::CallCreateProcessEvent(DWORD nPid, DWORD nTid)
{
	//遍历所有CreateProcessEvent函数的地址
	for (DWORD i = 0; i < m_CreateProcessEvent_Address.size(); i++)
	{
		//强制转换为函数指针
		CreateProcessEvent = (pCreateProcessEvent)m_CreateProcessEvent_Address[i];
		//通过函数指针调用该函数
		CreateProcessEvent(nPid, nTid);
	}
}

VOID LoadPlugin::CallCreateThreadEvent(DWORD nHandle, DWORD nThreadLocalBase)
{
	//遍历所有CreateThreadEvent函数的地址
	for (DWORD i = 0; i < m_CreateThreadEvent_Address.size(); i++)
	{
		//强制转换为函数指针
		CreateThreadEvent = (pCreateThreadEvent)m_CreateThreadEvent_Address[i];
		//通过函数指针调用该函数
		CreateThreadEvent(nHandle, nThreadLocalBase);
	}
}
