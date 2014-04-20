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


// CBusOnlineDlg 对话框
class CBusOnlineDlg : public CDialog
{
// 构造
public:
	CBusOnlineDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_BUSONLINE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
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
	CCANcomm m_CANcomm;//can总线
	CSSIcomm m_SSIcomm;//满意度调查器
	CDBcomm m_DBcomm;//报站器
	CFANcomm m_FANcomm;//电子风扇
	CGAScomm m_GAScomm;//漏气报警器

#ifdef USING_LOG
	CLog m_log;
#endif;

	//void PlayServiceVoice(BYTE bNum);//发车提醒打铃
	BOOL GetConfig(BOOL bRun=1);//获取主机编号，IP地址，和端口号,bRun=0,打开dnloadconfig.txt  bRun=1，打开config.txt
	bool DownloadExeFile( );//执行更新程序
	void CloseSysteemResource();//关闭资源
	void OpenSerialPort();//打开串口
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
