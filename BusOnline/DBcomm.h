#pragma once
#include "serial.h"
#include "LCDMenu.h"
#include "BusInfo.h"
#include "globe.h"

extern BOOL bIsServerConnect;

DWORD CommReadDBThread(LPVOID lpParam);

class CDBcomm :
	public CSerial
{
public:
	CDBcomm(void);
	~CDBcomm(void);


public:
	bool OpenDBComm();
    virtual void Comm_ProcessDo(BYTE bRecv);
	BOOL Checksum();
	void DBPara();
	bool BusDispatchKeyDeal( BYTE KeyValue);
	int PacketBuf(BYTE *buf, int len, BYTE cmd, BYTE addr=0x20);
	void LCDDisInfo(BYTE *pLcdContent, BYTE nMenuIndex);
	void LCDDisInfo(BYTE *pLcdContent,CString strContent);
	char* UnicodeStrToAnsiStr( CString strUni, int* pStrByteLen );
	void PlayServiceVoice(BYTE bNum);//���ŷ�������
	BYTE* MakeDBCmd( BYTE nCmd, BYTE *pSrcData, int *pLen,BYTE Addr);
	void SendCheckCmd();//��վ�����Ͳ�ѯ����
public:
	HANDLE m_comThreadHandle;
	int iPos;

	char DB_Buf[100];
	char TCP_DB_Buf[120];
	unsigned char  RecvState;
	BYTE Packetbuf[80];
	//BYTE CMDType;//������������	
public:
	CLCDMenu m_LCDMenu;
	CBusInfo *m_pBusInfo;
	void GetBusInfo(CBusInfo* pBusInfo)
	{
		m_pBusInfo=pBusInfo;
	};
	BOOL Close();
	CString m_strDebugInfo;

public:
	DWORD m_dwSendBytes;
	DWORD m_dwRecvBytes;


};
