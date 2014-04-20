#include "StdAfx.h"
#include "CEFTP.h"

CCEFTP::CCEFTP(void)
{

	m_strFileName = g_FTPListFile;
	m_strDnloadDir = g_FTPDnloadDir;
	m_FileDnloadInfo.dwDnloadedSize = 0;
	m_FileDnloadInfo.dwFileSize = 0;
	m_FileDnloadInfo.strFileName = L"";

	InitFileList();//��ʼ���ļ��б�ṹ��
	m_strFileName=L"FTP"+m_FileList.strListFileName;//��ʼ��FTP�����ļ�����
	m_FTPDir=g_FTPDir;//��ʼ��FTP����Ŀ¼
	bNewExe_Flag=FALSE;//���س���������±��
	bDownloading		= false;//����״̬���
	bNewConfig_Flag	= false;//�����ļ����±��
	bNewSSI_Flag=FALSE;//����ȵ��������±��
	bResetDevice=FALSE;//����������־

	CreateDirectory(m_strDnloadDir,NULL);
	CreateDirectory(m_strDnloadDir+L"backup",NULL);
	DeleteTempdFile(m_strDnloadDir+L"*.tmp",0);

	
}

CCEFTP::~CCEFTP(void)
{
}


void CCEFTP::FTPDownload(BYTE* pData,int ilen)
{
	if(pData[1]=='F' && pData[2]=='t' && pData[3]=='p')
	{
		CString strMessage=(CString)pData;

		strMessage = strMessage.Right(strMessage.GetLength()-5);
		strMessage = strMessage.Left(strMessage.GetLength()-1);

		markID = strMessage.Left(strMessage.Find(';'));
		strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		m_FTPServer = strMessage.Left(strMessage.Find(';'));
		strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		m_FTPUserName = strMessage.Left(strMessage.Find(';'));
		strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		m_FTPPassword = strMessage.Left(strMessage.Find(';'));
		strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		m_FTPPort = _wtoi(strMessage.Left(strMessage.Find(';')));
		strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		m_FTPFiletype = strMessage.Left(strMessage.Find(';'));
		strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		m_RetFile = strMessage;

		if (bDownloading==TRUE)//����û��ִ����
		{
			CString strtmp;
			strtmp = _T("(Ftp:") +markID + _T(";1)");
			USES_CONVERSION;
			char* chtmp = T2A(strtmp); 
			AddTCP_Data((BYTE*)chtmp,strlen(chtmp),FTPstart,FTPend);
			return;
		}

		if (m_FTPFiletype==L"exe"||m_FTPFiletype==L"cfg"||m_FTPFiletype=="lst"||m_FTPFiletype==L"ssi")
		{
			m_FTPDir=m_RetFile.Left(m_RetFile.Find(';'))+_T("\\");
			m_strFileName=m_RetFile.Right(m_RetFile.GetLength()-(m_RetFile.Find(';')+1));
			m_strFileName=m_strFileName.Left(m_strFileName.Find(';'));
		}
	

		static int nTimes=0;
		Disconnect();
		Sleep(5);
		Create(SOCK_STREAM);
		if (Connect(m_FTPServer,m_FTPPort))
		{
			char buf[128];
			char tempbuf[128];

			memset(buf,NULL,128);
			strcpy(buf,"user ");
			memset(tempbuf,NULL,128);
			WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_FTPUserName,wcslen(m_FTPUserName)
				,tempbuf,wcslen(m_FTPUserName),NULL,NULL);
			strcat(buf,tempbuf);
			strcat(buf,"\r\n");
			SendData(buf,strlen(buf));

			memset(buf,NULL,128);
			strcpy(buf,"pass ");
			memset(tempbuf,NULL,128);
			WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_FTPPassword,wcslen(m_FTPPassword)
				,tempbuf,wcslen(m_FTPPassword),NULL,NULL);
			strcat(buf,tempbuf);
			strcat(buf,"\r\n");
			SendData(buf,strlen(buf));

			nTimes=0;
			//bDownloading=TRUE;
		}
		else
		{
			CString strtmp;
			strtmp = _T("(Ftp:") +markID + _T(";1)");
			USES_CONVERSION;
			char* chtmp = T2A(strtmp); 
			AddTCP_Data((BYTE*)chtmp,strlen(chtmp),FTPstart,FTPend);

			if (++nTimes>2)
			{
				nTimes=0;
				bResetDevice=TRUE;
			}
			return;
		}
	}
}


