// BusOnline.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "BusOnline.h"
#include "BusOnlineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBusOnlineApp

BEGIN_MESSAGE_MAP(CBusOnlineApp, CWinApp)
END_MESSAGE_MAP()


// CBusOnlineApp ����
CBusOnlineApp::CBusOnlineApp()
	: CWinApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CBusOnlineApp ����
CBusOnlineApp theApp;

// CBusOnlineApp ��ʼ��

BOOL CBusOnlineApp::InitInstance()
{

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	SetCursorPos( 1024, 168 );
	ShowCursor( false );

	CBusOnlineDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˴����ô����ʱ�á�ȷ�������ر�
		//  �Ի���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

