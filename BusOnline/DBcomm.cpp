#include "StdAfx.h"
#include "DBcomm.h"


/*这是一个线程读的例子，你可以在你的主程序里参考该函数写出你需要的函数*/
//参数为串口类指针
BOOL bIsServerConnect = FALSE;

DWORD CommReadDBThread(LPVOID lpParam)
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

CDBcomm::CDBcomm(void)
{
	m_dwRecvBytes=0;
	m_dwSendBytes=0;
	RecvState=NO_RECV;
}

CDBcomm::~CDBcomm(void)
{
}
bool CDBcomm::OpenDBComm()
{
	if (Comm_Open(4,57600))
	{
		//创建读串口线程
		if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_comThreadHandle,0);
			m_comThreadHandle=INVALID_HANDLE_VALUE;
		}
		m_comThreadHandle = CreateThread(NULL,0,CommReadDBThread,this,0,NULL);
		if (m_comThreadHandle == NULL)
		{
			TRACE(L"读串口线程创建失败");
			return false;
		}
		return true;
	}
	return false;
}
void CDBcomm::Comm_ProcessDo(BYTE bRecv)
{
	++m_dwRecvBytes;
	if (iPos>35)
	{
		RecvState=NO_RECV;
		iPos=0;
		memset(DB_Buf,0,sizeof(DB_Buf));
	}
	switch (RecvState)
	{
	case NO_RECV:
		if (bRecv==0xAA)
		{
			RecvState=START;
			DB_Buf[iPos++]=bRecv;
		}
		break;
	case RECVING:
		DB_Buf[iPos++]=bRecv;
		if (iPos> DB_Buf[2] + 1)
		{
			//if (DB_Buf[iPos - 1] == 0x55 && Checksum())
			if (DB_Buf[iPos - 1] == 0x55)
			{	
				RecvState=REVED;
				//
				DBPara();
				memset(DB_Buf,0,iPos);
				iPos=0;
				RecvState=NO_RECV;
			}
			else
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(DB_Buf,0,iPos);
			}
			
		}
		break;
	case START:
		DB_Buf[iPos++]=bRecv;
		if (iPos>2)
		{
	
			if (DB_Buf[1] == 0x01)
			{
				RecvState=RECVING;
			}
			else
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(DB_Buf,0,3);
			}
		}
		break;
	default:
		break;
	}
}
BOOL CDBcomm::Checksum()// 求和校验
{
	int sum = 0;
	for (int i = 1; i < DB_Buf[2] - 1; ++i)
	{
		sum+= DB_Buf[i];
	}	
	if(sum == MAKEWORD(DB_Buf[DB_Buf[2]-1],DB_Buf[DB_Buf[2]]))
	{
		return TRUE;
	}
	return FALSE;
	
}

