#include "stdafx.h"
#include "CCETCP.h"
#include <stdlib.h>
#include <stdio.h>


CCETCP::CCETCP()
{


	m_SendThread=NULL;
	m_SendThreadState = CLOSED;


	m_dwRecvBytes=0;
	m_dwSendBytes=0;


	//GetConfig();
}
BOOL CCETCP::RecvTime(BYTE* pData,int nDataLen)
{
	for(int i=0;i<nDataLen-3;i++)
	{
		//TRACE(L"%x ",pData[i]);
		if(pData[i] == '(' && pData[i+1] == 'T' && pData[i+2] == 'I' && pData[i+3] == 'M') 
		{
			if (nDataLen > i+9)
			{
				SYSTEMTIME sysTime;
				sysTime.wYear = pData[i+5]+2000;
				sysTime.wMonth = pData[i+6];
				sysTime.wDay = pData[i+7];
				sysTime.wHour = pData[i+8];
				sysTime.wMinute = pData[i+9];
				sysTime.wSecond = 0;
				sysTime.wMilliseconds = 0;

				if (SetLocalTime(&sysTime))
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
BOOL CCETCP::SplitData(BYTE* pData,int nDataLen)
{
	int nStart;
	BOOL bTemp = FALSE;
	for(int i=0;i<nDataLen-2;i++)
	{
		if(pData[i] == '(' && pData[i+1] == 0x7E && pData[i+2] == 0x7E) //收到7E7E
		{
			nStart=i+1;
			bTemp = TRUE;
			break;
		}
	}
	if (!bTemp)
	{
		return FALSE;
	}
	for(int i=0;i<40 ;i++)
	{
		m_pBusInfo->RingData[i].bMonth=0;
		m_pBusInfo->RingData[i].bDay=0;
		m_pBusInfo->RingData[i].bHour=0;
		m_pBusInfo->RingData[i].bMinute=0;
		m_pBusInfo->RingData[i].nRingNum=0;
		m_pBusInfo->RingData[i].bValue=FALSE;
		m_pBusInfo->RingData[i].bRinged=FALSE;
		m_pBusInfo->RingData[i].bRinging=FALSE;
	}
	WORD wRingLen = MAKEWORD(pData[nStart+3],pData[nStart+4]);
	if(pData[nStart + 7 + wRingLen] != 0x55)  //收到的结尾不是55
	{ 
		//回复 正文长度不对
		if(pData[nStart+2]== 0x49)  //收到0x49
		{
			BYTE bNewData[]={'(',0x7E,0x7E,0x49,0x01,0x00,0x00,0x4A,0x00,0x55,')'};
			AddTCP_Data(bNewData,sizeof(bNewData),TCPstart,TCPend);
		}
		else if(pData[nStart+2] == 0x4A)  //收到0x4A
		{
			BYTE bNewData[]={'(',0x7E,0x7E,0x4A,0x01,0x00,0x00,0x4B,0x00,0x55,')'};
			AddTCP_Data(bNewData,sizeof(bNewData),TCPstart,TCPend);
		}
#ifdef USING_LOG
		CString strContent=L"打铃数据正文长度不对";
		AddLog_Data(strContent,LogTCPstart,LogTCPend);
#endif
		return FALSE;
	}
	WORD wRingSum=0;  //校验和
	WORD wRingTrueSum=MAKEWORD(pData[nStart + wRingLen + 5],pData[nStart + wRingLen + 6]);
	//TRACE(L" 校验和是%d ",wRingTrueSum);
	for(int j = 0;j < wRingLen+3;j++)
	{
		wRingSum=wRingSum+pData[nStart + j + 2];
	}
	if(wRingSum == wRingTrueSum)  //校验和正确,开始写入
	{
		BOOL bReWrite = FALSE;
		if(pData[nStart+2] == 0x49)  //收到0x49
		{
			for(int n = 0;n < wRingLen/6;n++)
			{
				m_pBusInfo->RingData[n].bMonth = pData[nStart + 5 + n*6 + 1];
				m_pBusInfo->RingData[n].bDay = pData[nStart + 5 + n*6 + 2];
				m_pBusInfo->RingData[n].bHour = pData[nStart + 5 + n*6 + 3];
				m_pBusInfo->RingData[n].bMinute = pData[nStart + 5 + n*6 + 4];
				m_pBusInfo->RingData[n].nRingNum = pData[nStart + 5 + n*6 + 5];
				m_pBusInfo->RingData[n].bValue = TRUE;
			}
			m_pBusInfo->WriteToTXT();		
			m_pBusInfo->bIsRecv = TRUE;
			//回复 成功
			BYTE bNewData[]={'(',0x7E,0x7E,0x49,0x00,0x00,0x00,0x49,0x00,0x55,')'};
			AddTCP_Data(bNewData,sizeof(bNewData),TCPstart,TCPend);
#ifdef USING_LOG
			char buf[30]={0};
			for (int i=0;i<wRingLen/6;++i)
			{
				sprintf(buf,"%02d-%02d %02d:%02d",m_pBusInfo->RingData[i].bMonth
					,m_pBusInfo->RingData[i].bDay
					,m_pBusInfo->RingData[i].bHour
					,m_pBusInfo->RingData[i].bMinute);

				AddLog_Data((BYTE*)buf,strlen(buf),LogTCPstart,LogTCPend);
			}
#endif
		}
		else if(pData[nStart+2] == 0x4A)  //收到0x4A
		{
#ifdef USING_LOG
			CString strContent=L"打铃数据校验错误";
			AddLog_Data(strContent,LogTCPstart,LogTCPend);
#endif
			m_pBusInfo->ReadFromTXT();
			for(int n = 0;n < wRingLen/5;n++)
			{
				CString csTemp;
				BYTE bCMonth = pData[nStart + 5 + n*5 + 1];
				BYTE bCDay = pData[nStart + 5 + n*5 + 2];
				BYTE bCHour = pData[nStart + 5 + n*5 + 3];
				BYTE bCMinute = pData[nStart + 5 + n*5 + 4];
				/*csTemp.Format(L"收到的是%x,%x,%x,%x",bCMonth,bCDay,bCHour,bCMinute);
				AfxMessageBox(csTemp);*/
				for(int i=0;i<40;i++)
				{
					if (m_pBusInfo->RingData[i].bValue && bCMinute == m_pBusInfo->RingData[i].bMinute && bCHour == m_pBusInfo->RingData[i].bHour && bCDay == m_pBusInfo->RingData[i].bDay && bCMonth == m_pBusInfo->RingData[i].bMonth)
					{
						m_pBusInfo->RingData[i].bValue = FALSE;
						bReWrite = TRUE;
					}
				}
			}
			if(bReWrite)
			{
				m_pBusInfo->WriteToTXT();
				//回复 成功
			BYTE bNewData[]={'(',0x7E,0x7E,0x4A,0x00,0x00,0x00,0x4A,0x00,0x55,')'};
			Send((char*)bNewData,sizeof(bNewData));
			//AddPacket(bNewData,sizeof(bNewData));
			}
#ifdef USING_LOG
			strContent=L"从文件读取打铃数据";
			AddLog_Data(strContent,LogTCPstart,LogTCPend);
#endif
		}
#ifdef USING_LOG
		char buf[30]={0};
		for (int i=0;i<wRingLen/6;++i)
		{
			sprintf(buf,"%02d-%02d %02d:%02d",m_pBusInfo->RingData[i].bMonth
				,m_pBusInfo->RingData[i].bDay
				,m_pBusInfo->RingData[i].bHour
				,m_pBusInfo->RingData[i].bMinute);

			AddLog_Data((BYTE*)buf,strlen(buf),LogTCPstart,LogTCPend);
		}
#endif
		return TRUE;
	}
	else  
	{
		//回复 校验和不正确
		//TRACE(L"校验和不正确");
		if(pData[nStart+2] == 0x49)  //收到0x49
		{
			BYTE bNewData[]={'(',0x7E,0x7E,0x49,0x03,0x00,0x00,0x4C,0x00,0x55,')'};
			AddTCP_Data(bNewData,sizeof(bNewData),TCPstart,TCPend);
		}
		else if(pData[nStart+2] == 0x4A)  //收到0x4A
		{
			BYTE bNewData[]={'(',0x7E,0x7E,0x4A,0x03,0x00,0x00,0x4D,0x00,0x55,')'};
			AddTCP_Data(bNewData,sizeof(bNewData),TCPstart,TCPend);
			//AddPacket(bNewData,sizeof(bNewData));
		}
		return FALSE;
	}
	
	return TRUE;
}


//数据接收事件
void CCETCP::ReadData(const char * buf, DWORD dwBufLen)
{

   
}
bool CCETCP::TCPDataDeal(BYTE* pData,int ilen)
{
	if (ilen<3)
	{
		return false;
	}
	if(pData[1]=='T' && pData[2]=='I' && pData[3]=='M')
	{
        RecvTime(pData,ilen);
	}
	else if(pData[1]==0x7E && pData[2]==0x7E)
	{
		SplitData(pData,ilen);
	}
	else if(pData[1]=='H' && pData[2]=='R' && pData[3]=='T')
	{
		
	}
	else if(pData[1]=='L' && pData[2]=='i' && pData[3]=='n')
	{
		//更改线路，暂时不需要
	}
	else if(pData[1]=='F' && pData[2]=='t' && pData[3]=='p')
	{
		m_FTP.FTPDownload(pData,ilen);
	}
	else if(pData[1]=='D' && pData[2]=='P' && pData[3]=='H')
	{
		m_pBusInfo->SetDespathInf(pData,ilen);
	}
	else if(pData[1]=='W' && pData[2]=='A' && pData[3]=='R')
	{
		m_pBusInfo->SetRingInf(pData,ilen);
	}
	else if(pData[1]=='M' && pData[2]=='s' && pData[3]=='g')
	{
	}
	return true;
}
bool CCETCP::OnReceive( char* buf, int len )
{
	m_dwRecvBytes+=len;
	static BYTE Data[301];
	static int iPos=0;
	static int iState=0;

	BYTE *pData=(BYTE*)buf;
	for (int i=0;i<len;i++)
	{
		if (iPos>300)
		{
			iPos=0;
			iState=0;
		}
		switch(iState)
		{
		  case 0://没有找到‘（’
			  if (pData[i]=='(')
			  { 
                  Data[iPos++]=pData[i];
				  iState=1;
			  }
			  break;
		  case 1:
			  Data[iPos++]=pData[i];
			  if (pData[i]==')')
			  { 
				  iState=2;
			  }
			  if (iState==2)
			  {
                  TCPDataDeal(Data,iPos);
				  iPos=0;
				  iState=0;
			  }
			  break;
		  default:
			  break;
		}
		
	}
	if(buf!=NULL)
	{
       delete[] buf;
    }
    return true;
}
void OnReceive()
{

}

bool CCETCP::LinkServer()
{
	Disconnect();
	Close();
	Create(SOCK_STREAM);
	if (Connect(m_strHostIP,m_dwPort))
	{
		char tempbuf[128];
		CString2Char(m_strBusCode,tempbuf,sizeof(tempbuf));
		Send(m_strBusCode);
#ifdef USING_LOG
		AddLog_Data(m_strBusCode,LogTCPstart,LogTCPend);
		Sleep(1000);
#endif

		CString2Char(g_strCodeVersion,tempbuf,sizeof(tempbuf));
		Send(g_strCodeVersion);
#ifdef USING_LOG
		AddLog_Data(g_strCodeVersion,LogTCPstart,LogTCPend);
		Sleep(1000);
#endif
		m_SendThread= CreateThread(NULL, 0, SendThreadFun, this, 0, NULL );
		return true;
	}
	return false;
}
DWORD WINAPI CCETCP::SendThreadFun(LPVOID pParam)
{
	CCETCP *parent = (CCETCP*) pParam;
	parent->m_SendThreadState = RUNNING;
	do 
	{
		parent->SendTCPPacket();
		Sleep(1);
	} while (parent->m_SendThreadState == RUNNING);
	parent->m_SendThreadState = CLOSED;
	parent->m_SendThread = NULL;
	return 0;
}
bool CCETCP::IsConnect()
{
   if (GetSocketState()==CCESocket::CONNECTED)
   {
	   return true;
   }
   return false;
}

bool CCETCP::AddTCPData(Data_Packet* tcpData)
{
   list_TCPData.AddTail(tcpData);
   return true;
}

/********************************
13-10-30 添加m_strSendComment记录发送内容
********************************/
bool CCETCP::SendTCPPacket()
{
	static unsigned int nCount=0;
	Data_Packet* pData;
	if(!list_TCPData.IsEmpty()) 
	{
			pData = list_TCPData.GetHead();
			if (pData->bNeedDeal==true &&pData->bUsing==false)
			{
				pData->bUsing=true;
				Send((char*)pData->Data,pData->isize);
				pData->bNeedDeal=false;
				pData->bUsing=false;

				m_dwSendBytes+=pData->isize;

				if (nCount>20)
				{
					nCount=0;
					m_strSendComment.Empty();
				}
				m_strSendComment+=(CString)pData->Data;
				m_strSendComment+=L" ";
				++nCount;
			}
			while(!list_TCPData.IsLast())
			{
				Sleep(10);
				pData = list_TCPData.GetNext();
				if (pData->bNeedDeal==true &&pData->bUsing==false)
				{
					pData->bUsing=true;
					Send((char*)pData->Data,pData->isize);
					pData->bNeedDeal=false;
					pData->bUsing=false;

					m_dwSendBytes+=pData->isize;

					if (nCount>20)
					{
						nCount=0;
						m_strSendComment.Empty();
					}
					m_strSendComment+=(CString)pData->Data;
					m_strSendComment+=L" ";
					++nCount;
				}
		   }
	}
	return true;
}



CCETCP::~CCETCP()
{
	int watchDog;
	watchDog = 0;
	while (m_SendThread)
	{
		SetThreadPriority(m_SendThread, THREAD_PRIORITY_HIGHEST);
		if((::WaitForSingleObject(m_SendThread, 1000) != WAIT_TIMEOUT) ||
			watchDog >= THREAD_TERMINATION_MAXWAIT)
		{
			CloseHandle(m_SendThread);
			m_SendThread = NULL;
		}
		watchDog++;
	}
}
//得到本地的IP地址
CString CCETCP::GetLocalIP()
{
	HOSTENT *LocalAddress;
	char	*Buff;
	TCHAR	*wBuff;
	CString strReturn = _T("");

	//创建新的缓冲区
	Buff = new char[256];
	wBuff = new TCHAR[256];
	//置空缓冲区
	memset(Buff, '\0', 256);
	memset(wBuff, TEXT('\0'), 256*sizeof(TCHAR));
	//得到本地计算机名
	if (gethostname(Buff, 256) == 0)
	{
		//转换成双字节字符串
		mbstowcs(wBuff, Buff, 256);
		//得到本地地址
		LocalAddress = gethostbyname(Buff);
		//置空buff
		memset(Buff, '\0', 256);
		//组合本地IP地址
		sprintf(Buff, "%d.%d.%d.%d\0", LocalAddress->h_addr_list[0][0] & 0xFF,
			LocalAddress->h_addr_list[0][1] & 0x00FF, LocalAddress->h_addr_list[0][2] & 0x0000FF, LocalAddress->h_addr_list[0][3] & 0x000000FF);
		//置空wBuff
		memset(wBuff, TEXT('\0'), 256*sizeof(TCHAR));
		//转换成双字节字符串
		mbstowcs(wBuff, Buff, 256);
		//设置返回值
		strReturn = wBuff;
	}
	else
	{
	}

	//释放Buff缓冲区
	delete[] Buff;
	Buff = NULL;
	//释放wBuff缓冲区
	delete[] wBuff;
	wBuff = NULL;
	return strReturn;
}

//获取配置
void CCETCP::GetConfig()
{
	FILE *fp=NULL;
	fp = fopen("\\NAND\\config.txt","rb");
	if (fp!=NULL)
	{
		char buf[512];
		CString strTemp;
		while(fgets(buf,1024,fp)!=NULL)
		{
			strTemp = buf;
			if (strTemp.Find(_T("BusCode"))!=-1)
			{
				m_strBusCode = strTemp.Mid(strTemp.Find('='),strTemp.Find(';')-strTemp.Find('='));
			}

			if (strTemp.Find(_T("HostIp"))!=-1)
			{
				m_strHostIP = strTemp.Mid(strTemp.Find('='),strTemp.Find(';')-strTemp.Find('='));
			}
			if (strTemp.Find(_T("HostPort"))!=-1)
			{
				int i=0;
				while (buf[i++]!= '=');
			
				char tempbuf[20];
				int j=0;
				while (buf[i]!=';')
				{
					tempbuf[j++]=buf[i++];
				}
				tempbuf[j] ='\0';
				m_dwPort = atoi(tempbuf);
			}	
		} 
		fclose(fp);
	}
}

void CCETCP::SetConfig(CString BusCode,CString HostIP, DWORD Port)
{
	m_strBusCode = BusCode;
	m_strHostIP = HostIP;
	m_dwPort = Port;
}


BOOL CCETCP::Close()
{
	TerminateThread(m_SendThread,0);
	if (m_SendThread!=NULL)
	{
		CloseHandle(m_SendThread);
		m_SendThread = NULL;	
	}

	return true;
}
