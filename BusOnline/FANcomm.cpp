#include "StdAfx.h"
#include "FANcomm.h"

#define MAX_RETURN_BYTES (2+7+2)

/*这是一个线程读的例子，你可以在你的主程序里参考该函数写出你需要的函数*/
//参数为串口类指针
DWORD CommReadFANThread(LPVOID lpParam)
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
CFANcomm::CFANcomm(void)
{
	RecvState=NO_RECV;
	iPos=0;
	memset(FAN_Buf,NULL,sizeof(FAN_Buf));
	memset(bRecvFlag,NULL,sizeof(bRecvFlag));
	Index =0;
	m_dwRecvBytes=0;
	m_dwSendBytes=0;
}

CFANcomm::~CFANcomm(void)
{
}
bool CFANcomm::OpenFANComm()
{
	if (Comm_Open(1,9600))
	//if (Comm_Open(5,9600))
	{
		//创建读串口线程
		if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_comThreadHandle,0);
			m_comThreadHandle=INVALID_HANDLE_VALUE;
		}
		m_comThreadHandle = CreateThread(NULL,0,CommReadFANThread,this,0,NULL);
		if (m_comThreadHandle == NULL)
		{
			TRACE(L"读串口线程创建失败");
			return false;
		}
		return true;
	}
	return false;
}

void CFANcomm::Comm_ProcessDo(BYTE bRecv)
{
	++m_dwRecvBytes;
	if (iPos>MAX_RETURN_BYTES)
	{
		RecvState=NO_RECV;
		iPos=0;
		memset(FAN_Buf,0,sizeof(FAN_Buf));
	}
	switch (RecvState)
	{
	case NO_RECV:
		if (bRecv==0xAA)
		{
			RecvState=START;
			FAN_Buf[iPos++]=bRecv;
		}
		break;
	case RECVING:
		FAN_Buf[iPos++]=bRecv;
		if (FAN_Buf[iPos-1]==0x55&&iPos==(BYTE)FAN_Buf[3]+2)
		{
			RecvState=REVED;
			TRACE(L"数据接收完成\r\n");
			if (FAN_Buf[4]==0x04)
			{
				SetDataValue(FAN_Buf);
				FormatFANData(FAN_Buf,iPos);
			}
			memset(FAN_Buf,0,iPos);
			iPos=0;
			RecvState=NO_RECV;
		}
		break;
	case START:
		FAN_Buf[iPos++]=bRecv;
		if (iPos>4)
		{
			if (FAN_Buf[3]>MAX_RETURN_BYTES-2)//最大长度为20，不包括始末字节
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(FAN_Buf,0,4);
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
		memset(FAN_Buf,0,sizeof(FAN_Buf));
		break;
	}
}

bool CFANcomm::FormatFANData(char* pFAN_Buf,int ilen)
{
	int i=0;
	memset(TCP_FAN_Buf,NULL,sizeof(TCP_FAN_Buf));
	BYTE *pData=TCP_FAN_Buf;

	*(pData+i++)= '(';
	*(pData+i++)= 0x7E;
	*(pData+i++)= 0x7E;
	*(pData+i++)= 0xAA;
	*(pData+i++)= *(pFAN_Buf+1);
	*(pData+i++)= *(pFAN_Buf+2);
	*(pData+i++)= *(pFAN_Buf+3);
	*(pData+i++)= 0x01;
	*(pData+i++)= 0x03;
	*(pData+i++)= LOBYTE(Index);
	*(pData+i++)= HIBYTE(Index);
	*(pData+i++)= *(pFAN_Buf+6);
	*(pData+i++)= *(pFAN_Buf+7);
	WORD wSUM = 0;
	for (int j=3;j<i;++j)
	{
		wSUM+=*(pData+j);
	}
	*(pData+i++)= LOBYTE(wSUM);
	*(pData+i++)= HIBYTE(wSUM);
	*(pData+i++)= 0x55;
	*(pData+i++)= ')';
	AddTCP_Data(pData,i,FANstart,FANend);


	//char buf[100]={0};
	//char tempbuf[10];
	//for (int j=0;j<i;++j)
	//{
	//	sprintf(tempbuf,"%02x ",*(pData+j));
	//	strcat(buf,tempbuf);
	//}
	//m_Log.WriteLog(buf,strlen(buf),L"\\NAND\\FANdata.txt");

	return true;
	//*(pData+i++)= '(';
	//*(pData+i++)= 0x7E;
	//*(pData+i++)= 0x7E;
	//memcpy(pData+i,pFAN_Buf,ilen-1);
	//i+=ilen-1;
	//WORD wSUM = 0;
	//for (int j=3;j<i;++j)
	//{
	//	wSUM+=*(pData+j);
	//}
	//*(pData+i++)= (BYTE)Index;
	//*(pData+i++)= bRecvFlag[Index];
	////*(pData+i++)= LOBYTE(wSUM);
	////*(pData+i++)= HIBYTE(wSUM);
	//*(pData+i++)= 0x55;
	//*(pData+i++)= ')';
	//AddTCP_Data(pData,i,FANstart,FANend);

	//char buf[100]={0};
	//char tempbuf[10];
	//for (int j=0;j<i;++j)
	//{
	//	sprintf(tempbuf,"%02x ",*(pData+j));
	//	strcat(buf,tempbuf);
	//}
	//m_Log.WriteLog(buf,strlen(buf),L"\\NAND\\FANdata.txt");

}


void CFANcomm::SetDataValue(char* pFAN_Buf)
{
	if (0x04==*(pFAN_Buf+4))
	{
		memcpy(&DATA[Index],pFAN_Buf+5,2);
		bRecvFlag[Index]=true;
	}
}

void CFANcomm::DataPolling()
{
	memset(FAN_SendBuf,NULL,sizeof(FAN_SendBuf));
	BYTE *pData=FAN_SendBuf;
	int i=0;
	*(pData+i++)=0xAA;//起始位
	*(pData+i++)=0x10;//地址
	*(pData+i++)=0x20;//主机地址
	*(pData+i++)=6+1;//数据长度
	*(pData+i++)=0x04;//命令符
	*(pData+i++)=(BYTE)Index;//数据下标
	WORD wSUM=0;//校验和
	for (int j=1;j<i;++j)
	{
		wSUM+=*(pData+j);
	}
	*(pData+i++)= LOBYTE(wSUM);
	*(pData+i++)= HIBYTE(wSUM);
	*(pData+i++)= 0x55;//结束符
	Comm_WriteSyncPort(pData,(DWORD)i);
	m_dwSendBytes+=i;
}



BOOL CFANcomm::Close()
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