void CDBcomm::DBPara()
{
	switch (DB_Buf[3])
	{
	case 0x01://广播回复
		memcpy(m_pBusInfo->LineNO, DB_Buf+5, 16);
		m_pBusInfo->StationCount = DB_Buf[16+5];
		m_pBusInfo->StationCur=DB_Buf[17+5];
		break;
	case 0x10:
		if (DB_Buf[4]==0x01)
		{
			CString strtmp;
			strtmp = _T("(Lin:") +m_pBusInfo->markID+ _T(";00)");
			CString2Char(strtmp,TCP_DB_Buf,sizeof(TCP_DB_Buf)); 
			AddTCP_Data((BYTE*)TCP_DB_Buf,strlen(TCP_DB_Buf),DBstart,DBend);
		}
		else if (DB_Buf[4]==0x00)
		{
			CString strtmp;
			strtmp = _T("(Lin:") + m_pBusInfo->markID + _T(";20)");
			CString2Char(strtmp,TCP_DB_Buf,sizeof(TCP_DB_Buf)); 
			AddTCP_Data((BYTE*)TCP_DB_Buf,strlen(TCP_DB_Buf),DBstart,DBend);
		}
		break;
	case 0x12:
		break;
	case 0x41:
		BusDispatchKeyDeal(DB_Buf[5]);
		break;
	case 0x30:
		if (DB_Buf[5]==0x00)
		{
			AfxMessageBox(_T("振铃执行失败"));
		}
		break;
	default:break;

	}
}
bool CDBcomm::BusDispatchKeyDeal( BYTE KeyValue)
{
	static UINT nMenuIndex = 0x00;
	BYTE LCDContent[4][16];
	DWORD nLen;
	DWORD dwWritten;
	unsigned char* pLcdContent = (unsigned char *) LCDContent;
	memset(pLcdContent,' ',64);

	switch (KeyValue)
	{
	case 0x01://切换键
		{	
			for(int i=0;i<40;i++)
			{
				if(m_pBusInfo->RingData[i].bValue && m_pBusInfo->RingData[i].bRinging &&m_pBusInfo->RingData[i].bRinged == FALSE)
				{
					m_pBusInfo->RingData[i].bRinging = FALSE;
					m_pBusInfo->RingData[i].bRinged = TRUE;
					m_pBusInfo->RingData[i].bValue = FALSE;
					BYTE bNewData[16];
					int nLen=16;
					m_pBusInfo->FormatRingTime(bNewData,2);
					char buf[100];
					sprintf(buf,"i=%d 打铃确认",i);
					m_strDebugInfo+=L"\r\n"+(CString)buf;	
#ifdef USING_LOG
					AddLog_Data((BYTE*)buf,strlen(buf),LogDBstart,LogDBend);
#endif	
					if(m_pBusInfo->Read4Record(m_pBusInfo->RingData[i].bMonth,m_pBusInfo->RingData[i].bDay,m_pBusInfo->RingData[i].bHour,m_pBusInfo->RingData[i].bMinute,2))
					{
						m_pBusInfo->Write4Send(bNewData);
						m_pBusInfo->Write4Record(m_pBusInfo->RingData[i].bMonth,m_pBusInfo->RingData[i].bDay,m_pBusInfo->RingData[i].bHour,m_pBusInfo->RingData[i].bMinute,2);
					}
					break;
				}
			}
		}
		nMenuIndex = 0x00;
		m_LCDMenu.SetLCDMenuPara(nMenuIndex);
		m_LCDMenu.LoadLCDMenu(nMenuIndex, pLcdContent);
		nLen = PacketBuf(pLcdContent, 64, 0x12);
		Comm_Write(Packetbuf, nLen, &dwWritten);
		m_dwSendBytes+=nLen;
		break;
	case 0x02://确定
		if (m_LCDMenu.nOk == INVILID) break;

		if (m_LCDMenu.nOk == BOTTOM)
		{
			if (nMenuIndex == 0xA0 || nMenuIndex == 0xA1)//主机参数
			{

				LCDDisInfo(pLcdContent,nMenuIndex);
				nLen = PacketBuf(pLcdContent,64,0x12);
				Comm_Write(Packetbuf, nLen,&dwWritten);
				m_dwSendBytes+=nLen;
				m_LCDMenu.nEsc = m_LCDMenu.nIndex;
				nMenuIndex = m_LCDMenu.nOk;
				m_LCDMenu.SetLCDMenuPara(nMenuIndex);

				break;
			}else if(nMenuIndex == 0x80 || nMenuIndex == 0x81 ||(nMenuIndex >= 0x20 && nMenuIndex <= 0x33))//固定信息 、进出总站
			{
				int ReplyCmdState;
				if (nMenuIndex == 0x80 || nMenuIndex == 0x81)
				{
					ReplyCmdState= nMenuIndex-0x80+60;
				}
				else if (nMenuIndex >= 0x20 && nMenuIndex <= 0x33)
				{
					ReplyCmdState= nMenuIndex-0x20+80;
				}
				

				SYSTEMTIME sysTime;    
				GetLocalTime(&sysTime);
				CString strtmp(_T(""));
				char infoline[100];
				memset(infoline,0x00,100);

				strtmp.Format(_T("(DPH:00;99;00;%d;%02d:%02d;;)"),ReplyCmdState,sysTime.wHour,sysTime.wMinute);
				int nByteLen = strtmp.GetLength()*2;
				char * pChar = UnicodeStrToAnsiStr( strtmp, &nByteLen );
				int nLen = nByteLen < 100 ? nByteLen : 100;
				memcpy(TCP_DB_Buf,pChar,nLen);
				delete pChar;		
				AddTCP_Data((BYTE*)TCP_DB_Buf,strlen(TCP_DB_Buf),DBstart,DBend);

				LCDDisInfo(pLcdContent,nMenuIndex);
				nLen = PacketBuf(pLcdContent, 64, 0x12);
				Comm_Write(Packetbuf, nLen, &dwWritten);
				m_dwSendBytes+=nLen;

				m_LCDMenu.nOk = INVILID;
				m_LCDMenu.nUp = INVILID;
				m_LCDMenu.nDown = INVILID;
				m_LCDMenu.nEsc = INVILID;

				Sleep(1000);

				m_LCDMenu.SetLCDMenuPara(nMenuIndex);
				m_LCDMenu.LoadLCDMenu(nMenuIndex,pLcdContent);
				nLen = PacketBuf(pLcdContent,64,0x12);
				Comm_Write(Packetbuf, nLen,&dwWritten);
				m_dwSendBytes+=nLen;
				break;
			}
			else if (nMenuIndex>=IndexFirst_4&&nMenuIndex<=IndexEnd_4)//调度信息
			{
				if (m_pBusInfo->InfCount>0)
				{
					for (int i=0;i<m_pBusInfo->InfCount;i++)
					{
						/*	CString strTemp;
						int iCur=(CurInf+8-i)%8-1;
						if (iCur<0) iCur=0;
						if (BusInf[iCur].bReply==false)
						{
						strTemp.Format(_T("信息%d(%s)"),i+1,_T("未读"));
						}else if (BusInf[iCur].bOK==TRUE)
						{
						strTemp.Format(_T("信息%d(%s)"),i+1,_T("同意"));
						}else if (BusInf[iCur].bOK==false)
						{
						strTemp.Format(_T("信息%d(%s)"),i+1,_T("不同意"));
						}	  
						LCD[2].AddMenueContent(i,strTemp);*/
					}
				}
				else
				{
					CString strContent=L"无调度信息";
					LCDDisInfo(pLcdContent,strContent);
					nLen = PacketBuf(pLcdContent, 64, 0x12);
					Comm_Write(Packetbuf, nLen, &dwWritten);
					m_dwSendBytes+=nLen;

					m_LCDMenu.nEsc = m_LCDMenu.nIndex;
					nMenuIndex = m_LCDMenu.nOk;
					m_LCDMenu.SetLCDMenuPara(nMenuIndex);
					break;
				}
			}
		}
		nMenuIndex = m_LCDMenu.nOk;
		m_LCDMenu.SetLCDMenuPara(nMenuIndex);
		m_LCDMenu.LoadLCDMenu(nMenuIndex, pLcdContent);
		nLen = PacketBuf(pLcdContent, 64, 0x12);
		Comm_Write(Packetbuf, nLen, &dwWritten);
		m_dwSendBytes+=nLen;
		break;
	case 0x03://返回
		if (m_LCDMenu.nEsc == INVILID) break;

		if (m_LCDMenu.nEsc == 0xFF)
		{
			//添加内容

			break;
		}
		nMenuIndex = m_LCDMenu.nEsc;
		m_LCDMenu.SetLCDMenuPara(nMenuIndex);
		m_LCDMenu.LoadLCDMenu(nMenuIndex,pLcdContent);
		nLen = PacketBuf(pLcdContent,64,0x12);
		Comm_Write(Packetbuf, nLen,&dwWritten);
		m_dwSendBytes+=nLen;
		break;
	case 0x04://向上
		if (m_LCDMenu.nUp == INVILID)	break;
		nMenuIndex = m_LCDMenu.nUp;
		m_LCDMenu.SetLCDMenuPara(nMenuIndex);
		m_LCDMenu.LoadLCDMenu(nMenuIndex,pLcdContent);
		nLen = PacketBuf(pLcdContent,64,0x12);
		Comm_Write(Packetbuf, nLen,&dwWritten);
		m_dwSendBytes+=nLen;
		break;
	case 0x05://向下
		if (m_LCDMenu.nDown == INVILID) break;
		nMenuIndex = m_LCDMenu.nDown;
		m_LCDMenu.SetLCDMenuPara(nMenuIndex);
		m_LCDMenu.LoadLCDMenu(nMenuIndex,pLcdContent);
		nLen = PacketBuf(pLcdContent,64,0x12);
		Comm_Write(Packetbuf, nLen,&dwWritten);
		m_dwSendBytes+=nLen;
		break;
	default:break;
	}
	return true;
}

