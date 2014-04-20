#pragma once
#include "serial.h"
#include "globe.h"

#define  MAXCANNUM 1024

DWORD CommReadCANThread(LPVOID lpParam);
class CCANcomm :
	public CSerial
{
public:
	CCANcomm(void);
	~CCANcomm(void);
public:
	int iPos;
	void Comm_ProcessDo(BYTE bRecv);
	bool OpenCANComm();
	void FormatCANData(char *pCAN_Buf,int ilen);
	BOOL Close();
	BOOL ResetCanSendState();
public:

	
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;
	char CAN_Buf[300];
	BYTE TCP_CAN_Buf[320];
	unsigned char RecvState;
	BOOL m_CanSended[MAXCANNUM];//�ѷ��ͱ�ǣ��ڹ涨��ʱ���ڣ�������������ͬ������

	HANDLE m_comThreadHandle;
	Data_Packet Log_Packet[3];
};
