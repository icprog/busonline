//////////////////////////////////////////////////////////////
#ifndef GLOBE_H
#define GLOBE_H

//#define USING_3G
//#define USING_LOG
#define USING_SSI //满意度调查器

//定义修改线路名称
#define WM_LINE_MODIFIED WM_USER + 104
//定义修改线路名称
#define WM_DISPATCh_LINE WM_USER + 110
//心跳包处理
#define WM_HEARTBEAT     WM_USER + 111

#define FILEDATAFRM_LEN		128

#define MAX_TCP_PACKET 120

#define GPSstart 0
#define GPSend 9
#define CANstart 10
#define CANend 29
#define TCPstart 30
#define TCPend 39
#define FTPstart 40
#define FTPend 49
#define DBstart 50
#define DBend 59
#define SSIstart 60
#define SSIend 69
#define FANstart 70
#define FANend 89
#define GASstart 90
#define GASend 99
#define DLGstart 100
#define DLGend 109

#ifdef USING_LOG

#define MAX_LOG_PACKET 50
#define LogGPSstart 0
#define LogGPSend 4
#define LogCANstart 5
#define LogCANend 9
#define LogTCPstart 10
#define LogTCPend 14
#define LogFTPstart 15
#define LogFTPend 19
#define LogDBstart 20
#define LogDBend 24
#define LogSSIstart 25
#define LogSSIend 29
#define LogFANstart 30
#define LogFANend 34
#define LogGASstart 35
#define LogGASend 39
#define LogDLGstart 40
#define LogDLGend 49

#endif

extern CRect g_ScreenRect;

extern char g_WorkDir[];	//工作目录
extern char g_CANDir[];		//CAN数据存放
extern char g_FTPDnloadDir[];	//FTP数据存放
extern char g_FTPDir[];     //FTP工作目录
extern char g_FTPListFile[];   //FTP保存的配置文件名称
extern char g_UpdateDir[];	//升级程序下载存放目录

extern CString	g_strBusCode;
extern CString g_strCodeVersion;
extern CString	g_strHostIP;
extern int		g_nHostPort;
extern char     g_EXEName[];
extern char    g_SDCardDir[];
extern char    g_SSIUpateName[];


extern char g_SettingFile[];
extern char g_XMLFile[];
extern char g_DnloadSettingFile[];

extern char g_GUISampleFile[];

extern char g_AdFile[];

extern void delay(UINT n);
extern void CalcCrc (unsigned char *pStr, unsigned short nStrLen, unsigned char nType, unsigned short *pRes, unsigned short nInitCrc);


extern CString RemoveSpace( CString Str );
extern void readLine( FILE* fp, char* string, int lenbuf = 256 );

extern CString TransformComma( CString ResStr );
extern CString RestoreComma( CString ResStr );

extern bool CheckFileExist( CString strFilePath );

extern unsigned short HexString2Short(CString strHex);
extern CString CStringToHexStr( CString strCString );
extern CString HexStrToCString( CString strHexStr );

extern unsigned short * AnsiStrToUnicodeStr( CString strAnsi, int* pStrUniByteLen );
extern char * UnicodeStrToAnsiStr( CString strUni, int* pStrByteLen );
extern int GetUnicodeStrByteLen( CString strUni );

extern float GetDistance( char chCurLongitudeType, float fCurLongitudeM, char chCurLatitudeType, float fCurLatitudeM, 
						 char chDesLongitudeType, float fDesLongitudeM, char chDesLatitudeType, float fDesLatitudeM );

extern bool GetSum( BYTE *dataBuf, int dataLen, BYTE *SumL, BYTE *SumH );
extern void  TfxSleep(int nWaitInMSecs);
extern void  XSleep(ULONG ulMilliSeconds);
extern void CString2Char(CString str, char ch[],int nSizeofch);
extern int CompareVersion(CString strVersionNew,CString strVersionOld);
extern CString SearchFileVersion(CString filename,CString strVersionNew, CString dir, CString listFile);
extern int UpdateInfo(CString Info,CString TagetFile);
extern int UpdateListFile(CString strFileName,CString strVersionOld,CString strVersionNew ,CString ListFile);
extern bool AddTCP_Data(BYTE* pSendData,int ilen,int iStart,int iEnd);/*iStart: Data_Packet[]起始坐标;
																	  iEnd: Data_Packet[]终止坐标;
																	  将 Data_Packet[]分为不同用处的段
																	 
																	  */

extern bool AddLog_Data(BYTE* pSendData,int ilen,int iStart,int iEnd);
extern bool AddLog_Data(CString strContent,int iStart,int iEnd);
//帮助文件名称
extern CString strHelpFile;

extern int g_nBroadcastShowTime;
extern int g_nForecastShowTime;
extern int g_nAdShowTime;
extern int g_nServiceShowTime;

extern int g_nAutoParaNum;
extern int g_nAssessorDataNum;
extern int g_nSendCmd485Num;

extern int g_nUpArrowLeft;
extern int g_nUpArrowTop;
extern int g_nDownArrowLeft;
extern int g_nDownArrowTop;

extern int g_nStringTop;

extern int g_nEnterDis;	//进站距离
extern int g_nLeaveDis;	//离站距离

extern CDialog *pMainDlg;


enum STATE_RECV
{
	NO_RECV,    //没有开始接收等待 等待帧开头
	START, //遇到帧头 开始接收
	RECVING,    //正在接收
	REVED      //接收完成
};

typedef struct
{
	bool  bUsing;   //是否正在填入数据
	bool  bNeedDeal;//需要处理
	int   isize;    //包大小以字节为单位
	BYTE  Data[256];
}Data_Packet;


extern Data_Packet TCP_Packet[];

#ifdef USING_LOG
extern Data_Packet Log_Packet[];
#endif

#endif