int CDBcomm::PacketBuf(BYTE *buf, int len, BYTE cmd, BYTE addr)
{
	int i = 0;
	int sum = 0;
	Packetbuf[i++] = 0xAA;
	Packetbuf[i++] = addr;
	Packetbuf[i++] = len + 5;
	Packetbuf[i++] = cmd;

	for(int j = 0; j< len;++j)
	{
		Packetbuf[i++] =*(buf + j);
	}

	for (int j = 1; j < i; ++j)
	{
		sum += Packetbuf[j];
	}

	Packetbuf[i++] = 0x00FF & sum;
	Packetbuf[i++] = sum >> 8;
	
	Packetbuf[i] =0x55;
	return len + 7;
}

void CDBcomm::LCDDisInfo(BYTE *pLcdContent, BYTE nMenuIndex)
{
	memset(pLcdContent,' ',64);
	CString strContent;
	CString strTemp;
	strContent = m_LCDMenu.FindContentByIndex(nMenuIndex);
	if (strContent.Compare(_T("主机编号")) == 0)//查看主机编号
	{
		strTemp = strContent;
		int nByteLen = strTemp.GetLength()*2;
		char * pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		int nLen = nByteLen < 16 ? nByteLen : 16;
		memcpy( pLcdContent, pChar, nLen );

		strTemp=g_strBusCode.Right(g_strBusCode.GetLength()-g_strBusCode.Find(':')-1 );
		strTemp=strTemp.Left(strTemp.Find(')'));

		nByteLen = strTemp.GetLength()*2;
		pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		nLen = nByteLen < 32 ? nByteLen : 32;
		memcpy( pLcdContent+16, pChar, nLen );

		strTemp = _T("            返回");
		nByteLen = strTemp.GetLength()*2;
		pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		nLen = nByteLen < 32 ? nByteLen : 32;
		memcpy( pLcdContent+16*3, pChar, nLen );

		delete pChar;
	}
	else if (strContent.Compare(_T("软件版本")) == 0)//查看软件版本
	{
		strTemp = strContent;
		int nByteLen = strTemp.GetLength()*2;
		char * pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		int nLen = nByteLen < 16 ? nByteLen : 16;
		memcpy( pLcdContent, pChar, nLen );

		strTemp=g_strCodeVersion.Right(g_strCodeVersion.GetLength()-g_strCodeVersion.Find(':')-1 );;
		strTemp=strTemp.Left(strTemp.Find(')'));

		nByteLen = strTemp.GetLength()*2;
		pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		nLen = nByteLen < 32 ? nByteLen : 32;
		memcpy( pLcdContent+16, pChar, nLen );

		strTemp = _T("            返回");
		nByteLen = strTemp.GetLength()*2;
		pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		nLen = nByteLen < 32 ? nByteLen : 32;
		memcpy( pLcdContent+16*3, pChar, nLen );

		delete pChar;
	}
	else if ( nMenuIndex == 0x80 || nMenuIndex == 0x81 ||(nMenuIndex >= 0x20 && nMenuIndex <= 0x33))
	{

		if (bIsServerConnect == TRUE)
		{	
			char *pChar = "  发送成功";
			memcpy( pLcdContent+16, pChar, strlen(pChar) );
			delete pChar;
		}
		else 
		{
			char *pChar = "  发送失败";
			memcpy( pLcdContent+16, pChar, strlen(pChar) );
			delete pChar;
		}
	}
}

