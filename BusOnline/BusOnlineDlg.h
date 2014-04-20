#pragma once
#include "GPScomm.h"
#include "CANcomm.h"
#include "SSIcomm.h"
#include "DBcomm.h"
#include "FANcomm.h"
#include "GAScomm.h"
#include "MyRAS.h"
#include "CCETCP.h"
#include "update.h"
#include "MemoryCtrl.h"
#include "CMemoryPool.h"
#include "Log.h"
#include <assert.h>


// CBusOnlineDlg �Ի���
class CBusOnlineDlg : public CDialog
{
// ����
public:
	CBusOnlineDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_BUSONLINE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedGenatering();
	afx_msg void OnClose();
	afx_msg void OnBnClickedClose();

public:
	CGPScomm m_GPScomm;//GPS
	CCANcomm m_CANcomm;//can����
	CSSIcomm m_SSIcomm;//����ȵ�����
	CDBcomm m_DBcomm;//��վ��
	CFANcomm m_FANcomm;//���ӷ���
	CGAScomm m_GAScomm;//©��������

#ifdef USING_LOG
	CLog m_log;
#endif;

	//void PlayServiceVoice(BYTE bNum);//�������Ѵ���
	BOOL GetConfig(BOOL bRun=1);//��ȡ������ţ�IP��ַ���Ͷ˿ں�,bRun=0,��dnloadconfig.txt  bRun=1����config.txt
	bool DownloadExeFile( );//ִ�и��³���
	void CloseSysteemResource();//�ر���Դ
	void OpenSerialPort();//�򿪴���
private:
	int iHeartBeatCount;//
	int bDeviceReset;
public:
    CRAS_CE m_RasDial;
	CCETCP  m_TCP;
	CUpdate m_Update;
	CBusInfo m_BusInfo;
	CString m_DebugInf;
	CString m_BusCode;
	CString m_VerNo;
	CString m_ServerIP;
	DWORD m_dwPort;
	CString m_LocalIP;

	CString m_strHostIP;
	CString m_strBusCode;

	DWORD m_GPSRecvBytes;
	DWORD m_GPSSendBytes;
	DWORD m_CANRecvBytes;
	DWORD m_CANSendBytes;
	DWORD m_SSIRecvBytes;
	DWORD m_SSISendBytes;
	DWORD m_FanSendBytes;
	DWORD m_FanRecvBytes;
	DWORD m_GasSendBytes;
	DWORD m_GasRecvBytes;
	DWORD m_DBSendBytes;
	DWORD m_DBRecvBytes;

	CString m_strTCPSend;
	DWORD m_TCPSendBytes;
	DWORD m_TCPRecvBytes;
public:
	CMemoryCtrl  memoryctrl;
	DWORD m_dwMemUsed;
	DWORD m_dwMemTotal;

	CString m_BootTime;
	CString m_RingData;
};
