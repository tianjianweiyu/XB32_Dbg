#include "pch.h"
#include "LoadPlugin.h"

LoadPlugin::LoadPlugin()
{
	//��ʼ��������LoadPluginFile����Ŀ¼�µ������ļ�����������
	LoadPluginFile();
}

LoadPlugin::~LoadPlugin()
{
	//��������
	//�˳�֮ǰ������������Ĳ��ж�أ��ͷ���Դ
	for (DWORD i = 0; i < m_Handle.size(); i++)
	{
		FreeLibrary((HMODULE)m_Handle[i]);
	}
	//������б����ڶ�̬��������ĺ���ָ��
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


	//����Ŀ¼���ڳ���Ŀ¼�µ�Plugin�ļ���
	CString PluginPath(GetProgramDir() + _T("Plugin"));

	//������������ļ���Ϣ
	WIN32_FIND_DATA FileInfo = { 0 };

	//���������·�����������ú�׺��
	//c_str()���ַ���������ת��Ϊ�� null ��β�� C ��ʽ�ַ���
	HANDLE FindHandle = FindFirstFile(PluginPath+_T("\\*"), &FileInfo);
	if (FindHandle == INVALID_HANDLE_VALUE)return;

	//һ��һ������
	do {

		//��ȡ��׺��
		LPTSTR pszExtension = PathFindExtension(FileInfo.cFileName);

		//��������һ���ļ��л��ߺ�׺����Ϊ.xbplugin��������
		if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY || lstrcmp(pszExtension, L".xbplugin") != 0)
		{
			continue;
		}
		//������ǲ���ļ�
		else
		{
			//����LoadLibrary�����ļ�
			Handle = LoadLibrary(PluginPath + _T("\\") + FileInfo.cFileName);

			//���ģ����سɹ�����Ҫ����ṩ�Լ�����Ϣ����ʽ���ǵ���һ���ض��ĺ���
			if (Handle)
			{
				//����GetProcAddressȡ��ָ���Ĳ����ʼ��������ַ
				nAddress = (DWORD)GetProcAddress(Handle, "InitPlugin");

				//���������ȡ�ɹ�
				if (nAddress)
				{
					//���ò���ļ��ؾ�����浽��̬������
					m_Handle.push_back((DWORD)Handle);
					//����ʼ��������ַ�����ڶ�̬������
					m_InitPlugin_Address.push_back((DWORD)nAddress);
					//����ʼ��������ַǿ��ת��λ����ָ��
					InitPlugin = (pInitPlugin)nAddress;
					//ͨ������ָ����ó�ʼ�����������ѵ��õķ��ؽ�����浽��̬�����ڣ����ص��ǲ��������
					m_PluginName.push_back(CString(InitPlugin()));

					//ȡ��ָ���ӿ�CreateProcessEvent�ĺ�����ַ
					nAddress = (DWORD)GetProcAddress(Handle, "CreateProcessEvent");
					//���������ַ��Ϊ0������ڴ˺�������������ַ���浽��̬������
					if (nAddress)m_CreateProcessEvent_Address.push_back((DWORD)nAddress);

					//ȡ��ָ���ӿ�CreateThreadEvent�ĺ�����ַ
					nAddress = (DWORD)GetProcAddress(Handle, "CreateThreadEvent");
					//���������ַ��Ϊ0������ڴ˺�������������ַ���浽��̬������
					if (nAddress)m_CreateThreadEvent_Address.push_back((DWORD)nAddress);

				}
				//���������ַ��ȡʧ�ܣ����ͷŸ��ļ�
				else
				{
					FreeLibrary(Handle);
					Handle = nullptr;
				}
			}
		}

		//����������һ���ļ�
	} while (FindNextFile(FindHandle, &FileInfo));
}


CString LoadPlugin::GetProgramDir()
{
	TCHAR exeFullPath[MAX_PATH]; // Full path
	TCHAR *nWeak;

	//��ȡ��ǰ���̵Ŀ�ִ���ļ���·����
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	//��·����ΪĿ¼
	nWeak = _tcsrchr(exeFullPath, '\\');
	nWeak[1] = 0;

	return CString(exeFullPath);

}

VOID LoadPlugin::CallCreateProcessEvent(DWORD nPid, DWORD nTid)
{
	//��������CreateProcessEvent�����ĵ�ַ
	for (DWORD i = 0; i < m_CreateProcessEvent_Address.size(); i++)
	{
		//ǿ��ת��Ϊ����ָ��
		CreateProcessEvent = (pCreateProcessEvent)m_CreateProcessEvent_Address[i];
		//ͨ������ָ����øú���
		CreateProcessEvent(nPid, nTid);
	}
}

VOID LoadPlugin::CallCreateThreadEvent(DWORD nHandle, DWORD nThreadLocalBase)
{
	//��������CreateThreadEvent�����ĵ�ַ
	for (DWORD i = 0; i < m_CreateThreadEvent_Address.size(); i++)
	{
		//ǿ��ת��Ϊ����ָ��
		CreateThreadEvent = (pCreateThreadEvent)m_CreateThreadEvent_Address[i];
		//ͨ������ָ����øú���
		CreateThreadEvent(nHandle, nThreadLocalBase);
	}
}
