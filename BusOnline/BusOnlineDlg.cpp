// BusOnlineDlg.cpp : ʵ���ļ�
//
#include "Winsock2.h"

#include "stdafx.h"
#include "BusOnline.h"
#include "BusOnlineDlg.h"
#include "MyRAS.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//Soft reset the device
#include  "winioctl.h"
#define IOCTL_HAL_REBOOT CTL_CODE(FILE_DEVICE_HAL, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
extern "C" __declspec(dllimport)void SetCleanRebootFlag(void);
extern "C" __declspec(dllimport) BOOL KernelIoControl(
	DWORD dwIoControlCode,
	LPVOID lpInBuf,
	DWORD nInBufSize,
	LPVOID lpOutBuf,
	DWORD nOutBufSize,
	LPDWORD lpBytesReturned);



BOOL ResetDevice()
{
	return KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
}


BOOL HardResetDevice()
{
	SetCleanRebootFlag();
	return KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
}
// CBusOnlineDlg �Ի���

CBusOnlineDlg::CBusOnlineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBusOnlineDlg::IDD, pParent)
	, m_DebugInf(_T(""))
	, m_BusCode(_T(""))
	, m_VerNo(_T(""))
	, m_ServerIP(_T(""))
	, m_dwPort(0)
	, m_LocalIP(_T(""))
	, m_GPSRecvBytes(0)
	, m_GPSSendBytes(0)
	, m_CANRecvBytes(0)
	, m_CANSendBytes(0)
	, m_SSIRecvBytes(0)
	, m_SSISendBytes(0)
	, m_strTCPSend(_T(""))
	, m_FanSendBytes(0)
	, m_FanRecvBytes(0)
	, m_GasSendBytes(0)
	, m_GasRecvBytes(0)
	, m_dwMemUsed(0)
	, m_dwMemTotal(0)
	, m_DBSendBytes(0)
	, m_DBRecvBytes(0)
	, m_TCPSendBytes(0)
	, m_TCPRecvBytes(0)
	, m_BootTime(_T(""))
	, m_RingData(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBusOnlineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_DebugInf);
	DDX_Text(pDX, IDC_BUSCODE, m_BusCode);
	DDX_Text(pDX, IDC_VERNO, m_VerNo);
	DDX_Text(pDX, IDC_SERVERIP, m_ServerIP);
	DDX_Text(pDX, IDC_PORT, m_dwPort);
	DDX_Text(pDX, IDC_LOCALIP, m_LocalIP);
	DDX_Text(pDX, IDC_GPSRECVBYTES, m_GPSRecvBytes);
	DDX_Text(pDX, IDC_GPSSENDBYTES, m_GPSSendBytes);
	DDX_Text(pDX, IDC_CANRECVBYTES, m_CANRecvBytes);
	DDX_Text(pDX, IDC_CANSENDBYTES, m_CANSendBytes);
	DDX_Text(pDX, IDC_SSIRECVBYTES, m_SSIRecvBytes);
	DDX_Text(pDX, IDC_SSISENDBYTES, m_SSISendBytes);
	DDX_Text(pDX, IDC_TCPSEND, m_strTCPSend);
	DDX_Text(pDX, IDC_FANSENDBYTES, m_FanSendBytes);
	DDX_Text(pDX, IDC_FANRECVBYTES, m_FanRecvBytes);
	DDX_Text(pDX, IDC_GASSENDBYTES, m_GasSendBytes);
	DDX_Text(pDX, IDC_GASRECVBYTES, m_GasRecvBytes);
	DDX_Text(pDX, IDC_MEMUSED, m_dwMemUsed);
	DDX_Text(pDX, IDC_MEMTOTAL, m_dwMemTotal);
	DDX_Text(pDX, IDC_DBSENDBYTES, m_DBSendBytes);
	DDX_Text(pDX, IDC_DBRECVBYTES, m_DBRecvBytes);
	DDX_Text(pDX, IDC_TCPSENDBYTES, m_TCPSendBytes);
	DDX_Text(pDX, IDC_TCPRECVBYTES, m_TCPRecvBytes);
	DDX_Text(pDX, IDC_BOOTTIME, m_BootTime);
	DDX_Text(pDX, IDC_RINGDATA, m_RingData);
}