BOOL CCEFTP::SendData(const char * buf , DWORD dwBufLen)
{
	int nBytes = 0;
	int nSendBytes=0;

	while (nSendBytes < dwBufLen)
	{
		nBytes = send(s,buf+nSendBytes,dwBufLen-nSendBytes,0);
		if (nBytes==SOCKET_ERROR )
		{
			int iErrorCode = WSAGetLastError();
			//�ر�socket
			Disconnect();
			//�Ͽ�
			return FALSE;
		}
		//�ۼƷ��͵��ֽ���
		nSendBytes = nSendBytes + nBytes;
		if (nSendBytes < dwBufLen)
		{
			Sleep(1000);
		}
	} 
	return TRUE; 
}

bool CCEFTP::OnReceive(char* buf, int len)
{
	FILE *fp=NULL;
	FILE* fp_temp=NULL;
	static int nFileIndex=0;//��Ҫ���ص��ļ�������
	static INT nDnloadMode = 1; //1 ȫ�����أ�2 �����ϴ�δ�������
	static INT nErrCount = 0;//����һ���ļ��������
	static DWORD dwFileSize=0;//���ص��ļ���С
	static BOOL bDnloadFail = FALSE;//����ʧ��
	static const int nMinusBytes=2;
	static BOOL bMinusBytesFlag=FALSE;

	CString strRecv=(CString)buf;
	CString strMessage=strRecv;
	strMessage+="\r\n";

	strDebugInfo+=strMessage;
#ifdef USING_LOG
	AddLog_Data((BYTE*)buf,strlen(buf),LogFTPstart,LogFTPend);
#endif
	CString strCode;
	static char cmdbuf[128]={0};
	while (strMessage.Find(L"\r\n")!=-1)
	{
		strCode=strMessage.Left(3);
		strMessage=strMessage.Right(strMessage.GetLength()-(strMessage.Find(L"\r\n")+2));
		switch(_wtoi(strCode.GetBuffer(0)))
		{
		case 220:
			bDownloading=TRUE;
			break;
		case 331:
			bDownloading=TRUE;
			break;
		case 230://���ù���Ŀ¼
			{
				bDownloading=TRUE;
				char tempbuf[128];
				memset(tempbuf,NULL,128);
				WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_FTPDir,wcslen(m_FTPDir)
					,tempbuf,wcslen(m_FTPDir),NULL,NULL);
				strcpy(cmdbuf,"cwd ");
				strcat(cmdbuf,tempbuf);
				strcat(cmdbuf,"\r\n");
				SendData(cmdbuf,strlen(cmdbuf));
				TRACE(L"���ù���Ŀ¼\r\n");
				break;
			}	
		case 250://���뱻��ģʽ
			bDownloading=TRUE;
			strcpy(cmdbuf,"pasv\r\n");
			SendData(cmdbuf,strlen(cmdbuf));
			TRACE(L"���뱻��ģʽ");
			break;
		case 227://���ݴ���ģʽ
			{
				bDownloading=TRUE;
				static int nTimes=0;
				if((m_dataSocket=createDataSocket(buf)) != -1)
				{
					nTimes=0;
					strcpy(cmdbuf,"type i\r\n");
					SendData(cmdbuf,strlen(cmdbuf));
					TRACE(L"���ݴ���ģʽ\r\n");
				}
				else
				{
			
					TRACE(L"���ݴ���ͨ������ʧ��\r\n");
					bDownloading=FALSE;	
					bDnloadFail=TRUE;
					if (++nTimes>4)//������ν�������ͨ��ʧ������������
					{					
						
						nTimes=0;
						bResetDevice=TRUE;
					}
				}
				break;
			}
			
		case 200:
			{
				bDownloading=TRUE;
				//�ж϶ϵ����������ļ��Ƿ����,�����ļ��� ���ص��ļ������Ϻ�׺.temp��������ڣ���ȡ�Ѿ������ֽ�����
				m_FileDnloadInfo.strFileName = m_strFileName;
				fp=_wfopen(m_strDnloadDir+m_strFileName+L".tmp",L"r");
				nDnloadMode = 1;
				if(fp!=NULL&&fread(&m_FileDnloadInfo.dwDnloadedSize,sizeof(m_FileDnloadInfo.dwDnloadedSize),1,fp) != NULL)
				{
						//�Ѿ������ļ���С�ͼ�¼���Ƿ�һ�£���һ���������������ļ�
						WIN32_FIND_DATAW wfd;
						HANDLE hFile = FindFirstFile(m_strDnloadDir+m_strFileName,&wfd);

						if (m_FileDnloadInfo.dwDnloadedSize!=wfd.nFileSizeLow)
						{
							DeleteFile(m_strDnloadDir+m_strFileName+L".tmp"); //�������,ɾ����ʱ�ļ�
							DeleteFile(m_strDnloadDir+m_strFileName); //�������,ɾ����ʱ�ļ�
							m_FileDnloadInfo.dwDnloadedSize=0;
							nDnloadMode=1;//��������
							break;
						}

						//���ϴ����صĺ�nMinusBytes�ֽڶ���
						if ((m_FileDnloadInfo.dwDnloadedSize-=nMinusBytes)<0)
						{
							nDnloadMode = 1;
							m_FileDnloadInfo.dwDnloadedSize=0;
						}
						else
						{
							nDnloadMode = 2;
							bMinusBytesFlag=TRUE;//��ǣ������ļ�ʱǰbMinusBytes���ֽڶ���
						}
						sprintf(cmdbuf,"rest %d\r\n",m_FileDnloadInfo.dwDnloadedSize);//�������ؿ�ʼ��ƫ����
						SendData(cmdbuf,strlen(cmdbuf));	
						TRACE(L"����ƫ����\r\n");
				}
				else
				{
					//��ȡ�ļ���С
					char tempbuf[128];
					memset(tempbuf,NULL,128);
					WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_strFileName,wcslen(m_strFileName)
						,tempbuf,wcslen(m_strFileName),NULL,NULL);
					strcpy(cmdbuf,"size ");
					strcat(cmdbuf,tempbuf);
					strcat(cmdbuf,"\r\n");
					SendData(cmdbuf,strlen(cmdbuf));
					TRACE(L"��ȡ�ļ���С\r\n");
				}
				if (fp!=NULL)
				{
					fclose(fp);
					fp=NULL;
				}
				break;
			}
		case 350:
			{
	
				//��ȡ�ļ���С
				char tempbuf[128];
				memset(tempbuf,NULL,128);
				WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_strFileName,wcslen(m_strFileName)
					,tempbuf,wcslen(m_strFileName),NULL,NULL);
				strcpy(cmdbuf,"size ");
				strcat(cmdbuf,tempbuf);
				strcat(cmdbuf,"\r\n");
				SendData(cmdbuf,strlen(cmdbuf));
				TRACE(L"��ȡ�ļ���С\r\n");
				break;
			}
		case 213:
			strRecv.Delete(0,4);
			dwFileSize = _wtoi(strRecv.GetBuffer(0));
			m_FileDnloadInfo.dwFileSize =dwFileSize;
			char tempbuf[128];
			memset(tempbuf,NULL,128);
			WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_strFileName,wcslen(m_strFileName)
				,tempbuf,wcslen(m_strFileName),NULL,NULL);
			strcpy(cmdbuf, "retr "); 
			strcat(cmdbuf,tempbuf);
			strcat(cmdbuf,"\r\n");
			SendData(cmdbuf,strlen(cmdbuf));
			TRACE(L"ȡ���ļ�\r\n");
			break;
		case 150:bDownloading=TRUE;
		case 125:
			{	
				bDownloading=TRUE;
				if(nDnloadMode == 1)
				{
					fp = _wfopen(m_strDnloadDir + m_strFileName,L"wb+");
				}
				else if(nDnloadMode == 2)
				{
					fp = _wfopen(m_strDnloadDir + m_strFileName,L"ab+");	
				}

				if(!fp)
				{
					break;
				}

				if(m_strFileName.Compare(m_FileDnloadInfo.strFileName) == 0)
				{
					 fseek(fp,m_FileDnloadInfo.dwDnloadedSize,0);
				
				}
				fp_temp =_wfopen(m_strDnloadDir+m_strFileName+L".tmp",L"wb+"); 

				timeval times ;
				times.tv_sec   = 20 ;
				times.tv_usec  = 0 ;
				fd_set	readfd;
				FD_ZERO( &readfd );
				FD_SET(m_dataSocket , &readfd );

				int nCount =2000;
				char databuf[1050];
				int buflen=0;
				int nByteWritten = 0;//д���ļ����ֽ���
				TRACE(L"��ʼ���ݴ���\r\n");
				while(nCount--)
				{	
					static int nTimes=0;
					if ( select( 0 , &readfd , 0 , 0 , &times ) <=0 )  
					{	
						break;
					/*	closesocket(m_dataSocket);
						m_dataSocket= INVALID_SOCKET;
							TRACE(L"��ʱ\r\n");
						if (++nTimes>5)
						{
							nTimes=0;
							bDownloading=FALSE;
							bDnloadFail=TRUE;
							break;
						}
						TRACE(L"��������\r\n");
						
						Close();
						Sleep(50);
						Create(SOCK_STREAM);
						if (Connect(m_FTPServer,m_FTPPort))
						{
							char buf[128];
							char tempbuf[128];

							memset(buf,NULL,128);
							strcpy(buf,"user ");
							memset(tempbuf,NULL,128);
							WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_FTPUserName,wcslen(m_FTPUserName)
								,tempbuf,wcslen(m_FTPUserName),NULL,NULL);
							strcat(buf,tempbuf);
							strcat(buf,"\r\n");
							SendData(buf,strlen(buf));

							memset(buf,NULL,128);
							strcpy(buf,"pass ");
							memset(tempbuf,NULL,128);
							WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_FTPPassword,wcslen(m_FTPPassword)
								,tempbuf,wcslen(m_FTPPassword),NULL,NULL);
							strcat(buf,tempbuf);
							strcat(buf,"\r\n");
							SendData(buf,strlen(buf));

							bDownloading=TRUE;

							break;
						}*/
		
					
					}
					nTimes=0;
					memset(databuf,NULL,1024);
					buflen = recv(m_dataSocket,databuf,	1024,0);
					if(buflen == SOCKET_ERROR)
					{
						break;
					}

					if(buflen == 0) //�����������ݶ���
					{
						if(m_FileDnloadInfo.dwDnloadedSize == m_FileDnloadInfo.dwFileSize)
						{

							m_FileDnloadInfo.dwDnloadedSize = 0;
							m_FileDnloadInfo.dwFileSize = 0;
							m_FileDnloadInfo.strFileName.Empty();
						}
						break;
					}

					if(bMinusBytesFlag)
					{
						nByteWritten=fwrite(databuf+nMinusBytes,sizeof(char),buflen-nMinusBytes,fp);
						buflen-=nMinusBytes;
						m_FileDnloadInfo.dwDnloadedSize+=nMinusBytes;//���ؼ�ȥ���ֽ���
						bMinusBytesFlag=FALSE;

					}
					else
					{
						nByteWritten = fwrite(databuf,sizeof(char),buflen,fp);
					}

					if (nByteWritten != buflen)
					{
						TRACE(L"д���ֽ�������ȷ\r\n");
						break;
					}
					else
					{
						m_FileDnloadInfo.dwDnloadedSize += buflen;
						//���ļ��м�¼�Ѿ������ֽڸ���
						fseek(fp_temp,0,SEEK_SET);
						fwrite(&m_FileDnloadInfo.dwDnloadedSize,sizeof(m_FileDnloadInfo.dwDnloadedSize),1,fp_temp);
						fflush(fp);
						fflush(fp_temp);

						if (m_FileDnloadInfo.dwDnloadedSize >=m_FileDnloadInfo.dwFileSize) 
						{
							m_FileDnloadInfo.dwDnloadedSize = 0;
							m_FileDnloadInfo.dwFileSize = 0;
							m_FileDnloadInfo.strFileName.Empty();
							break;
						}
					}
				}
				fseek(fp_temp,0,SEEK_SET);
				fwrite(&m_FileDnloadInfo.dwDnloadedSize,sizeof(m_FileDnloadInfo.dwDnloadedSize),1,fp_temp);


				if (m_FileDnloadInfo.dwDnloadedSize >= m_FileDnloadInfo.dwFileSize)
				{
					closesocket(m_dataSocket);
					m_dataSocket= INVALID_SOCKET;
				}
				else
				{
					Sleep(10000);
					strcpy(cmdbuf, "abor "); 
					strcat(cmdbuf,tempbuf);
					strcat(cmdbuf,"\r\n");
					SendData(cmdbuf,strlen(cmdbuf));
					TRACE(L"\r\n");
					Sleep(5000);
				}
				if(fclose(fp) == EOF)
				{
					TRACE(L"�ر��ļ�����\r\n");
				}
				if (fclose(fp_temp) == EOF)
				{
					TRACE(L"�ر��ļ�����\r\n");
				}
				break;
			}
		case 426:
			{
				if (cmdbuf[0]=='a'&&cmdbuf[1]=='b'&&cmdbuf[2]=='o'&&cmdbuf[3]=='r')
				{
					strcpy(cmdbuf,"pasv\r\n");
					SendData(cmdbuf,strlen(cmdbuf));
					Sleep(500);
				}
			}
		case 226:
			if(m_FileDnloadInfo.dwDnloadedSize >= m_FileDnloadInfo.dwFileSize)
			{	
				nErrCount=0;
				DeleteFile(m_strDnloadDir+m_strFileName+L".tmp"); //�������,ɾ����ʱ�ļ�
				TRACE(L"�������\r\n");

				WIN32_FIND_DATAW wfd;
				HANDLE hFile = FindFirstFile(m_strDnloadDir+m_strFileName,&wfd);
		
				if (dwFileSize!=wfd.nFileSizeLow)
				{
					bDownloading=FALSE;
					bDnloadFail=TRUE;
					DeleteFile(m_strDnloadDir+m_strFileName); //����ʧ��,ɾ����ʱ�ļ�
					break;
				}
			
				if (m_FTPFiletype==L"ssi")
				{
					CString strSSIPath=(CString)g_WorkDir;
					strSSIPath+=(CString)g_SSIUpateName;
					DeleteFile(strSSIPath);
					Sleep(50);
					MoveFile(m_strDnloadDir+m_strFileName,strSSIPath);//�ƶ��ļ�������Ŀ¼
					bNewSSI_Flag=TRUE;
					bDnloadFail=FALSE;
					bDownloading=FALSE;
					TRACE(L"SSI����\r\n");
					break;
				}

				if (m_FTPFiletype==L"exe")
				{
					CString strExePath;
					strExePath=g_UpdateDir;
					strExePath=strExePath+m_strFileName;
					DeleteFile(strExePath);
					Sleep(50);
					MoveFile(m_strDnloadDir+m_strFileName,strExePath);	//�ƶ��ļ�������Ŀ¼
					bNewExe_Flag=TRUE;
					strDnldExeFileName=m_strFileName;
					bDnloadFail=FALSE;
					bDownloading=FALSE;
					TRACE(L"׼�����³���\r\n");
					break;
				}
				else if (m_FTPFiletype==L"cfg")
				{	
					CString strCfgFileName;
					strCfgFileName=g_DnloadSettingFile;
					CString strCfgFilePath;
					strCfgFilePath = g_WorkDir;
					strCfgFilePath=strCfgFilePath+strCfgFileName;
					DeleteFile(strCfgFilePath);
					Sleep(50);
					MoveFile(m_strDnloadDir+m_strFileName,strCfgFilePath);	//�ƶ��ļ�������Ŀ¼
					bNewConfig_Flag=TRUE;
					bDnloadFail=FALSE;
					bDownloading=FALSE;
					TRACE(L"׼����������\r\n");
					break;
				}
				else if (m_FTPFiletype==L"lst")//������ص����ļ��б����ȡ��Ҫ���ص��ļ���
				{
					m_FTPFiletype="";
					GetDnloadFileName();
					TRACE(L"��ȡ��Ҫ���ص��ļ���\r\n");
					//�����ļ�
					for(int i=0;i<m_FileList.nFileNum;++i)
					{
						MoveFile(m_strDnloadDir+m_FileList.strFileName[i],m_strDnloadDir+_T("backup\\")+m_FileList.strFileName[i]);	//�����ļ�	
						TRACE(L"�����ļ�\r\n");
					}
					if (m_FileList.nFileNum != 0)
					{
						m_strFileName = m_FileList.strFileName[0];
					}
					else
					{
						break;
					}
				}
				else
				{
					m_FileList.bDnloaded[nFileIndex++] = TRUE;
					if (nFileIndex == m_FileList.nFileNum)
					{
						bDownloading=FALSE;
						//nFileIndex=0;//��Ҫ���ص��ļ�������
						nDnloadMode = 1; //1 ȫ�����أ�2 �����ϴ�δ�������
						nErrCount = 0;//����һ���ļ��������
						m_strFileName=L"FTP"+m_FileList.strListFileName;
						break;//�������
					}
					m_strFileName = m_FileList.strFileName[nFileIndex];
				}
			}

			if (cmdbuf[0]=='a'&&cmdbuf[1]=='b'&&cmdbuf[2]=='o'&&cmdbuf[3]=='r')
			{
				strcpy(cmdbuf,"pasv\r\n");
				SendData(cmdbuf,strlen(cmdbuf));
				Sleep(500);
			}
			break;
		case 550:
			{
				bDnloadFail=TRUE;
				TRACE(L"����ʧ��\r\n");
				++nErrCount;
				if (nErrCount>5)//������δ���������һ���ļ�
				{
					dwFileSize=0;
					nFileIndex++;
					nErrCount=0;
					if (fp!=NULL)
					{
						fclose(fp);
						fp=NULL;
					}
					if (fp_temp!=NULL)
					{
						fclose(fp_temp);
						fp_temp=NULL;
					}
					MoveFile(m_strDnloadDir+_T("backup\\")+m_strFileName,m_strDnloadDir+m_strFileName);	//�ָ��ļ�	
					DeleteFile(m_strDnloadDir+m_strFileName+L".tmp");
					nFileIndex++;
					if (nFileIndex == m_FileList.nFileNum)
					{
						bDnloadFail=TRUE;
						bDownloading=FALSE;

					//	nFileIndex=0;//��Ҫ���ص��ļ�������
						nDnloadMode = 1; //1 ȫ�����أ�2 �����ϴ�δ�������
						nErrCount = 0;//����һ���ļ��������
						m_strFileName=L"FTP"+m_FileList.strListFileName;
						break;//�������,δ�������
					}
					m_strFileName = m_FileList.strFileName[nFileIndex];
				}
				strcpy(cmdbuf,"pasv\r\n");
				SendData(cmdbuf,strlen(cmdbuf));
				break;
			}
		default:
			break;
		}

		if (_wtoi(strCode.GetBuffer(0))==226 || _wtoi(strCode.GetBuffer(0))==550)
		{
			if (m_FileList.nFileNum==0&&m_FTPFiletype==L"")
			{
				bDownloading=FALSE;
				bDnloadFail=FALSE;
				TRACE(L"û����Ҫ���µ��ļ�\r\n");
				
			}
			else
			{
				if (nFileIndex == m_FileList.nFileNum&&m_FTPFiletype==L"")
				{
					nFileIndex=0;
					UpdateFileList();
					TRACE(L"ִ�и������\r\n");
					bDownloading=FALSE;
					InitFileList();
					nErrCount = 0;
					nDnloadMode = 1; 
					m_strFileName=L"FTP"+m_FileList.strListFileName;
					//DeleteFile(m_strDnloadDir+L"FileList.txt");
				}
			}
		}

		if (bDownloading==FALSE)
		{
			if (bDnloadFail)
			{
				CString strtmp;
				strtmp = _T("(Ftp:") +markID + _T(";1)");
				USES_CONVERSION;
				char* chtmp = T2A(strtmp); 
				AddTCP_Data((BYTE*)chtmp,strlen(chtmp),FTPstart,FTPend);
				bDnloadFail=FALSE;//�����Ǹ�λ
				CString strContent=L"����ʧ��";
#ifdef USING_LOG
				AddLog_Data(strContent,LogFTPstart,LogFTPend);
#endif
				return FALSE;
			}
			else
			{
				CString strtmp;
				strtmp = _T("(Ftp:") +markID + _T(";0)");
				USES_CONVERSION;
				char* chtmp = T2A(strtmp); 
				AddTCP_Data((BYTE*)chtmp,strlen(chtmp),FTPstart,FTPend);
				strCode+=L"���سɹ�\r\n";
				TRACE(strCode);
#ifdef USING_LOG
				CString strContent=L"���سɹ�";
				AddLog_Data(strContent,LogFTPstart,LogFTPend);
#endif
				return TRUE;
			}
			Disconnect();
			//Close();
		}
	}
	return TRUE;
}


