#pragma once
#include "serial.h"
#include "globe.h"

#define MAX_RETURN_BYTES (100)

DWORD CommReadGASThread(LPVOID lpParam);
class CGAScomm :
	public CSerial
{
public:
	CGAScomm(void);
	~CGAScomm(void);

public:
	int iPos;
	char GAS_Buf[100];
	BYTE TCP_GAS_Buf[120];
	unsigned char RecvState;

	bool OpenGASComm();
	void Comm_ProcessDo(BYTE bRecv);
	bool FormatGASData(char *pGas_Buf,int ilen);
	BOOL Close();
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;
	HANDLE m_comThreadHandle;
};
