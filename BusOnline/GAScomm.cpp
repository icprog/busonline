#include "StdAfx.h"
#include "GAScomm.h"


/*这是一个线程读的例子，你可以在你的主程序里参考该函数写出你需要的函数*/
//参数为串口类指针
DWORD CommReadGASThread(LPVOID lpParam)
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


CGAScomm::CGAScomm(void)
{
	RecvState=NO_RECV;
	iPos=0;
	memset(GAS_Buf,NULL,sizeof(GAS_Buf));
	for (int i = 0; i < 10; ++i)
	{
		TCP_Packet[i].bNeedDeal=false;
		TCP_Packet[i].bUsing=false;
		TCP_Packet[i].isize=0;
	}
	m_dwRecvBytes=0;
	m_dwSendBytes=0;
}

CGAScomm::~CGAScomm(void)
{
}
bool CGAScomm::OpenGASComm()
{
	if (Comm_Open(8,9600))
	//if (Comm_Open(5,9600))
	{
		//创建读串口线程
		if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_comThreadHandle,0);
			m_comThreadHandle=INVALID_HANDLE_VALUE;
		}
		m_comThreadHandle = CreateThread(NULL,0,CommReadGASThread,this,0,NULL);
		if (m_comThreadHandle == NULL)
		{
			TRACE(L"读串口线程创建失败");
			return false;
		}
		return true;
	}
	return false;
}


void CGAScomm::Comm_ProcessDo(BYTE bRecv)
{
	++m_dwRecvBytes;
	if (iPos>MAX_RETURN_BYTES)
	{
		RecvState=NO_RECV;
		iPos=0;
		memset(GAS_Buf,0,sizeof(GAS_Buf));
	}
	switch (RecvState)
	{
	case NO_RECV:
		if (bRecv==0xAA)
		{
			RecvState=START;
			GAS_Buf[iPos++]=bRecv;
		}
		break;
	case RECVING:
		GAS_Buf[iPos++]=bRecv;
		if (GAS_Buf[iPos-1]==0x55&&iPos==(BYTE)GAS_Buf[3]+2)
		{
			RecvState=REVED;
			FormatGASData(GAS_Buf,iPos);
			memset(GAS_Buf,0,iPos);
			iPos=0;
			RecvState=NO_RECV;
		}
		break;
	case START:
		GAS_Buf[iPos++]=bRecv;
		if (iPos>4)
		{
			if(GAS_Buf[4]>0xFF||GAS_Buf[4]<0x01)
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(GAS_Buf,0,5);
			}
			else if (GAS_Buf[3]>MAX_RETURN_BYTES-2)//最大长度为MAX_RETURN_BYTES-2，不包括始末字节
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(GAS_Buf,0,5);
			}
			else
			{
				RecvState=RECVING;
			}
		}
		break;
	default:
		RecvState=NO_RECV;
		iPos=0;
		memset(GAS_Buf,0,sizeof(GAS_Buf));
		break;
	}
}

bool CGAScomm::FormatGASData(char *pGas_Buf,int ilen)
{
	memset(TCP_GAS_Buf,NULL,sizeof(TCP_GAS_Buf));
	BYTE*  pData=TCP_GAS_Buf;
	int i=0;
	*(pData + i++ ) = '(';
	*(pData + i++ ) = 'G';
	*(pData + i++ ) = 'A';
	*(pData + i++ ) = 'S';
	*(pData + i++ ) = ':';
	memcpy(pData+i,GAS_Buf,GAS_Buf[3]+2);
	i+=GAS_Buf[3]+2;
	*(pData + i++) = ')';
	AddTCP_Data(pData,i,GASstart,GASend);

	return true;
}



BOOL CGAScomm::Close()
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