BEGIN_MESSAGE_MAP(CBusOnlineDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_UPDATESSI, &CBusOnlineDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_GENATERING, &CBusOnlineDlg::OnBnClickedGenatering)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CLOSE, &CBusOnlineDlg::OnBnClickedClose)
END_MESSAGE_MAP()

BOOL CBusOnlineDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetCursorPos( 1024, 768 );
	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	ShowWindow(SW_SHOWMAXIMIZED);


	FILE *fp=NULL;
	if (fopen_s(&fp,"\\NAND\\BUSONLINE.txt","a+")==0)
	{
		char buf[100];
		SYSTEMTIME st; 
		GetLocalTime(&st); 
		sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		fwrite(buf,sizeof(char),strlen(buf),fp);

		sprintf_s(buf,100,"��������\r\n");
		fwrite(buf,sizeof(char),strlen(buf),fp);
		fflush(fp);
		fclose(fp);
	}


	SYSTEMTIME st; 
	GetLocalTime(&st); 
	m_BootTime.Format(L"%04d-%02d-%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);

	for (int i=0;i<MAX_TCP_PACKET;i++)
	{
		TCP_Packet[i].bNeedDeal=false;
		TCP_Packet[i].bUsing=false;
		TCP_Packet[i].isize=0;
	}
	for (int i=0;i<MAX_TCP_PACKET;i++)
	{
		m_TCP.AddTCPData(&TCP_Packet[i]);
	}
#ifdef USING_LOG
	for (int i=0;i<MAX_LOG_PACKET;++i)

	{
		Log_Packet[i].bNeedDeal=false;
		Log_Packet[i].bUsing=false;
		TCP_Packet[i].isize=0;
	}
	for (int i=0;i<MAX_LOG_PACKET;i++)
	{
		m_log.AddLogData(&Log_Packet[i]);
	}
	m_log.WriteLog(L"������������");
#endif
	m_Update.UpdataFromSDCardCfg();

	GetConfig(); // ���ļ��л��������š�IP��ַ���˿ں�
	m_TCP.SetConfig(m_strBusCode,m_strHostIP,m_dwPort);

	m_TCP.GetBusInfo(&m_BusInfo);
	m_DBcomm.GetBusInfo(&m_BusInfo);

	m_BusCode = m_strBusCode;
	m_VerNo = L"(Ver:V3.1.1)";

	g_strBusCode=m_strBusCode;
	g_strCodeVersion=m_VerNo;

	m_ServerIP = m_strHostIP;

	int nTimer1= SetTimer(1,1100,NULL);//��STM8ͨ�� ʵ�ֿ��Ź�����
	int nTimer2= SetTimer(2,5000,NULL);//���ź�TCP����
	int nTimer3= SetTimer(3,15000,NULL);//��������
	int nTimer4= SetTimer(4,900,NULL);//ˢ�½���
	int nTimer5= SetTimer(5,11000,NULL);//���CAN������������������ͱ��
	int nTimer6= SetTimer(6,10100,NULL);//����ʱ���� ������������ÿ֡
										//���ݺ������10s��û���յ��ظ��������·�������
										//����ʱ�Զ����³���
	int nTimer7 = SetTimer(7,6000,NULL);//���¶�ʱ��
	int nTimer8 = SetTimer(8,5100,NULL);//��ʱ���͵�ǰ��·��Ϣ
	int nTimer9 = SetTimer(9,4500,NULL);//��վ������վ���ѯָ��
	int nTimer10 = SetTimer(10,4000,NULL);//���ӷ�����ѯ
	int nTimer11 =SetTimer(11,10000,NULL);//�򿪴���
	int nTime12 =SetTimer(12,20000,NULL);//����ȵ�������ѯ˾����Ϣ


	iHeartBeatCount=0;
	bDeviceReset=false;
	return TRUE;
}
int CBusOnlineDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}
void CBusOnlineDlg::OnTimer(UINT nIDEvent) 
{
	static int  nConnectCount=0;


	if (nIDEvent == 1)
	{
		DWORD dwBytesWritten;
		BYTE szSendBuf[] = "BusOnline";
		if(m_CANcomm.Comm_IsOpen())
		{
			if(m_CANcomm.Comm_Write(szSendBuf,9,&dwBytesWritten))
			{
				m_CANcomm.m_dwSendBytes+=9;
#ifdef USING_LOG
				static unsigned int Count=0;
				if (Count++>=60)
				{
					Count=0;
					AddLog_Data((BYTE*)szSendBuf,sizeof(szSendBuf),LogDLGstart,LogDLGend);
				}

				if (Count==30)
				{
					char szMemoryState[50];
					sprintf(szMemoryState,"memory %d of %d",m_dwMemUsed,m_dwMemTotal);
					AddLog_Data((BYTE*)szMemoryState,sizeof(szMemoryState),LogDLGstart,LogDLGend);
				}
#endif
				

			
			}
		}

		static int count=0;
		if (count%30==0)
		{
			FILE *fp=NULL;
			if (fopen_s(&fp,"\\NAND\\BUSONLINE.txt","a+")==0)
			{
				char buf[100];
				SYSTEMTIME st; 
				GetLocalTime(&st); 
				sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
				fwrite(buf,sizeof(char),strlen(buf),fp);
				memoryctrl.GetMemUsgesNt();    
				m_dwMemUsed=(memoryctrl.totalmem-memoryctrl.mem)/1024;   
				m_dwMemTotal=memoryctrl.totalmem/1024; 
				sprintf_s(buf,100,"count=%d COM of CAN OPEN: %d  Memory  %d of %d\r\n",count,m_CANcomm.Comm_IsOpen(),m_dwMemUsed,m_dwMemTotal);
				fwrite(buf,sizeof(char),strlen(buf),fp);
				fflush(fp);
				fclose(fp);
			}
		}
		count++;

	}else if(nIDEvent==2)
	{
		iHeartBeatCount++;
		if (iHeartBeatCount>20)//�������20û�������Ϸ�����������
		{

			char buf[]="�����Զ�����";
#ifdef USING_LOG
			m_DebugInf+=(CString)buf+_T("\r\n");
			AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			iHeartBeatCount=0;
			bDeviceReset=true;
			ResetDevice();
		}

#ifdef USING_3G
		if (!m_RasDial.isConnected())
		{
			m_RasDial.HangUp();
			if (!m_RasDial.Dial(L"3G",L"",L""))
			{
				m_RasDial.SetConnect();
				char buf[]="���ڲ���";
				m_DebugInf+=(CString)buf + _T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			}
			else
			{
				char buf[]="���ųɹ�";
				m_DebugInf+=(CString)buf + _T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			}
		}
		else
#endif
		{	
			if(!m_TCP.IsConnect())
			{
				bIsServerConnect = FALSE;
				char buf[]="�Ͽ�";
				m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
				
				if (m_TCP.LinkServer())
				{
					m_TCP.m_FTP.bDownloading=FALSE;//��������״̬
					char buf[]="��������";
					m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
					AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
				}
				else
				{
					char buf[]="����ʧ��";
					m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
					AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
				if (++nConnectCount>5)
					{
					  nConnectCount=0;
					  m_RasDial.SetConnect();
					}
				}
			}else
			{	//���ӳɹ�
				iHeartBeatCount=0;
				bDeviceReset=false;
				char heart_tmp[]="(Hrt:)";
				m_DebugInf+=L"(Hrt:) ";
				bIsServerConnect = TRUE;

				AddTCP_Data((BYTE*)heart_tmp,sizeof(heart_tmp),DLGstart,DLGend);
#ifdef USING_LOG
				static unsigned int Count=0;
				if (Count++>=10)
				{
					Count=0;
					AddLog_Data((BYTE*)heart_tmp,sizeof(heart_tmp),LogDLGstart,LogDLGend);
				}
#endif
			}
		}
	}else if (nIDEvent==3)
	{
		static int ntime=0;
		ntime++;

		static int requestTime=0;

		if (!m_BusInfo.bIsRecv)
		{
			if(m_TCP.IsConnect())
			{
				if (requestTime++<5)
				{
					//TRACE(L"//����������");
					BYTE bNewData[]={'(',0x7E,0x7E,0x49,0x04,0x00,0x00,0x4D,0x00,0x55,')'};
					if(AddTCP_Data(bNewData,sizeof(bNewData),DLGstart,DLGend))
					{

						char buf[]="�����������ӵ����Ͷ���";
						m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
						AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
					}
					else
					{
						char buf[]="�����������ӵ����Ͷ���ʧ��";
						m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
						AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
					}
				}
				
			}
			else
			{
				requestTime=0;
				char buf[]="������巢��ʧ��,�������Ӵ���";
				m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);	
#endif		
			}

		}

		if (m_TCP.IsConnect())
		{
			if (ntime%4 == 0)
			{
				m_BusInfo.Read4Send();
			}
		}
		//�Ա�ϵͳʱ��ȷ���Ƿ�Ҫ�����������

		if (ntime%4 == 2)
		{
			SYSTEMTIME TempTime;
			GetLocalTime(&TempTime);
			int iTemp1,iTemp2,iTemp3,iTemp4;
			iTemp1 = (int)TempTime.wDay;
			iTemp2 = (int)m_BusInfo.RingData[0].bDay;
			iTemp3 = (int)TempTime.wMonth;
			iTemp4 = (int)m_BusInfo.RingData[0].bMonth;
			if (iTemp2 !=iTemp1 || iTemp3 != iTemp4)
			{
				m_BusInfo.bIsRecv = FALSE;
				requestTime=0;
			}

			CString strRingData;
			m_RingData.Empty();
			for(int n = 0;n < 40;n++)
			{
				if (m_BusInfo.RingData[n].bValue)
				{
					strRingData.Format(L"\r\n%d-%d %d:%d ",m_BusInfo.RingData[n].bMonth,m_BusInfo.RingData[n].bDay,m_BusInfo.RingData[n].bHour,m_BusInfo.RingData[n].bMinute);
					m_RingData+=strRingData;
				}
			}
		}


		static RingStruct* sStruct = NULL;
		static bool bFirstRing = false;

		if(ntime%2 == 0)
		{
			if (sStruct==NULL || !sStruct->bRinging)
			{
				for(int i=0;i<40;i++)
				{
					if(m_BusInfo.RingData[i].bValue)
					{
						sStruct = &m_BusInfo.RingData[i];
						if(m_BusInfo.CompareTime(sStruct))
						{
							//TRACE(L"in the daling");
							sStruct->bRinging=TRUE;//��ʼ������������
							bFirstRing=true;

							char buf[100];
							sprintf(buf,"i= %d ��ʼ������������",i);
							m_DebugInf+=_T("\r\n")+(CString)buf+_T("\r\n");
#ifdef USING_LOG
							AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
							break;

						}
					}
				}
			}
		}
		if (sStruct!=NULL && sStruct->bRinging == TRUE && sStruct->bRinged == FALSE)
		{
			//AfxMessageBox(_T("Start Ring"));
			m_DBcomm.PlayServiceVoice(sStruct->nRingNum);
			char buf[50];
			sprintf(buf,"%d-%d %d:%d ���ڴ���",sStruct->bMonth,sStruct->bDay,sStruct->bHour,sStruct->bMinute);
			m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
			AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			if (bFirstRing)
			{
				BYTE bNewData[16];
				m_BusInfo.FormatRingTime(bNewData,1);

				if(m_BusInfo.Read4Record(sStruct->bMonth,sStruct->bDay,sStruct->bHour,sStruct->bMinute,1))
				{
					m_BusInfo.Write4Send(bNewData);
					m_BusInfo.Write4Record(sStruct->bMonth,sStruct->bDay,sStruct->bHour,sStruct->bMinute,1);

					char buf[]="��ʼ��������ӵ����Ͷ���";
					m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
					AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
				}
				bFirstRing=false;
			}

		}
	}
	else if (nIDEvent==4)
	{
		g_strBusCode=m_strBusCode;//���ڱ�վ������ʾ
		g_strCodeVersion=m_VerNo;

		m_strTCPSend=m_TCP.m_strSendComment;	//���͵�����
		m_LocalIP=m_TCP.GetLocalIP();//����IP

		m_GPSRecvBytes=m_GPScomm.m_dwRecvBytes;
		m_GPSSendBytes=m_GPScomm.m_dwSendBytes;

		m_CANRecvBytes=m_CANcomm.m_dwRecvBytes;
		m_CANSendBytes=m_CANcomm.m_dwSendBytes;

		m_SSIRecvBytes=m_SSIcomm.m_dwRecvBytes;
		m_SSISendBytes=m_SSIcomm.m_dwSendBytes;

		m_GasRecvBytes=m_GAScomm.m_dwRecvBytes;
		m_GasSendBytes=m_GAScomm.m_dwSendBytes;

		m_FanRecvBytes=m_FANcomm.m_dwRecvBytes;
		m_FanSendBytes=m_FANcomm.m_dwSendBytes;

		m_DBRecvBytes=m_DBcomm.m_dwRecvBytes;
		m_DBSendBytes=m_DBcomm.m_dwSendBytes;

		m_TCPRecvBytes=m_TCP.m_dwRecvBytes;
		m_TCPSendBytes=m_TCP.m_dwSendBytes;

		memoryctrl.GetMemUsgesNt();    
		m_dwMemUsed=(memoryctrl.totalmem-memoryctrl.mem)/1024;   
		m_dwMemTotal=memoryctrl.totalmem/1024; 

		m_DebugInf+=m_SSIcomm.strDebugInfo;
		m_SSIcomm.strDebugInfo.Empty();

		m_DebugInf+=m_DBcomm.m_strDebugInfo;
		m_DBcomm.m_strDebugInfo.Empty();

		m_DebugInf+=m_TCP.m_FTP.strDebugInfo;
		m_TCP.m_FTP.strDebugInfo.Empty();

		if(m_DebugInf.GetLength()>300)
		{
			m_DebugInf.Empty();
		}



	}
	else if(nIDEvent==5)
	{
		m_CANcomm.ResetCanSendState();
	}
	else if (nIDEvent ==6)//�������жϣ����¸���
	{
		if (m_SSIcomm.Comm_IsOpen())
		{
			if (m_SSIcomm.bUpdate==TRUE&&m_SSIcomm.bReply==FALSE)
			{

				if (m_SSIcomm.UpdateState==CSSIcomm::CHECH)
				{
					m_DebugInf+=L"\r\n�����Զ�����������ȵ�����\r\n";
			
				}
				else if(m_SSIcomm.UpdateState==CSSIcomm::SEND)
				{
					m_DebugInf+=L"\r\nǿ�����¸��³���\r\n";
				}
				m_SSIcomm.ResetUpdateState();
				m_SSIcomm.UpdateSSISoftWare();
				m_TCP.m_FTP.bNewSSI_Flag=FALSE;
			}
			else if(m_SSIcomm.bUpdate==TRUE&&m_SSIcomm.bReply==TRUE)
			{
				m_DebugInf+=L"��������\r\n";
			}
			m_SSIcomm.bReply=FALSE;//��0��bReply��ֵ�ں���	m_SSIcomm.UpdateSSISoftWare();����1;
		}
		
	}
	else if (nIDEvent == 7)
	{
		if (m_TCP.m_FTP.bResetDevice)//FTP�������ӷ�����ʧ�ܣ�����
		{
			bDeviceReset=true;
			//ResetDevice();
		}
		//���³���ʱ��
		if (m_TCP.m_FTP.bNewExe_Flag)//���ص����������
		{
			char buf[]="\r\n���³��ص��Գ���";
			m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
			AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			DownloadExeFile();

			bDeviceReset=true;
			ResetDevice();

			m_TCP.m_FTP.bNewExe_Flag=FALSE;
		}

		if (m_TCP.m_FTP.bNewSSI_Flag&&m_SSIcomm.Comm_IsOpen())//����ȵ������������
		{

			char buf[]="\r\n����ȵ���������";
			m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
			AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			m_SSIcomm.ResetUpdateState();
			m_SSIcomm.UpdateSSISoftWare();
			m_TCP.m_FTP.bNewSSI_Flag=FALSE;
		}
		
		if (m_TCP.m_FTP.bNewConfig_Flag)//���ص��������ļ�����
		{
			CString	strTmpBusCode		= m_strBusCode;
			CString	strTmpHostIP		= m_strHostIP;
			DWORD	dwTmpHostPort		= m_dwPort;
			int nTmpEnterDis			= g_nEnterDis;
			int nTmpLeaveDis			= g_nLeaveDis;
			
			m_TCP.m_FTP.bNewConfig_Flag=FALSE;
			if (GetConfig(0))
			{
				if( m_strBusCode != g_strBusCode || m_strHostIP!=g_strHostIP || m_dwPort!=g_nHostPort )//��������ʹ���²������ӣ���ʱ���ͳ��ƺ���
				{
					m_TCP.SetConfig(m_strBusCode,m_strHostIP,m_dwPort);
					m_TCP.LinkServer();
				}
				CString strCurPath(g_WorkDir);
				CString strNewPath(g_WorkDir);
				strNewPath= strNewPath+   _T("dnloadconfig.txt");;
				strCurPath = strCurPath + _T("CONFIG.txt");
				WIN32_FIND_DATA fdCur;
				HANDLE hCurFind = ::FindFirstFile( strCurPath, &fdCur );
				if ( hCurFind != INVALID_HANDLE_VALUE )
				{
					int rc = DeleteFile( strCurPath );
					if( !rc )
						rc = GetLastError( );
					::FindClose(hCurFind);
				}

				//�������ƶ�����ǰִ���ļ�·��
				CFile::Rename( strNewPath, strCurPath);

				char buf[]="\r\n���������ļ�";
				m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif

			}	
		}
		
	}
	else if (nIDEvent == 8)//��ʱ���͵�ǰ��·��Ϣ
	{
		//(Way:��ǰ��·�ţ���վ����;��ǰվ����)
		for (int i=0;i<16;i++)
		{
			if (m_BusInfo.LineNO[i]==' ')
			{
				m_BusInfo.LineNO[i]=0x00;
			}
		}
		char lineInfo[30];
		sprintf(lineInfo,"(Way:%s;%02x;%02x)",
			m_BusInfo.LineNO,m_BusInfo.StationCount,m_BusInfo.StationCur);		
		AddTCP_Data((BYTE*)lineInfo,strlen(lineInfo),DLGstart,DLGend);
	}
	else if(nIDEvent==9)//��վ������վ���ѯָ��
	{
		if (m_DBcomm.Comm_IsOpen())
		{
			m_DBcomm.SendCheckCmd();
		}
	}
	else if(nIDEvent==10)
	{
		if (m_FANcomm.Comm_IsOpen())
		{
			static int Count=0;
			if (m_FANcomm.bRecvFlag[m_FANcomm.Index]==true||m_FANcomm.Index==0)//����ӵ��ظ�����ѯ��һ������
			{
				Count=0;
				if (m_FANcomm.Index==7)
				{
					m_FANcomm.Index=0;
					m_FANcomm.DataPolling();
				}
				else
				{
					m_FANcomm.Index++;
					m_FANcomm.DataPolling();
				}
			
			}
			else 
			{
				Count++;
				if (Count==5)//�����ʱû�н��յ��ظ�����ѯ��һ������
				{
					Count=0;
					m_FANcomm.Index++;
					if (m_FANcomm.Index==8)
					{
						m_FANcomm.Index=0;
					}
					m_FANcomm.DataPolling();
				}
			}
		}
	}
	else if(nIDEvent==11)
	{
		static int bComOpen=0;
		if (bComOpen==0)
		{
			OpenSerialPort();
			bComOpen=1;
		}
	
	}