SOCKET CCEFTP::createDataSocket(const char * mess) 
{ 
	char p1[5], p2[5]; 
	unsigned short port; 
	int p1s, p2s; 
	int r1;
	SOCKET datasocket; 
	struct sockaddr_in server; 

	memset(p1, NULL, 5); 
	memset(p2, NULL, 5); 
	p1s = 0; 
	p2s = 0; 
	for (int i = 0, r1 = 0; i < 200 && mess[i] != '\0'; i++) 
	{ 
		if (mess[i] == ',') { 
			r1++; 
			continue; 
		} 
		if (r1 == 4) { 
			p1[p1s] = mess[i]; 
			p1s++; 
			continue; 
		} 
		if (r1 == 5) { 
			p2[p2s] = mess[i]; 
			p2s++; 
			continue; 
		} 
	} 
	port = atoi(p1) * 256 + atoi(p2); 
	server.sin_family = AF_INET; 
	server.sin_port = htons(port); 

	char ansiRemoteHost[128];
	ZeroMemory(ansiRemoteHost,128);
	WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,m_FTPServer,wcslen(m_FTPServer)
		,ansiRemoteHost,wcslen(m_FTPServer),NULL,NULL);
	server.sin_addr.S_un.S_addr = inet_addr(ansiRemoteHost); 
	datasocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	/*TCP*/
	if (datasocket ==- 1) { 
		return - 1; 
	} 
	r1 = connect(datasocket, (struct sockaddr * ) & server, sizeof(server)); 


	if (r1 == SOCKET_ERROR) { 
		r1 = WSAGetLastError();
		closesocket(datasocket); 
		datasocket= INVALID_SOCKET;
		return - 1; 
	} 
	return datasocket; 
}



