#include "StdAfx.h"
#include "CANcomm.h"

/*这是一个线程读的例子，你可以在你的主程序里参考该函数写出你需要的函数*/
//参数为串口类指针
DWORD CommReadCANThread(LPVOID lpParam)
{
	HANDLE hComm;
	CSerial* pSerial;
	pSerial = (CSerial*)lpParam;
	hComm = pSerial->Comm_GetHandle();
	DWORD dwReaded, dwMask;
	BYTE btRecv;

	if(hComm != INVALID_HANDLE_VALUE)
		::SetCommMask(hComm,EV_RXCHAR);
	while(pSerial->Comm_GetHandle() != INVALID_HANDLE_VALUE)
	{
		hComm = pSerial->Comm_GetHandle();
		::WaitCommEvent(hComm,&dwMask,NULL);
		::SetCommMask(hComm,EV_RXCHAR);
		if(dwMask & EV_RXCHAR)
		{
			do
			{
				::ReadFile(hComm,&btRecv,1,&dwReaded,NULL);
				if(dwReaded == 1)
				{
					pSerial->Comm_Process(btRecv);
				}
			}
			while(dwReaded == 1);
		}
	}
	return 0;
}

CCANcomm::CCANcomm(void)
{
	RecvState=NO_RECV;
	iPos=0;
	memset(CAN_Buf,NULL,sizeof(CAN_Buf));

	m_dwRecvBytes=0;
	m_dwSendBytes=0;

	ResetCanSendState();
}

CCANcomm::~CCANcomm(void)
{
}

bool CCANcomm::OpenCANComm()
{
	if (Comm_Open(3,115200,PT_EVEN))
	//if (Comm_Open(3,115200))
	{
		//创建读串口线程
		if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_comThreadHandle,0);
			m_comThreadHandle=INVALID_HANDLE_VALUE;
		}
		m_comThreadHandle = CreateThread(NULL,0,CommReadCANThread,this,0,NULL);
		if (m_comThreadHandle == NULL)
		{
			TRACE(L"读串口线程创建失败");
			return false;
		}
		return true;
	}
	return false;
}
void CCANcomm::Comm_ProcessDo(BYTE bRecv)
{
	m_dwRecvBytes++;
	if (iPos>300)
	{
		RecvState=NO_RECV;
		iPos=0;
		memset(CAN_Buf,0,sizeof(CAN_Buf));
	}
	switch (RecvState)
	{
	case NO_RECV:
		if (bRecv==0xAA)
		{
			iPos=0;
			RecvState=START;
			CAN_Buf[iPos++]=bRecv;
		}
		break;
	case RECVING:
		CAN_Buf[iPos++]=bRecv;
		if (bRecv == 0x55 && iPos == (BYTE)CAN_Buf[1] +2)
		{
			RecvState=REVED;
			FormatCANData(CAN_Buf,iPos);
			memset(CAN_Buf,NULL,iPos);
			iPos=0;
			RecvState=NO_RECV;
		}
		break;
	case START:
		CAN_Buf[iPos++]=bRecv;
		if (iPos>3)
		{
			if ((CAN_Buf[2] >= 0x01 && CAN_Buf[2] <= 0x05 )
				|| CAN_Buf[2] == 0x10 )
			{
				if ((CAN_Buf[3] >= 0x01 && CAN_Buf[3] <= 0x06))
				{
					RecvState = RECVING;
					break;
				}
				else
				{
					RecvState=NO_RECV;
					iPos=0;
					memset(CAN_Buf,NULL,4);
				}
			}
			else
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(CAN_Buf,NULL,4);
			}

			if (CAN_Buf[1] > 90)
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(CAN_Buf,NULL,4);
			}

		
		}
		break;
	default:
		break;
	}
	return;
}



void CCANcomm::FormatCANData(char *pCAN_Buf,int ilen)
{

	WORD wS;
	memcpy(&wS,pCAN_Buf+4,2);
	if (m_CanSended[wS]==TRUE&&!(pCAN_Buf[2]==0x02&&pCAN_Buf[3]==0x02))
	{
		return ;
	}
	else
	{
		m_CanSended[wS]=TRUE;
	}
	memset(TCP_CAN_Buf,NULL,sizeof(TCP_CAN_Buf));
	BYTE *pData=TCP_CAN_Buf;
	int i=0;
	*(pData+i++) = '(';
	*(pData+i++)= 0x7E;
	*(pData+i++)= 0x7E;
	*(pData+i++)= 0xAA; 
	*(pData+i++)= 0x20;
	*(pData+i++)= 0x50;
	memcpy(pData+i,pCAN_Buf+1,ilen-2);
	i+=ilen-2;
	WORD wSUM = 0;
	for (int j=3;j<i;++j)
	{
		wSUM+=*(pData+j);
	}
	*(pData+i++)= LOBYTE(wSUM);
	*(pData+i++)= HIBYTE(wSUM);
	*(pData+i++)= 0x55;
	*(pData+i++)= ')';
	AddTCP_Data(pData,i,CANstart,CANend);
	return;
}


BOOL CCANcomm::Close()
{
	Comm_Close();
	TerminateThread(m_comThreadHandle,0);
	if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
	{
		TerminateThread(m_comThreadHandle,0);
		CloseHandle(m_comThreadHandle);
		m_comThreadHandle=INVALID_HANDLE_VALUE;
	}
	return true;
}

BOOL CCANcomm::ResetCanSendState()
{
	memset(m_CanSended,NULL,MAXCANNUM);
	return TRUE;
}