#ifdef USING_SSI
	else if(nIDEvent==12)//��ѯ˾����Ϣ
	{
		if (!m_TCP.m_FTP.bNewSSI_Flag)
		{
			m_SSIcomm.QueryDriverInfo();
		}
	}
#endif


	UpdateData(FALSE);
}



//��ȡ����
BOOL CBusOnlineDlg::GetConfig(BOOL bRun)
{
	BOOL bFlag[3];//�Ƿ��������
	for (int i=0;i<3;++i)
	{
		bFlag[i]=FALSE;
	}

	FILE *fp=NULL;
	if (bRun==1)
	{
		fp = fopen("\\NAND\\config.txt","rb");
	}
	else 
	{
		fp = fopen("\\NAND\\dnloadconfig.txt","rb");
	}

	if (fp!=NULL)
	{
		char buf[1024];
		CString strTemp;

		while(fgets(buf,1024,fp)!=NULL)
		{
			strTemp = buf;
			if (strTemp.Find(_T("BusCode"))!=-1)
			{
				m_strBusCode = strTemp.Mid(strTemp.Find('=')+1,strTemp.Find(';')-strTemp.Find('=')-1);
				bFlag[0]=TRUE;
			}

			if (strTemp.Find(_T("HostIp"))!=-1)
			{
				m_strHostIP = strTemp.Mid(strTemp.Find('=')+1,strTemp.Find(';')-strTemp.Find('=')-1);
				bFlag[1]=TRUE;
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
				bFlag[2]=TRUE;
			}	
		}
		fclose(fp);
	}

	if (bFlag[0]==FALSE||bFlag[1]==FALSE||bFlag[2]==FALSE)
	{
		return FALSE;
	}
	else
	{
		m_BusCode=m_strBusCode;
		m_ServerIP=m_strHostIP;
		return TRUE;
	}


}