BOOL CCEFTP::GetDnloadFileName()//��ȡ��Ҫ���ص��ļ���
{
	CString strLine;
	CString strFileName;
	CString strVersionOld;
	CString strVersionNew;
	int k,index;

	char buf[512];
	memset(buf,NULL,512);

	FILE *fp = NULL;
	fp = _wfopen(m_strDnloadDir+m_FileList.strListFileName,L"r");
	if (fp !=NULL)//�����б��ļ�����
	{

		fclose(fp);
		//�����ص��б��ļ�
		fp = _wfopen(m_strDnloadDir+L"FTP"+m_FileList.strListFileName,L"r");
		m_FileList.nFileNum=0;
		while(fgets(buf,512,fp) != NULL)
		{
			strLine = buf;
			index =strLine.Find(_T("FileName"));
			if(index!=-1)//���ļ���
			{
				index=strLine.Find('=');
				if(index!=-1)
				{
					strFileName=strLine.Mid(index+1,strLine.Find(';')-index-1);

					index=strLine.Find(_T("Version"));
					if(index!=-1)//�Ұ汾��
					{
						index=strLine.Find(_T("=V"));
						strVersionNew=strLine.Mid(index+2,strLine.ReverseFind(';')-index-2);
						if(!strVersionNew.IsEmpty())
						{
							strVersionOld= SearchFileVersion(strFileName,strVersionNew,m_strDnloadDir,m_FileList.strListFileName);
							k=CompareVersion(strVersionNew,strVersionOld);
							if(k==1)
							{
								m_FileList.strFileName[m_FileList.nFileNum] = strFileName;
								m_FileList.strVersion[m_FileList.nFileNum] = strVersionNew;
								m_FileList.bDnloaded[m_FileList.nFileNum++] = FALSE;
							}
						}
					}
				}

			}
		}
		fclose( fp );
	}
	else
	{
		//MoveFile(m_strDnloadDir+L"FTP"+m_FileList.strListFileName,m_strDnloadDir+m_FileList.strListFileName);
		fp = _wfopen(m_strDnloadDir+L"FTP"+m_FileList.strListFileName,L"r");
		while(fgets(buf,512,fp) != NULL)
		{
			strLine = buf;
			index =strLine.Find(_T("FileName"));
			if(index!=-1)//���ļ���
			{
				index=strLine.Find('=');
				if(index!=-1)
				{
					m_FileList.strFileName[m_FileList.nFileNum]=strLine.Mid(index+1,strLine.Find(';')-index-1);
					index=strLine.Find(_T("=V"));
					m_FileList.strVersion[m_FileList.nFileNum]=strLine.Mid(index+2,strLine.ReverseFind(';')-index-2);
					m_FileList.bDnloaded[m_FileList.nFileNum++] = FALSE;
				}
			}
		}
		fclose(fp);
	}
	return TRUE;
}


