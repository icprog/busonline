// BusOnlineDlg.cpp : 实现文件
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
// CBusOnlineDlg 对话框

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
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	ShowWindow(SW_SHOWMAXIMIZED);


	FILE *fp=NULL;
	if (fopen_s(&fp,"\\NAND\\BUSONLINE.txt","a+")==0)
	{
		char buf[100];
		SYSTEMTIME st; 
		GetLocalTime(&st); 
		sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		fwrite(buf,sizeof(char),strlen(buf),fp);

		sprintf_s(buf,100,"主机重启\r\n");
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
	m_log.WriteLog(L"主机程序重启");
#endif
	m_Update.UpdataFromSDCardCfg();

	GetConfig(); // 从文件中获得主机编号、IP地址、端口号
	m_TCP.SetConfig(m_strBusCode,m_strHostIP,m_dwPort);

	m_TCP.GetBusInfo(&m_BusInfo);
	m_DBcomm.GetBusInfo(&m_BusInfo);

	m_BusCode = m_strBusCode;
	m_VerNo = L"(Ver:V3.1.1)";

	g_strBusCode=m_strBusCode;
	g_strCodeVersion=m_VerNo;

	m_ServerIP = m_strHostIP;

	int nTimer1= SetTimer(1,1100,NULL);//与STM8通信 实现看门狗功能
	int nTimer2= SetTimer(2,5000,NULL);//拨号和TCP连接
	int nTimer3= SetTimer(3,15000,NULL);//发车提醒
	int nTimer4= SetTimer(4,900,NULL);//刷新界面
	int nTimer5= SetTimer(5,11000,NULL);//清除CAN总线数据向服务器发送标记
	int nTimer6= SetTimer(6,10100,NULL);//设置时间标记 车载主机发出每帧
										//数据后如果在10s内没有收到回复，则重新发送数据
										//开机时自动更新程序
	int nTimer7 = SetTimer(7,6000,NULL);//更新定时器
	int nTimer8 = SetTimer(8,5100,NULL);//定时发送当前线路信息
	int nTimer9 = SetTimer(9,4500,NULL);//向报站器发送站点查询指令
	int nTimer10 = SetTimer(10,4000,NULL);//电子风扇轮询
	int nTimer11 =SetTimer(11,10000,NULL);//打开串口
	int nTime12 =SetTimer(12,20000,NULL);//满意度调查器查询司机信息


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
		if (iHeartBeatCount>20)//如果连续20没有连接上服务器则重启
		{

			char buf[]="掉线自动重启";
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
				char buf[]="正在拨号";
				m_DebugInf+=(CString)buf + _T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			}
			else
			{
				char buf[]="拨号成功";
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
				char buf[]="断开";
				m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
				
				if (m_TCP.LinkServer())
				{
					m_TCP.m_FTP.bDownloading=FALSE;//重置下载状态
					char buf[]="正在连接";
					m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
					AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
				}
				else
				{
					char buf[]="连接失败";
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
			{	//连接成功
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
					//TRACE(L"//请求新数据");
					BYTE bNewData[]={'(',0x7E,0x7E,0x49,0x04,0x00,0x00,0x4D,0x00,0x55,')'};
					if(AddTCP_Data(bNewData,sizeof(bNewData),DLGstart,DLGend))
					{

						char buf[]="请求打铃已添加到发送队列";
						m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
						AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
					}
					else
					{
						char buf[]="请求打铃已添加到发送队列失败";
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
				char buf[]="请求打铃发送失败,网络连接错误";
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
		//对比系统时间确定是否要请求打铃数据

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
							sStruct->bRinging=TRUE;//开始发送振铃命令
							bFirstRing=true;

							char buf[100];
							sprintf(buf,"i= %d 开始发送振铃命令",i);
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
			sprintf(buf,"%d-%d %d:%d 正在打铃",sStruct->bMonth,sStruct->bDay,sStruct->bHour,sStruct->bMinute);
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

					char buf[]="开始打铃已添加到发送队列";
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
		g_strBusCode=m_strBusCode;//用于报站器上显示
		g_strCodeVersion=m_VerNo;

		m_strTCPSend=m_TCP.m_strSendComment;	//发送的数据
		m_LocalIP=m_TCP.GetLocalIP();//主机IP

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
	else if (nIDEvent ==6)//若更新中断，重新更新
	{
		if (m_SSIcomm.Comm_IsOpen())
		{
			if (m_SSIcomm.bUpdate==TRUE&&m_SSIcomm.bReply==FALSE)
			{

				if (m_SSIcomm.UpdateState==CSSIcomm::CHECH)
				{
					m_DebugInf+=L"\r\n开机自动检测更新满意度调查器\r\n";
			
				}
				else if(m_SSIcomm.UpdateState==CSSIcomm::SEND)
				{
					m_DebugInf+=L"\r\n强制重新更新程序\r\n";
				}
				m_SSIcomm.ResetUpdateState();
				m_SSIcomm.UpdateSSISoftWare();
				m_TCP.m_FTP.bNewSSI_Flag=FALSE;
			}
			else if(m_SSIcomm.bUpdate==TRUE&&m_SSIcomm.bReply==TRUE)
			{
				m_DebugInf+=L"正在升级\r\n";
			}
			m_SSIcomm.bReply=FALSE;//置0，bReply的值在函数	m_SSIcomm.UpdateSSISoftWare();中置1;
		}
		
	}
	else if (nIDEvent == 7)
	{
		if (m_TCP.m_FTP.bResetDevice)//FTP下载连接服务器失败，重启
		{
			bDeviceReset=true;
			//ResetDevice();
		}
		//更新程序定时器
		if (m_TCP.m_FTP.bNewExe_Flag)//车载电脑软件更新
		{
			char buf[]="\r\n更新车载电脑程序";
			m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
			AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			DownloadExeFile();

			bDeviceReset=true;
			ResetDevice();

			m_TCP.m_FTP.bNewExe_Flag=FALSE;
		}

		if (m_TCP.m_FTP.bNewSSI_Flag&&m_SSIcomm.Comm_IsOpen())//满意度调查器程序更新
		{

			char buf[]="\r\n满意度调查器升级";
			m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
			AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif
			m_SSIcomm.ResetUpdateState();
			m_SSIcomm.UpdateSSISoftWare();
			m_TCP.m_FTP.bNewSSI_Flag=FALSE;
		}
		
		if (m_TCP.m_FTP.bNewConfig_Flag)//车载电脑配置文件更新
		{
			CString	strTmpBusCode		= m_strBusCode;
			CString	strTmpHostIP		= m_strHostIP;
			DWORD	dwTmpHostPort		= m_dwPort;
			int nTmpEnterDis			= g_nEnterDis;
			int nTmpLeaveDis			= g_nLeaveDis;
			
			m_TCP.m_FTP.bNewConfig_Flag=FALSE;
			if (GetConfig(0))
			{
				if( m_strBusCode != g_strBusCode || m_strHostIP!=g_strHostIP || m_dwPort!=g_nHostPort )//网络重新使用新参数连接，此时发送车牌号码
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

				//将更新移动到当前执行文件路径
				CFile::Rename( strNewPath, strCurPath);

				char buf[]="\r\n更新配置文件";
				m_DebugInf+=(CString)buf+_T("\r\n");
#ifdef USING_LOG
				AddLog_Data((BYTE*)buf,sizeof(buf),LogDLGstart,LogDLGend);
#endif

			}	
		}
		
	}
	else if (nIDEvent == 8)//定时发送当前线路信息
	{
		//(Way:当前线路号；总站点数;当前站点编号)
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
	else if(nIDEvent==9)//向报站器发送站点查询指令
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
			if (m_FANcomm.bRecvFlag[m_FANcomm.Index]==true||m_FANcomm.Index==0)//如果接到回复，轮询下一个数据
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
				if (Count==5)//如果超时没有接收到回复，轮询下一个数据
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
	else if(nIDEvent==12)//查询司机信息
	{
		if (!m_TCP.m_FTP.bNewSSI_Flag)
		{
			m_SSIcomm.QueryDriverInfo();
		}
	}
#endif


	UpdateData(FALSE);
}



//获取配置
BOOL CBusOnlineDlg::GetConfig(BOOL bRun)
{
	BOOL bFlag[3];//是否读到配置
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

			//改名
			CFile::Rename( strNewPath, strExepath);

			::FindClose(hNewFind);
			//启动初始外壳进程，并退出本进程以使系统重启
			WIN32_FIND_DATA fdGUI;
			CString strGUIExefile;
			strGUIExefile = g_GUISampleFile;
			CString strGUIExePath;
			strGUIExePath = "\\Windows\\";
			strGUIExePath = strGUIExePath + strGUIExefile;
			HANDLE hGuiFind = ::FindFirstFile( strGUIExePath, &fdGUI );
			if ( hGuiFind != INVALID_HANDLE_VALUE )//如果外壳进程执行文件存在则运行
			{
				SHELLEXECUTEINFO ShellInfo; 
				memset(&ShellInfo, 0, sizeof(ShellInfo)); 
				ShellInfo.cbSize = sizeof(ShellInfo);
				ShellInfo.hwnd = NULL; 
				ShellInfo.lpVerb = _T("open"); 
				ShellInfo.lpFile = strGUIExePath; // 此处写执行文件的绝对路径
				ShellInfo.nShow = SW_SHOWNORMAL; 
				ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 

				BOOL bResult = ShellExecuteEx(&ShellInfo);

				::FindClose(hGuiFind);

				//如果启动成功则退出本进程
				if( bResult)
				{
					m_RasDial.HangUp();//断开所有连接
					exit( 0 );
				}
				
			} 
		}
	}

	return 0;
}


void CBusOnlineDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_SSIcomm.ResetUpdateState();
	m_DebugInf+=L"满意度调查器升级\r\n";
	m_SSIcomm.UpdateSSISoftWare();
}

void CBusOnlineDlg::OnBnClickedGenatering()
{
	// TODO: 在此添加控件通知处理程序代码


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
	//回复 成功
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
	// TODO: 在此添加消息处理程序代码和/或调用默认值
#ifdef USING_LOG
	char buf[]="主机程序关闭";
	m_log.WriteLog(L"主机程序关闭\r\n");
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
	// TODO: 在此添加控件通知处理程序代码
	CloseSysteemResource();
#ifdef USING_LOG
	m_log.WriteLog(L"主机程序关闭");
#endif

	CDialog::OnOK();
}
void CBusOnlineDlg::OpenSerialPort()
{
	if (m_GPScomm.OpenGPSComm())
	{
		m_DebugInf+=L"GPS串口打开成功\r\n";
	}
	else
	{
		m_DebugInf+=L"GPS串口打开失败\r\n";
	}

	if(m_CANcomm.OpenCANComm())
	{
		m_DebugInf+=L"CAN串口打开成功\r\n";
	}
	else
	{
		m_DebugInf+=L"CAN串口打开失败\r\n";
	}

	if (m_SSIcomm.OpenSSIComm())
	{
		m_DebugInf+=L"满意度调查器串口打开成功\r\n";
	}
	else
	{
		m_DebugInf+=L"满意度调查器串口打开失败\r\n";
	}

	if (m_DBcomm.OpenDBComm())
	{
		m_DebugInf+=L"报站器串口打开成功\r\n";
	}
	else
	{
		m_DebugInf+=L"报站器串口打开失败\r\n";
	}

	if (m_FANcomm.OpenFANComm())
	{
		m_DebugInf+=L"电子风扇串口打开成功\r\n";
	}
	else
	{
		m_DebugInf+=L"电子风扇串口打开失败\r\n";
	}

	if (m_GAScomm.OpenGASComm())
	{
		m_DebugInf+=L"漏气报警器串口打开成功\r\n";
	}
	else
	{
		m_DebugInf+=L"漏气报警器串口打开失败\r\n";
	}
#ifdef USING_LOG
	AddLog_Data(m_DebugInf,LogDLGstart,LogDLGend);
#endif
}