bool CBusOnlineDlg::DownloadExeFile( )
{
	CString strNewFileName = m_TCP.m_FTP.strDnldExeFileName;

	if( strNewFileName == _T("TDT.BAK") )
	{
		CString strNewPath;
		CString strExepath;
		strNewPath= g_UpdateDir;
		strExepath=g_UpdateDir;
		strExepath=strExepath+_T("TDT.exe");
		strNewPath = strNewPath + strNewFileName;
		WIN32_FIND_DATA fdNew;
		HANDLE hNewFind = ::FindFirstFile( strNewPath, &fdNew );
		if ( hNewFind != INVALID_HANDLE_VALUE )
		{

			//����
			CFile::Rename( strNewPath, strExepath);

			::FindClose(hNewFind);
			//������ʼ��ǽ��̣����˳���������ʹϵͳ����
			WIN32_FIND_DATA fdGUI;
			CString strGUIExefile;
			strGUIExefile = g_GUISampleFile;
			CString strGUIExePath;
			strGUIExePath = "\\Windows\\";
			strGUIExePath = strGUIExePath + strGUIExefile;
			HANDLE hGuiFind = ::FindFirstFile( strGUIExePath, &fdGUI );
			if ( hGuiFind != INVALID_HANDLE_VALUE )//�����ǽ���ִ���ļ�����������
			{
				SHELLEXECUTEINFO ShellInfo; 
				memset(&ShellInfo, 0, sizeof(ShellInfo)); 
				ShellInfo.cbSize = sizeof(ShellInfo);
				ShellInfo.hwnd = NULL; 
				ShellInfo.lpVerb = _T("open"); 
				ShellInfo.lpFile = strGUIExePath; // �˴�дִ���ļ��ľ���·��
				ShellInfo.nShow = SW_SHOWNORMAL; 
				ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 

				BOOL bResult = ShellExecuteEx(&ShellInfo);

				::FindClose(hGuiFind);

				//��������ɹ����˳�������
				if( bResult)
				{
					m_RasDial.HangUp();//�Ͽ���������
					exit( 0 );
				}
				
			} 
		}
	}

	return 0;
}


void CBusOnlineDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_SSIcomm.ResetUpdateState();
	m_DebugInf+=L"����ȵ���������\r\n";
	m_SSIcomm.UpdateSSISoftWare();
}

void CBusOnlineDlg::OnBnClickedGenatering()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������


	BYTE bMonth;
	BYTE bDay;
	BYTE bHour;
	BYTE bMinute;
	BYTE nRingNum;



	SYSTEMTIME sysTime;    
	GetLocalTime(&sysTime);

	bMonth = (BYTE)sysTime.wMonth;
	bDay = (BYTE)sysTime.wDay;
	bHour=(BYTE)sysTime.wHour;
	bMinute=(BYTE)sysTime.wMinute+3;
	nRingNum=8;

	for(int n = 0;n < 5;n++)
	{
		m_BusInfo.RingData[n].bMonth = bMonth;
		m_BusInfo.RingData[n].bDay = bDay;
		m_BusInfo.RingData[n].bHour = bHour;
		m_BusInfo.RingData[n].bMinute = bMinute;
		m_BusInfo.RingData[n].nRingNum =nRingNum;
		m_BusInfo.RingData[n].bValue = TRUE;
		bMinute+=10;
		if (bMinute>=60)
		{
			bMinute-=60;
			bHour+=1;
		}

		if (bHour>=24)
		{
			bHour-=24;
			bDay+=1;
		}
	}
	m_BusInfo.WriteToTXT();		
	m_BusInfo.bIsRecv = TRUE;
	//�ظ� �ɹ�
	BYTE bNewData[]={'(',0x7E,0x7E,0x49,0x00,0x00,0x00,0x49,0x00,0x55,')'};
	AddTCP_Data(bNewData,sizeof(bNewData),DLGstart,DLGend);

	CString strRingData;
	for(int n = 0;n < 5;n++)
	{
		strRingData.Format(L"\r\n%d-%d %d:%d ",m_BusInfo.RingData[n].bMonth,m_BusInfo.RingData[n].bDay,m_BusInfo.RingData[n].bHour,m_BusInfo.RingData[n].bMinute);
		m_DebugInf+=strRingData;
	}
}

void CBusOnlineDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
#ifdef USING_LOG
	char buf[]="��������ر�";
	m_log.WriteLog(L"��������ر�\r\n");
#endif

	CloseSysteemResource();
	CDialog::OnClose();
}


void CBusOnlineDlg::CloseSysteemResource()
{
	m_CANcomm.Close();
	m_DBcomm.Close();
	m_FANcomm.Close();
	m_GAScomm.Close();
	m_GPScomm.Close();
	m_SSIcomm.Close();
	m_RasDial.HangUp();
#ifdef USING_LOG 
	m_log.Close();
#endif
	m_TCP.Close();
}
void CBusOnlineDlg::OnBnClickedClose()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CloseSysteemResource();
#ifdef USING_LOG
	m_log.WriteLog(L"��������ر�");
#endif

	CDialog::OnOK();
}
void CBusOnlineDlg::OpenSerialPort()
{
	if (m_GPScomm.OpenGPSComm())
	{
		m_DebugInf+=L"GPS���ڴ򿪳ɹ�\r\n";
	}
	else
	{
		m_DebugInf+=L"GPS���ڴ�ʧ��\r\n";
	}

	if(m_CANcomm.OpenCANComm())
	{
		m_DebugInf+=L"CAN���ڴ򿪳ɹ�\r\n";
	}
	else
	{
		m_DebugInf+=L"CAN���ڴ�ʧ��\r\n";
	}

	if (m_SSIcomm.OpenSSIComm())
	{
		m_DebugInf+=L"����ȵ��������ڴ򿪳ɹ�\r\n";
	}
	else
	{
		m_DebugInf+=L"����ȵ��������ڴ�ʧ��\r\n";
	}

	if (m_DBcomm.OpenDBComm())
	{
		m_DebugInf+=L"��վ�����ڴ򿪳ɹ�\r\n";
	}
	else
	{
		m_DebugInf+=L"��վ�����ڴ�ʧ��\r\n";
	}

	if (m_FANcomm.OpenFANComm())
	{
		m_DebugInf+=L"���ӷ��ȴ��ڴ򿪳ɹ�\r\n";
	}
	else
	{
		m_DebugInf+=L"���ӷ��ȴ��ڴ�ʧ��\r\n";
	}

	if (m_GAScomm.OpenGASComm())
	{
		m_DebugInf+=L"©�����������ڴ򿪳ɹ�\r\n";
	}
	else
	{
		m_DebugInf+=L"©�����������ڴ�ʧ��\r\n";
	}
#ifdef USING_LOG
	AddLog_Data(m_DebugInf,LogDLGstart,LogDLGend);
#endif
}