void CCEFTP::InitFileList()
{
	m_FileList.strListFileName=L"FileList.txt";
	m_FileList.nFileNum=0;
}


void CCEFTP::Close()
{
	Disconnect();
	int watchDog = 0;
	while (m_readThread)
	{
		SetThreadPriority(m_readThread, THREAD_PRIORITY_HIGHEST);
		if((::WaitForSingleObject(m_readThread, 1000) != WAIT_TIMEOUT) ||
			watchDog >= THREAD_TERMINATION_MAXWAIT)
		{
			CloseHandle(m_readThread);
			m_readThread = NULL;
		}
		watchDog++;
	}
	watchDog = 0;
	while (m_acceptThread)
	{
		SetThreadPriority(m_acceptThread, THREAD_PRIORITY_HIGHEST);
		if((::WaitForSingleObject(m_acceptThread, 1000) != WAIT_TIMEOUT) ||
			watchDog >= THREAD_TERMINATION_MAXWAIT)
		{
			CloseHandle(m_acceptThread);
			m_acceptThread = NULL;
		}
		watchDog++;
	}
	watchDog = 0;
	while (m_writeThread)
	{
		SetThreadPriority(m_writeThread, THREAD_PRIORITY_HIGHEST);
		if((::WaitForSingleObject(m_writeThread, 1000) != WAIT_TIMEOUT) ||
			watchDog >= THREAD_TERMINATION_MAXWAIT)
		{
			CloseHandle(m_writeThread);
			m_writeThread = NULL;
		}
		watchDog++;
	}
	m_ReadBuffer.ClearData();
}


