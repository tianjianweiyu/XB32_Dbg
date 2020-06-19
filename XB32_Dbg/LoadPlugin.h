#pragma once
#include "vector"
using std::vector;

// 1. ����Ĵ�����ʽ
//	- ���ͨ������ dll ����ʽ���ڵģ�����һЩ
//	- Ӧ�ó���Ϊ�������Լ��Ĳ����ͨ�����޸�
//	- ���ĺ�׺��Ϊ�������ñ�����ʽ  .plg

// 2. ����Ĳ��ҷ�ʽ
//	- ͨ��һ��Ӧ�ó��򶼻���Ĭ�ϵĲ�����Ŀ¼��
//	- ���еĲ�����ᱻ���浽���Ŀ¼�У�Ӧ�ó���
//	- ͨ�������ļ����жϺ�׺��ȷ�����ݵķ�ʽ��ȷ
//	- ���ҵ�����һ����Ч�Ĳ���ļ�


class LoadPlugin
{
public:
	LoadPlugin();
	~LoadPlugin();

	//������
	vector <DWORD>m_Handle;
	//�����ʼ��������ַ
	vector <DWORD>m_InitPlugin_Address;
	//�����
	vector <CString>m_PluginName;
	//�ӿ�CreateProcessEvent�ĺ�����ַ
	vector <DWORD>m_CreateProcessEvent_Address;
	//�ӿ�CreateThreadEvent�ĺ�����ַ
	vector <DWORD>m_CreateThreadEvent_Address;

	//�����ʼ������ĺ���ָ��
	typedef CHAR*(WINAPI *pInitPlugin)(VOID);
	pInitPlugin InitPlugin;

	typedef VOID(WINAPI *pCreateProcessEvent)(DWORD, DWORD);
	pCreateProcessEvent CreateProcessEvent;

	typedef VOID(WINAPI *pCreateThreadEvent)(DWORD, DWORD);
	pCreateThreadEvent CreateThreadEvent;

	/*!
	*  �� �� ���� LoadPluginFile
	*  ��    �ڣ� 2020/06/17
	*  �������ͣ� VOID
	*  ��    �ܣ� ������
	*/
	VOID LoadPluginFile();

	/*!
	*  �� �� ���� GetProgramDir
	*  ��    �ڣ� 2020/06/17
	*  �������ͣ� CString
	*  ��    �ܣ� ��ȡ��������Ŀ¼
	*/
	CString GetProgramDir();

	/*!
	*  �� �� ���� CallCreateProcessEvent
	*  ��    �ڣ� 2020/06/17
	*  �������ͣ� VOID
	*  ��    ���� DWORD nPid �����Խ���ID
	*  ��    ���� DWORD nTid �������߳�ID
	*  ��    �ܣ� ����ȫ�������CreateProcessEvent����,�������̴����¼�������
	*/
	VOID CallCreateProcessEvent(DWORD nPid, DWORD nTid);

	/*!
	*  �� �� ���� CallCreateThreadEvent
	*  ��    �ڣ� 2020/06/17
	*  �������ͣ� VOID
	*  ��    ���� DWORD nHandle ���µ����¼����̵߳ľ��  _CREATE_THREAD_DEBUG_INFO�ṹ���� hThread �ֶ�ֵ
	*  ��    ���� DWORD nThreadLocalBase ָ�����ݿ��ָ�� _CREATE_THREAD_DEBUG_INFO�ṹ���� lpThreadLocalBase �ֶ�ֵ;
	*  ��    �ܣ� ����ȫ�������CreateThreadEvent�����������̴߳����¼�ʱ������ 
	*/
	VOID CallCreateThreadEvent(DWORD nHandle, DWORD nThreadLocalBase);

};