void CDBcomm::LCDDisInfo(BYTE *pLcdContent,CString strContent)
{
	CString strTemp=strContent;
	int nByteLen = strContent.GetLength()*2;
	if (nByteLen < 16)
	{
		for (int i = 0; i < (16-nByteLen)/2; ++i) 
		{
			strTemp =L" " + strTemp;
		}
		nByteLen = strTemp.GetLength()*2;
		char * pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		memcpy( pLcdContent+16, pChar, nByteLen );
	}
	else if (nByteLen >=16&&nByteLen<=16*3)
	{
		nByteLen = strTemp.GetLength()*2;
		char * pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		memcpy( pLcdContent, pChar, nByteLen );
	}
	else if (nByteLen>16*3)
	{
		nByteLen = strTemp.GetLength()*2;
		char * pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
		memcpy( pLcdContent, pChar, 48);
	}
	

	strTemp=L"            返回";
	nByteLen = strTemp.GetLength()*2;
	char * pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
	int nLen = nByteLen < 16 ? nByteLen : 16;
	memcpy( pLcdContent+16*3, pChar, nByteLen);
	delete pChar;
}



void  CDBcomm::PlayServiceVoice(BYTE bNum)
{
	int nByteLen=1;
	bNum-=1;
	unsigned char* pData=MakeDBCmd( 0x30, &bNum, &nByteLen ,0x20);
	Comm_WriteSyncPort(pData,nByteLen);
	m_dwSendBytes+=nByteLen;
	delete pData;
}
BYTE* CDBcomm::MakeDBCmd( BYTE nCmd, BYTE *pSrcData, int *pLen,BYTE Addr)
{
	//报站器命令格式
	//0xAA+ADDR+LEN+CMD+DATA0+…DATAN+SUML+SUMH+0x55
	int nLen = *pLen+7;;//总数据包长度

	BYTE *pData = new BYTE[nLen];
	memset( pData, 0, nLen );
	*(pData+0) = 0xAA;
	*(pData+1) = Addr;
	*(pData+2) = nLen-2;
	*(pData+3) = nCmd;

	memcpy( pData+4, pSrcData,*pLen ); 

	int nSum = 0;
	for( int i=1; i<nLen-3; i++ )
	{
		nSum = nSum + *(pData+i);
	}
	*(pData+nLen-3) = nSum%256;
	*(pData+nLen-2) = nSum/256;

	*(pData+nLen-1) = 0x55;

	*pLen = nLen;

	return pData;
}