int CCEFTP::UpdateFileList()
{
	FILE *fp=NULL;
	if ((fp=_wfopen(m_strDnloadDir+m_FileList.strListFileName,L"r"))!=NULL)
	{	
		char buf[512];
		memset(buf,NULL,512);
		CString strConment;
		while(fgets(buf,512,fp) != NULL)
		{
			strConment = (CString)buf;
			int nIndex=strConment.Find(L"FileName");
			if (nIndex!=-1)
			{
				nIndex=strConment.Find('=');
				strConment=strConment.Mid(nIndex+1,strConment.Find(';')-nIndex-1);
				BOOL bFlag = FALSE;
				for (nIndex=0;nIndex<m_FileList.nFileNum;++nIndex)
				{
					if (m_FileList.strFileName[nIndex].Compare(strConment)==0)
					{
						bFlag=TRUE;break;		
					}
				}
				if (bFlag==FALSE)
				{
					m_FileList.strFileName[m_FileList.nFileNum]=strConment;
					strConment = (CString)buf;
					nIndex=strConment.Find(_T("=V"));
					m_FileList.strVersion[m_FileList.nFileNum]=strConment.Mid(nIndex+2,strConment.ReverseFind(';')-nIndex-2);
					m_FileList.bDnloaded[m_FileList.nFileNum] = TRUE;
					m_FileList.nFileNum++;
				}
			}
		}
		fclose(fp);
	}

	if ((fp=_wfopen(m_strDnloadDir+m_FileList.strListFileName,L"wb+"))!=NULL)
	{
		char buf[612];
		for (int nIndex=0;nIndex<m_FileList.nFileNum;++nIndex)
		{
			if (m_FileList.bDnloaded[nIndex]==TRUE)
			{
				CString strConment=L"FileName=";
				strConment+=m_FileList.strFileName[nIndex];
				strConment+=L";Version=V";
				strConment+=m_FileList.strVersion[nIndex];
				strConment+=L";\r\n";
				CString2Char(strConment,buf,sizeof(buf));
				fputs(buf,fp);
			}
		}
	}
	fclose(fp);
	return 0;
}


//ɾ����ʱ�ļ�
BOOL CCEFTP::DeleteTempdFile(CString strPath ,UINT nDates)
{
	WIN32_FIND_DATAW wfd;

	//CString strPath =L"\\NAND\\*.tmp";//����tmp�ļ���

	CString strFileName=L"\\NAND\\FTPData\\";
	HANDLE hFile = FindFirstFile(strPath,&wfd);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	do{
		DeleteFile(strFileName+wfd.cFileName);
	}while(FindNextFile(hFile,&wfd));
	return TRUE;
}


