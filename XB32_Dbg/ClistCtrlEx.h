#pragma once


// ClistCtrlEx

class ClistCtrlEx : public CListCtrl
{
	DECLARE_DYNAMIC(ClistCtrlEx)

public:
	ClistCtrlEx();
	virtual ~ClistCtrlEx();

protected:
	DECLARE_MESSAGE_MAP()

public:

	/*!
	*  �� �� ���� SetSelectionEx
	*  ��    �ڣ� 2020/06/06
	*  �������ͣ� VOID
	*  ��    ���� DWORD nItem �������
	*  ��    �ܣ� ѡ���б��һ�У�����������Ϊ����
	*/
	VOID SetSelectionEx(DWORD nItem);

	/*!
	*  �� �� ���� IsSelectionItem
	*  ��    �ڣ� 2020/06/06
	*  �������ͣ� BOOL
	*  ��    ���� DWORD nItem �������
	*  ��    �ܣ� ���ָ�����Ƿ�ѡ�У�ѡ�з���TRUE,ûѡ�з���FALSE
	*/
	BOOL IsSelectionItem(DWORD nItem);


	/*!
	*  �� �� ���� GetSelectionItem
	*  ��    �ڣ� 2020/06/07
	*  �������ͣ� DWORD
	*  ��    �ܣ� ��ȡѡ�е��������
	*/
	DWORD GetSelectionItem();

	/*!
	*  �� �� ���� GetSelectionSubItem
	*  ��    �ڣ� 2020/06/12
	*  �������ͣ� DWORD
	*  ��    �ܣ� ��ȡѡ���������
	*/
	DWORD GetSelectionSubItem();

};


