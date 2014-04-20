//#include "BusOnlineDlg.h"
#include "StdAfx.h"
#include "GPScomm.h"
#include "Serial.h"



/*这是一个线程读的例子，你可以在你的主程序里参考该函数写出你需要的函数*/
//参数为串口类指针
DWORD CommReadGPSThread(LPVOID lpParam)
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

CGPScomm::CGPScomm(void)
{
	RecvState=NO_RECV;
	iPos=0;
	memset(GPRMC_Buf,0,sizeof(GPRMC_Buf));
	TCP_Packet[0].bNeedDeal=false;
	TCP_Packet[0].bUsing=false;
	TCP_Packet[0].isize=0;

	TCP_Packet[1].bNeedDeal=false;
	TCP_Packet[1].bUsing=false;
	TCP_Packet[1].isize=0;

	TCP_Packet[2].bNeedDeal=false;
	TCP_Packet[2].bUsing=false;
	TCP_Packet[2].isize=0;

	m_dwRecvBytes=0;
	m_dwSendBytes=0;
}

CGPScomm::~CGPScomm(void)
{
}
bool CGPScomm::OpenGPSComm()
{
	if (Comm_Open(2,9600))
	{
		//创建读串口线程
		if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_comThreadHandle,0);
			m_comThreadHandle=INVALID_HANDLE_VALUE;
		}
		m_comThreadHandle = CreateThread(NULL,0,CommReadGPSThread,this,0,NULL);
		if (m_comThreadHandle == NULL)
		{
			TRACE(L"读串口线程创建失败");
			return false;
		}
		return true;
	}
	return false;
}
void CGPScomm::Comm_ProcessDo(BYTE bRecv)
{
	m_dwRecvBytes++;

	if (iPos>90)
	{
		RecvState=NO_RECV;
		iPos=0;
		memset(GPRMC_Buf,0,sizeof(GPRMC_Buf));
	}
	switch (RecvState)
	{
	case NO_RECV:
		if (bRecv=='$')
		{
			RecvState=START;
			GPRMC_Buf[iPos++]=bRecv;
		}
		break;
	case RECVING:
		GPRMC_Buf[iPos++]=bRecv;
		if (bRecv=='*')
		{
			RecvState=REVED;
			GPSPara();
			FormatGPSData(GPRMC_Buf,iPos);
			memset(GPRMC_Buf,0,iPos);
			iPos=0;
			RecvState=NO_RECV;
		}
		break;
	case START:
		GPRMC_Buf[iPos++]=bRecv;
		if (iPos>5)
		{
			if ((GPRMC_Buf[3]=='R')&&(GPRMC_Buf[4]=='M')&&(GPRMC_Buf[5]=='C'))
			{
				RecvState=RECVING;
			}else
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(GPRMC_Buf,0,6);
			}
		}
		break;
	default:
		break;
	}
	return;
}
void CGPScomm::StrucGPRMInit()
{
    memset(&GPSRMC,'0',sizeof(GPSRMC));
	GPSRMC.UTCTime[6]='.';
    GPSRMC.Status='V';
	GPSRMC.Latitude[4]='.';
	GPSRMC.NS='N';
	GPSRMC.Longitude[5]='.';
	GPSRMC.EW='E';
	GPSRMC.Speed[3]='.';
	GPSRMC.Course[3]='.';
}
void CGPScomm::GPSPara()
{
		char* pGPSbuf=GPRMC_Buf;
		int iCommaNO=0;
		int iCount=0;
		StrucGPRMInit();
		while (*pGPSbuf != '*' && *pGPSbuf != '\0')
		{
			if (*pGPSbuf==',')
			{
				iCommaNO++;
				iCount=0;
				pGPSbuf ++;
				continue;
			}
			switch(iCommaNO)
			{
			case 1: //UTC标准定位时间
				GPSRMC.UTCTime[iCount++]=*pGPSbuf;
				break;
			case 2: //定位状态A A= 资料有效; V = 资料无效
				GPSRMC.Status=*pGPSbuf;
				break;
			case 3: //纬度
				GPSRMC.Latitude[iCount++]=*pGPSbuf;
				break;
			case 4: //北/南半球指示器
				GPSRMC.NS=*pGPSbuf;
				break;
			case 5: //经度
				GPSRMC.Longitude[iCount++]=*pGPSbuf;
				break;
			case 6: //东/西半球指示器
				GPSRMC.EW=*pGPSbuf;
				break;
			case 7: //对地速度 节
				GPSRMC.Speed[iCount++]=*pGPSbuf;
				break;
			case 8: //对地方向
				GPSRMC.Course[iCount++]=*pGPSbuf;
				break;
			case 9: //UTC日期 日日月月年年
				GPSRMC.UTCDate[iCount++]=*pGPSbuf;
				break;
			default:
				break;
			}
			pGPSbuf ++;
		}
}
bool CGPScomm::GPSSetTime(char UTCime[10],char UTCDate[6])
{
	SYSTEMTIME GPSTime;
	GPSTime.wYear = 2000+(UTCDate[4]-'0')*10 + (UTCDate[5]-'0');
	GPSTime.wMonth = (UTCDate[2]-'0')*10 + (UTCDate[3]-'0');
	GPSTime.wDay = (UTCDate[0]-'0')*10 + (UTCDate[1]-'0');
	GPSTime.wHour = (UTCime[0]-'0')*10 + (UTCime[1]-'0');
	GPSTime.wMinute = (UTCime[2]-'0')*10 + (UTCime[3]-'0');
	GPSTime.wSecond = (UTCime[4]-'0')*10 + (UTCime[5]-'0');

	SetSystemTime(&GPSTime);
	return true;
}



bool CGPScomm::FormatGPSData(char *pGPRMC_Buf,int ilen)
{
	static int iCount=0;
	BYTE*  pData;
	BYTE   btTemp=0;
	if (iCount%5==0)
	{
		int i=0;
		pData=TCP_GPRMC_Buf;
		*(pData + i++ ) = '(';
		*(pData + i++ ) = 'G';
		*(pData + i++ ) = 'p';
		*(pData + i++ ) = 's';
		*(pData + i++ ) = ':';
		memcpy(pData+i,GPSRMC.UTCTime,9);i+=9;
		memcpy(pData+i,&GPSRMC.Status,1);i+=1;
		memcpy(pData+i,GPSRMC.Latitude,9);i+=9;
		memcpy(pData+i,&GPSRMC.NS,1);i+=1;
		memcpy(pData+i,GPSRMC.Longitude,10);i+=10;
		memcpy(pData+i,&GPSRMC.EW,1);i+=1;
		memcpy(pData+i,GPSRMC.UTCDate,6);i+=6;
		btTemp='J';
		memcpy(pData+i,&btTemp,1);i+=1; 
		memcpy(pData+i,GPSRMC.Course,5);i+=5;
		btTemp='S';
		memcpy(pData+i,&btTemp,1);i+=1; 
		memcpy(pData+i,GPSRMC.Speed,5);i+=5;
		*(pData + i++) = ')';

		AddTCP_Data(TCP_GPRMC_Buf,i,GPSstart,GPSend);
		
	}
	if (iCount%10 == 0)
	{
		GPSSetTime(GPSRMC.UTCTime,GPSRMC.UTCDate);
	}
	iCount++;
	return true;
}



BOOL CGPScomm::Close()
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
