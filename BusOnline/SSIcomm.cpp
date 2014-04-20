#include "StdAfx.h"
#include "SSIcomm.h"


/*这是一个线程读的例子，你可以在你的主程序里参考该函数写出你需要的函数*/
//参数为串口类指针
DWORD CommReadSSIThread(LPVOID lpParam)
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

CSSIcomm:: CSSIcomm(void)
{
	RecvState=NO_RECV;
	iPos=0;
	memset(SSI_Buf,NULL,sizeof(SSI_Buf));
	for (int i = 0; i < 5; ++i)
	{
		TCP_Packet[i].bNeedDeal=false;
		TCP_Packet[i].bUsing=false;
		TCP_Packet[i].isize=0;
	}

	bUpdate=FALSE;
	UpdateState=CHECH;
	m_dwRecvBytes=0;
	m_dwSendBytes=0;


	ReplyCMD=0x00; //满意度调查器回复命令
	CMD=0x00;//车载主机命令
	nDataSize=0;//要发送的文件大小
	nRemainingBytes=0;//剩下的要发送的个数

	numRead=0;//从升级文件中读取的字节个数
	memset(sendbuf,NULL,sizeof(sendbuf));
	FrameNO=0;//发送的数据帧序号
	nBytesSend=0;//发送的字节数
	bUpdate=TRUE;//需要更新状态，TRUE，需要更新或正在更新，FALSE 不需要更新或更新结束
	bReply=FALSE;//有返回

	fp=NULL;//升级文件句柄
}
CSSIcomm::~CSSIcomm(void)
{
}
bool CSSIcomm::OpenSSIComm()
{
	if (Comm_Open(5,9600))
	//if (Comm_Open(8,9600))
	//if (Comm_Open(1,9600))
	{
		//创建读串口线程
		if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_comThreadHandle,0);
			m_comThreadHandle=INVALID_HANDLE_VALUE;
		}
		m_comThreadHandle = CreateThread(NULL,0,CommReadSSIThread,this,0,NULL);
		if (m_comThreadHandle == NULL)
		{
			TRACE(L"读串口线程创建失败");
			return false;
		}
		return true;
	}
	return false;
}
void CSSIcomm::Comm_ProcessDo(BYTE bRecv)
{
	++m_dwRecvBytes;
	if (iPos>25)
	{
		RecvState=NO_RECV;
		iPos=0;
		memset(SSI_Buf,0,sizeof(SSI_Buf));
	}
	switch (RecvState)
	{
	case NO_RECV:
		if (bRecv==0xAA)
		{
			RecvState=START;
			SSI_Buf[iPos++]=bRecv;
		}
		break;
	case RECVING:
		SSI_Buf[iPos++]=bRecv;
		if (SSI_Buf[iPos-1]==0x55&&iPos==(BYTE)SSI_Buf[3]+2)
		{
			RecvState=REVED;
			TRACE(L"数据接收完成\r\n");
			if (SSI_Buf[4]==SSI_Buf[5]&&(SSI_Buf[4]>=0x01&&SSI_Buf[4]<=0x03))
			{
				TRACE(L"工作 ");
				FormatSSIData(SSI_Buf,iPos);
				//TCP_Send_SSI();
			}
			else if (bUpdate == TRUE)
			{
				TRACE(L"升级 ");
				if (UpdateState!=SEND)
				{
					FormatSSIData(SSI_Buf,iPos);;//发送到服务器
				}
				UpdateSSISoftWare();

				if (FINISH==UpdateState)
				{

				}
			}
			memset(SSI_Buf,0,iPos);
			iPos=0;
			RecvState=NO_RECV;
		}
		break;
	case START:
		SSI_Buf[iPos++]=bRecv;
		if (iPos>4)
		{
			if (SSI_Buf[3]>20)//最大长度为20，不包括始末字节
			{
				RecvState=NO_RECV;
				iPos=0;
				memset(SSI_Buf,0,4);
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
		memset(SSI_Buf,0,sizeof(SSI_Buf));
		break;
	}
}


BOOL CSSIcomm::UpdateSSISoftWare(CString strFileName)
{
	CMD=SSI_SendBuf[4];
	if (CMD!=0x10&&CMD!=0x20&&CMD!=NULL)
	{
		return FALSE;
	}
	ReplyCMD = SSI_Buf[5];

	bReply=TRUE;

	if(CMD==0x10)
	{
		//满意度调查器确认车载主机查询命令 主机查询命令符 
		switch(ReplyCMD)
		{
		case 0x01: bUpdate=FALSE;//以保存相同数据的，不需要接收
			TRACE(L"已保存相同数据，不需要接收\r\n");
			strDebugInfo+=L"已保存相同数据，不需要接收\r\n";
			break;
		case 0x02: bUpdate=TRUE;//准备接收数据
			TRACE(L"准备接收数据");
			strDebugInfo+=L"准备接收数据\r\n";
			UpdateState=SEND;
			break;
		case 0x03: bUpdate=FALSE;//不能接收数据，数据过大
			TRACE(L"不能接收数据，数据过大");
			strDebugInfo+=L"不能接收数据，数据过大\r\n";
			break;
		case 0x04: bUpdate=FALSE;//不能接收数据，硬件型号不符
			TRACE(L"不能接收数据，硬件型号不符");
			strDebugInfo+=L"不能接收数据，硬件型号不\r\n";
			break;
		default:
			bUpdate=FALSE;
			return FALSE;
			break;
		}
	}
	
	
	if (bUpdate==FALSE)
	{
		if(fp!=NULL)
		{
			fclose(fp);
			fp=NULL;
		}
		return FALSE;
	}

	switch (UpdateState)
	{
	case CHECH:
		{
			//查找文件
			CString strFilePath=(CString)g_SDCardDir+strFileName;
			WIN32_FIND_DATA fdNew;
			HANDLE hNewFind = ::FindFirstFile( strFilePath, &fdNew );
		
			if (hNewFind == INVALID_HANDLE_VALUE)//SD卡中没有程序
			{
				bUpdate = FALSE;
				strDebugInfo+=L"\r\n!!!\r\nSD卡中没有程序\r\n!!!从车载主机工作目录中更新\r\n";

				strFilePath=(CString)g_WorkDir+(CString)strFileName;
				hNewFind = ::FindFirstFile( strFilePath, &fdNew );
				if (hNewFind == INVALID_HANDLE_VALUE) //工作目录中没有更新程序
				{
					bUpdate = FALSE;
					strDebugInfo+=L"工作目录中找不到程序\r\n";
				}
				else
				{
					::FindClose(hNewFind);
				}

			}
			else
			{
				strDebugInfo+=L"从SD卡中更新\r\n";
				//将更新移动到当前执行文件路径
				int rc = CopyFile( strFilePath, (CString)g_WorkDir+(CString)strFileName ,0);
				strDebugInfo+=L"移动到工作目录\r\n";

				if( !rc )
					rc = GetLastError( );
				::FindClose(hNewFind);
			}
			
		//	else
			{
	
				if (fp!=NULL)
				{
					fclose(fp);
				}
				fp=_wfopen(strFilePath,L"rb");
				if (fp!=NULL)
				{
					bUpdate=TRUE;
					fseek(fp,0L,SEEK_END); // 定位到文件末尾 
					nRemainingBytes=ftell(fp); // 得到剩余文件的文件大小 
					nRemainingBytes = nRemainingBytes-512-32;

					fseek(fp,512,SEEK_SET);//跳过文件头，512个字节
					fseek(fp,3,SEEK_CUR);//跳过校验;
					unsigned char buf[32];
					fread(buf,sizeof(char),29,fp);
					
					//查询是否需要升级
					int i=0;
					SSI_SendBuf[i++]=0xAA;
					SSI_SendBuf[i++]=0x40;
					SSI_SendBuf[i++]=0x20;
					SSI_SendBuf[i++]=28+6;
					SSI_SendBuf[i++]=0x10;
					for (int j=0;j<28;++j)
					{
						SSI_SendBuf[i++]=buf[j];
					}
					WORD SUM=0x00;
					for (int j=1;j<i;++j)
					{
						SUM+=SSI_SendBuf[j];
					}
					SSI_SendBuf[i++]=0x00FF&SUM;
					SSI_SendBuf[i++]=0x00FF&(SUM>>8);
					SSI_SendBuf[i++]=0x55;

					
					memcpy(&nDataSize,buf+20,4);
					fseek(fp,512L,SEEK_SET); // 重新定位到描述部分 


					Sleep(1);
					if (Comm_WriteSyncPort((BYTE*)SSI_SendBuf,(DWORD) i))
					{
						m_dwSendBytes+=i;
					}
				}  
			}
		break;
		}
	case SEND:
		{
			if (CMD==0x20)
			{
				switch(ReplyCMD)
				{
				case 0x30://接收正确
					TRACE(L"接收正确 ");
					break;
				case 0x31:
					{
						TRACE(L"接收帧错误 ");
						memcpy(&FrameNO,SSI_Buf+8,2);
						nBytesSend-=numRead;
						fseek(fp,nBytesSend+512+32,SEEK_SET);
						break;
					}
				
				default:
					break;
				}
			}

			fseek(fp,512+nBytesSend,SEEK_SET);
			memset(sendbuf,NULL,sizeof(sendbuf));
			numRead=fread(sendbuf,1,128,fp);
			
			int i=0;
			SSI_SendBuf[i++]=0xAA;
			SSI_SendBuf[i++]=0x40;
			SSI_SendBuf[i++]=0x20;
			SSI_SendBuf[i++]=numRead+2+6;
			SSI_SendBuf[i++]=0x20;
			SSI_SendBuf[i++]=FrameNO&0x00FF;
			SSI_SendBuf[i++]=(FrameNO>>8)&0x00FF;

			memcpy(SSI_SendBuf+i,sendbuf,numRead);
			i+=numRead;
			WORD SUM=0x00;
			for (int j=1;j<i;++j)
			{
				SUM+=SSI_SendBuf[j];
			}
			SSI_SendBuf[i++]=0x00FF&SUM;
			SSI_SendBuf[i++]=0x00FF&(SUM>>8);
			SSI_SendBuf[i++]=0x55;

			++FrameNO;
			Sleep(1);
			if (Comm_WriteSyncPort((BYTE*)SSI_SendBuf,(DWORD) i))
			{
				m_dwSendBytes+=i;
				nBytesSend+=numRead;
				nRemainingBytes-=numRead;
				strDebugInfo.Format(L"%d %d of %d\r\n",FrameNO,nBytesSend,nDataSize);
				TRACE(strDebugInfo);
				if (nRemainingBytes == 0||nBytesSend >= nDataSize)
				{
					TRACE(L"数据传输完毕");
					bUpdate=FALSE;
					numRead=0;
					FrameNO=0;
					nBytesSend=0;
					ResetUpdateState();
					UpdateState=FINISH;
					strDebugInfo+=L"升级完毕\r\n";
				}
			}
		}
		break;
	case FINISH:
		break;
	default:
		break;
	}

	if (bUpdate==FALSE)
	{
		if(fp!=NULL)
		{
			fclose(fp);
			fp=NULL;
		}
	}
	return TRUE;
}

void CSSIcomm::ResetUpdateState()
{
	if (fp!=NULL)
	{
		fclose(fp);
	}
	UpdateState=CHECH;
	bUpdate=TRUE;
	FrameNO=0;
	nBytesSend=0;
	memset(SSI_SendBuf,NULL,sizeof(SSI_SendBuf));
}


bool CSSIcomm::FormatSSIData(char* pSSI_Buf,int ilen)
{
	int i=0;
	memset(TCP_SSI_Buf,NULL,sizeof(TCP_SSI_Buf));
	BYTE *pData=TCP_SSI_Buf;
	*(pData+i++)= '(';
	*(pData+i++)= 0x7E;
	*(pData+i++)= 0x7E;
	*(pData+i++)= 0xAA;
	*(pData+i++)= 0x20;
	*(pData+i++)= 0x40;
	memcpy(pData+i,pSSI_Buf+3,ilen-4);
	i+=ilen-4;
	WORD wSUM = 0;
	for (int j=3;j<i;++j)
	{
		wSUM+=*(pData+j);
	}
	*(pData+i++)= LOBYTE(wSUM);
	*(pData+i++)= HIBYTE(wSUM);
	*(pData+i++)= 0x55;
	*(pData+i++)= ')';
	AddTCP_Data(pData,i,SSIstart,SSIend);
	return true;
}


BOOL CSSIcomm::QueryDriverInfo()
{
	BYTE Query[9];
	int i=0;
	Query[i++]=0xAA;
	Query[i++]=0x40;
	Query[i++]=0x20;
	Query[i++]=7;
	Query[i++]=0x03;
	Query[i++]=0x00;
	WORD wSUM = 0;
	for (int j=1;j<i;++j)
	{
		wSUM+=Query[j];
	}
	Query[i++]= LOBYTE(wSUM);
	Query[i++]= HIBYTE(wSUM);
	Query[i++]= 0x55;

	m_dwSendBytes+=i;
	Comm_WriteSyncPort(Query,(DWORD)i);


	return true;
}
BOOL CSSIcomm::Close()
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