char * CDBcomm::UnicodeStrToAnsiStr( CString strUni, int* pStrByteLen )
{
	int nUniLen = *pStrByteLen/2;
	unsigned short  *pUniData = new unsigned short[nUniLen];
	for( int i=0; i<nUniLen; i++)
	{
		if( i < strUni.GetLength() )
			*(pUniData+i) = strUni[i];
	}

	int nAnsiLen = WideCharToMultiByte( CP_ACP, NULL, (LPCWSTR)pUniData, nUniLen, NULL, 0, NULL, NULL );
	char  *pAnsiData = new char[nAnsiLen];
	WideCharToMultiByte( CP_ACP, NULL, (LPCWSTR)pUniData, nUniLen, pAnsiData, nAnsiLen, NULL, NULL );

	delete pUniData;

	*pStrByteLen = nAnsiLen;

	return pAnsiData;
}

void CDBcomm::SendCheckCmd()
{
	//DATA0-5:时间 (分钟小时日期星期月份年份)
	//DATA6: (上下行bit7=0 上行 bit7=1 下行, bit0-bit6 站点总数)
	//DATA7: (预报bit7=1,报站bit7=0, bit0-bit6 当前站点指示)
	//DATA8-9: 保留(固定0x00)
	SYSTEMTIME sysTime;    
	GetLocalTime(&sysTime);

	unsigned char chData[10];
	memset( chData, 0, 10 );

	chData[0] = (char)sysTime.wMinute; 
	chData[1] = (char)sysTime.wHour; 
	chData[2] = (char)sysTime.wDay; 
	chData[3] = (char)sysTime.wDayOfWeek; 
	chData[4] = (char)sysTime.wMonth; 
	chData[5] = (char)(sysTime.wYear - 2000); 
	chData[6] = 0xFF; 
	chData[7] = 0xFF; 
	bool newInf=false;
	BYTE newNum=0;

	for (int j=0;j<m_pBusInfo->InfCount;j++)
	{
		int iCur=(m_pBusInfo->CurInf+8-j)%8-1;
		if (iCur<0) iCur=0;
		if (iCur>=10)
		{
			iCur=9;
		}
		if (m_pBusInfo->InfPacket[iCur].bRead==false)
		{
			m_pBusInfo->InfPacket[iCur].bRead=true;
			//ModifyItemXMLState(iCur,BusInf[iCur].bRead,BusInf[iCur].bReply); 
			newInf=true;
		}
		if (m_pBusInfo->InfPacket[iCur].bReply==false)
		{
			newNum++;
		}
	}

	if (newInf)
	{
		chData[8] = newNum+0x80;
	}
	else
	{
		chData[8] = newNum;
	}
	int nByteLen = 10;
	unsigned char* pData=MakeDBCmd( 0x01, (BYTE*)chData, &nByteLen ,0x20);
	Comm_WriteSyncPort(pData,(DWORD)nByteLen);
	m_dwSendBytes+=nByteLen;
	delete pData;
}



BOOL CDBcomm::Close()
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
