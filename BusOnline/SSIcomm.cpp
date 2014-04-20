#include "StdAfx.h"
#include "SSIcomm.h"


/*����һ���̶߳������ӣ�������������������ο��ú���д������Ҫ�ĺ���*/
//����Ϊ������ָ��
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


	ReplyCMD=0x00; //����ȵ������ظ�����
	CMD=0x00;//������������
	nDataSize=0;//Ҫ���͵��ļ���С
	nRemainingBytes=0;//ʣ�µ�Ҫ���͵ĸ���

	numRead=0;//�������ļ��ж�ȡ���ֽڸ���
	memset(sendbuf,NULL,sizeof(sendbuf));
	FrameNO=0;//���͵�����֡���
	nBytesSend=0;//���͵��ֽ���
	bUpdate=TRUE;//��Ҫ����״̬��TRUE����Ҫ���»����ڸ��£�FALSE ����Ҫ���»���½���
	bReply=FALSE;//�з���

	fp=NULL;//�����ļ����
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
		//�����������߳�
		if (m_comThreadHandle!=INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_comThreadHandle,0);
			m_comThreadHandle=INVALID_HANDLE_VALUE;
		}
		m_comThreadHandle = CreateThread(NULL,0,CommReadSSIThread,this,0,NULL);
		if (m_comThreadHandle == NULL)
		{
			TRACE(L"�������̴߳���ʧ��");
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
			TRACE(L"���ݽ������\r\n");
			if (SSI_Buf[4]==SSI_Buf[5]&&(SSI_Buf[4]>=0x01&&SSI_Buf[4]<=0x03))
			{
				TRACE(L"���� ");
				FormatSSIData(SSI_Buf,iPos);
				//TCP_Send_SSI();
			}
			else if (bUpdate == TRUE)
			{
				TRACE(L"���� ");
				if (UpdateState!=SEND)
				{
					FormatSSIData(SSI_Buf,iPos);;//���͵�������
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
			if (SSI_Buf[3]>20)//��󳤶�Ϊ20��������ʼĩ�ֽ�
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
		//����ȵ�����ȷ�ϳ���������ѯ���� ������ѯ����� 
		switch(ReplyCMD)
		{
		case 0x01: bUpdate=FALSE;//�Ա�����ͬ���ݵģ�����Ҫ����
			TRACE(L"�ѱ�����ͬ���ݣ�����Ҫ����\r\n");
			strDebugInfo+=L"�ѱ�����ͬ���ݣ�����Ҫ����\r\n";
			break;
		case 0x02: bUpdate=TRUE;//׼����������
			TRACE(L"׼����������");
			strDebugInfo+=L"׼����������\r\n";
			UpdateState=SEND;
			break;
		case 0x03: bUpdate=FALSE;//���ܽ������ݣ����ݹ���
			TRACE(L"���ܽ������ݣ����ݹ���");
			strDebugInfo+=L"���ܽ������ݣ����ݹ���\r\n";
			break;
		case 0x04: bUpdate=FALSE;//���ܽ������ݣ�Ӳ���ͺŲ���
			TRACE(L"���ܽ������ݣ�Ӳ���ͺŲ���");
			strDebugInfo+=L"���ܽ������ݣ�Ӳ���ͺŲ�\r\n";
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
			//�����ļ�
			CString strFilePath=(CString)g_SDCardDir+strFileName;
			WIN32_FIND_DATA fdNew;
			HANDLE hNewFind = ::FindFirstFile( strFilePath, &fdNew );
		
			if (hNewFind == INVALID_HANDLE_VALUE)//SD����û�г���
			{
				bUpdate = FALSE;
				strDebugInfo+=L"\r\n!!!\r\nSD����û�г���\r\n!!!�ӳ�����������Ŀ¼�и���\r\n";

				strFilePath=(CString)g_WorkDir+(CString)strFileName;
				hNewFind = ::FindFirstFile( strFilePath, &fdNew );
				if (hNewFind == INVALID_HANDLE_VALUE) //����Ŀ¼��û�и��³���
				{
					bUpdate = FALSE;
					strDebugInfo+=L"����Ŀ¼���Ҳ�������\r\n";
				}
				else
				{
					::FindClose(hNewFind);
				}

			}
			else
			{
				strDebugInfo+=L"��SD���и���\r\n";
				//�������ƶ�����ǰִ���ļ�·��
				int rc = CopyFile( strFilePath, (CString)g_WorkDir+(CString)strFileName ,0);
				strDebugInfo+=L"�ƶ�������Ŀ¼\r\n";

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
					fseek(fp,0L,SEEK_END); // ��λ���ļ�ĩβ 
					nRemainingBytes=ftell(fp); // �õ�ʣ���ļ����ļ���С 
					nRemainingBytes = nRemainingBytes-512-32;

					fseek(fp,512,SEEK_SET);//�����ļ�ͷ��512���ֽ�
					fseek(fp,3,SEEK_CUR);//����У��;
					unsigned char buf[32];
					fread(buf,sizeof(char),29,fp);
					
					//��ѯ�Ƿ���Ҫ����
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
					fseek(fp,512L,SEEK_SET); // ���¶�λ���������� 


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
				case 0x30://������ȷ
					TRACE(L"������ȷ ");
					break;
				case 0x31:
					{
						TRACE(L"����֡���� ");
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
					TRACE(L"���ݴ������");
					bUpdate=FALSE;
					numRead=0;
					FrameNO=0;
					nBytesSend=0;
					ResetUpdateState();
					UpdateState=FINISH;
					strDebugInfo+=L"�������\r\n";
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
