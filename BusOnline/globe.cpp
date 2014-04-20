#include "stdafx.h"
#include "time.h"
#include <stdio.h> 

#include "globe.h"


//#define MODE1//新协力
//#define MODE2//昆明
//#define MODE3//一汽
//当前版本
#ifdef MODE3//一汽
//char CodeVersion[] = "(Ver:V3.0.2)";
#endif

#ifdef MODE2//昆明
//char CodeVersion[] = "(Ver:V1.5.1)";
#endif

#ifdef MODE1//新协力
//char CodeVersion[] = "(Ver:V2.0.9)";
//char CodeVersion[] = "(Ver:V2.1.0)";//电子风扇测试用
#endif

//显示屏幕分辨率
CRect g_ScreenRect( 0, 0, 1024, 768 );

//工作目录
char g_WorkDir[] = "\\NAND\\";				//工作目录
char g_CANDir[] = "\\NAND\\CANData\\";		//CAN数据存放
char g_FTPDnloadDir[]= "\\NAND\\FTPData\\";		//FTP数据存放
char g_FTPDir[]="/BusOnlineCfg";     //FTP工作目录
char g_FTPListFile[]="FileList.txt";    //FTP保存的配置文件名称
char g_UpdateDir[] = "\\NAND\\Update\\";	//升级程序下载存放目录

char g_SettingFile[]		= "config.txt";					//站点配置文件名称
char g_XMLFile[]		    = "Data.xml";					//保存发送和接收信息
char g_DnloadSettingFile[]	= "dnloadconfig.txt";			//网络下载的站点配置文件名称

char g_GUISampleFile[]	= "GUIsample.exe";			//开机启动的程序名称
char g_EXEName[]="TDT.exe";
char g_SSIUpateName[]="TDPS.fw1";//满意度调查器的升级文件
//char g_SSIUpateName[]="Setup_WebClient.exe";//测试文件
char g_SDCardDir[]="\\Storage Card\\busonline\\";




CString	g_strBusCode = _T("(Lic:粤A-1234)");
CString g_strCodeVersion = _T("(Ver:V2.1.4)");
//CString	g_strHostIP	 = _T("114.215.30.169");
//int		g_nHostPort	 = 5000;

//CString	g_strHostIP	 = _T("114.215.30.169");
//int		g_nHostPort	 = 5000;
CString	g_strHostIP	 = _T("120.85.129.116");
//CString	g_strHostIP	 = _T("200.200.1.239");
int		g_nHostPort	 = 6000;

bool g_bShowAD = 0;				//车载电脑是否播放广告 1-播放 0-不播放
bool g_bShowLineInfo = 0;		//车载电脑是否显示报站和预报站文字 1-显示 0-不显示


int g_nBroadcastShowTime = 20;
int g_nForecastShowTime = 20;
int g_nAdShowTime = 20;
int g_nServiceShowTime = 20;

//最大接收的车辆参数包个数，超过这个个数将丢弃第一个数据包
int g_nAutoParaNum = 200;

int g_nAssessorDataNum = 500;

int g_nSendCmd485Num = 50;

//帮助文件名称,根据实际需要进行修改,也可为word或其他类型文件
CString strHelpFile = L"help.htm";

int g_nUpArrowLeft		= 50;
int g_nUpArrowTop		= 520;
int g_nDownArrowLeft	= 900;
int g_nDownArrowTop		= 520;

int g_nStringTop		=290;

int g_nEnterDis = 10;	//进站距离
int g_nLeaveDis = 20;	//离站距离

CDialog *pMainDlg = NULL;

//计算CRC校验值
//pStr：需要计算校验值的串
//nStrLen：需要计算校验值的串的长度，字节数
//nType：CRC类型
//nType=0:  CCITT_POLY   X16 + X12 + X5 +1                      
//nType=1:  CRC16_POLY   X16 + X15 + X2 +1                      
//nType=2:  POLY_NR_2    X16 + X15 + X10 + X6 + X5 + 1          
//nType=3:  POLY_NR_3    X16 + X15 + X13 + X7 + X4 + X2 + X + 1 
//pRes：需要计算校验值的串，返回的CRC校验值
//nInitCrc：CRC校验值初始值，一般=0
void CalcCrc (unsigned char   *pStr,
                   unsigned short   nStrLen,
                   unsigned char    nType,
                   unsigned short  *pRes,
                   unsigned short   nInitCrc)
{
    unsigned short c;
    unsigned short b;
    unsigned short shift_reg;
    BOOLEAN        carry_shifted_out;

    static const unsigned short polynomial[4] = { 0x8480, 
                                                  0xa001, 
                                                  0x8621,
                                                  0xe950 };
    if (nType    > 3) nType = 0;
    if (nStrLen < 1) return;
   
    shift_reg = nInitCrc;
    for (c = 0; c < nStrLen; c++)
    {
        shift_reg = (unsigned short)(shift_reg ^ pStr[c]);
        for (b = 0; b < 8; b++)
        {
            carry_shifted_out = (BOOLEAN)( (shift_reg & 0x00000001) == 0x00000001 );
            shift_reg = (unsigned short)(shift_reg >> 1);
            if (carry_shifted_out)                
            {
                shift_reg = (unsigned short)(shift_reg ^ polynomial[nType]);
            }
        }                                                     
    }   
                                                         
    *pRes = shift_reg;
}

#include <afx.h>



bool CheckFileExist( CString strFilePath )
{
	WIN32_FIND_DATA m_Nextdata;
	HANDLE m_hFileHandle = FindFirstFile( strFilePath, &m_Nextdata );

	if ( m_hFileHandle == INVALID_HANDLE_VALUE )
		return false;

	return true;
}

unsigned short HexString2Short( CString strHex)
{
	TCHAR cTmp;
	unsigned short usRtn = 0;
	if(strHex == "") 
		return 0;
	for(int i = 0; i < 4 && i < strHex.GetLength(); i++)
	{
		cTmp = strHex[i];
		if(cTmp >47 && cTmp < 58)		//0-9
			cTmp -= 48;
		else if(cTmp >64 && cTmp < 71)	//A-F
			cTmp -= 55;
		else if(cTmp >96 && cTmp < 103)	//A-F
			cTmp -= 87;
		else 
			cTmp = 0;

		usRtn <<= 4;
		usRtn = usRtn + cTmp;
	}
	return usRtn;
}

CString CStringToHexStr( CString strCString )
{
	CString strRlt = _T("");

	for( int i=0; i<strCString.GetLength(); i++)
	{
		TCHAR tmpCh = strCString[i];

		CString strTmp;
		strTmp.Format( _T("%04x"), tmpCh );

		strRlt = strRlt + strTmp;
	}

	return strRlt;
}

CString HexStrToCString( CString strHexStr )
{
	CString strRlt = _T("");

	int num = strHexStr.GetLength()/4;
	for( int i=0; i<num; i++)
	{
		CString strTmp = strHexStr.Left( 4 );
		strHexStr = strHexStr.Right( strHexStr.GetLength() - 4 );

		TCHAR tmpCh = HexString2Short( strTmp );
		strRlt = strRlt + tmpCh;
	}

	return strRlt;
}

unsigned short * AnsiStrToUnicodeStr( CString strAnsi, int* pStrUniByteLen ) 
{
	int nAnsiLen = strAnsi.GetLength();
	char  *pAnsiData = new char[nAnsiLen];
	memcpy( pAnsiData, strAnsi, nAnsiLen );

	int nUniLen = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pAnsiData, nAnsiLen, NULL, 0 );
	unsigned short  *pUniData = new unsigned short[nUniLen];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pAnsiData, nAnsiLen, (LPWSTR)pUniData, nUniLen );

	delete pAnsiData;

	*pStrUniByteLen = sizeof(unsigned short)*nUniLen;

	return pUniData;
}

char * UnicodeStrToAnsiStr( CString strUni, int* pStrByteLen ) 
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

/*
 * 函数名: CString2Char
 * 参数1: CString str					待转换字符串
 * 参数2: char ch[]						转换后将要储存的位置
 * 将Unicode下的CString转换为char*
 */
void CString2Char(CString str, char ch[],int nSizeOfch)
{
    int i;
    char *tmpch;
    int wLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//得到Char的长度
    tmpch = new char[wLen + 1];        //分配变量的地址大小
	if (wLen+1>nSizeOfch)
	{
		AfxMessageBox(_T("Err in CString2Char"));
		return;
	}
    WideCharToMultiByte(CP_ACP, 0, str, -1, tmpch, wLen, NULL, NULL);       //将CString转换成char*
	for(i = 0; tmpch[i] != '\0'; i++) ch[i] = tmpch[i];
    ch[i] = '\0';

	delete tmpch;
}


//比较两个文件的版本号
int CompareVersion(CString strVersionNew,CString strVersionOld)
{
	if(strVersionNew.Compare(strVersionOld)==0) return 0;

	char str1[10],str2[10];
	while(strVersionNew.IsEmpty()==false||strVersionOld.IsEmpty()==false)
	{
		int n=strVersionNew.Find('.');
		if(n>0)
			CString2Char(strVersionNew.Left(n),str1,sizeof(str1));
		else
			CString2Char(strVersionNew,str1,sizeof(str2));	

		n=strVersionOld.Find('.');
		if(n>0)
			CString2Char(strVersionOld.Left(n),str2,sizeof(str1));
		else
			CString2Char(strVersionOld,str2,sizeof(str2));

		if(atoi(str1)>atoi(str2)) 
			return 1;
		else if(atoi(str1)<atoi(str2))
			return -1;

		strVersionNew.Delete(0,n+1);
		strVersionOld.Delete(0,n+1);
	}
	return 0;
}
//查找文件的版本号
CString SearchFileVersion(CString strFileName,CString strVersionNew, CString dir, CString listFile)
{
	CString str;
	CString strVersionOld;
	FILE *fp=NULL;
	char buf[512];
    char line[512];
	int index;
	CString2Char(dir+listFile,buf,sizeof(buf));
	if( fopen_s( &fp, buf, "r" ) != 0 )
	{
        return strVersionOld; 
    } 
    memset(line,'\0',512);
   while( fgets( line, 512, fp ) != NULL)
   {
	   str=line;
	   index =str.Find(strFileName);
	   if(index!=-1)//找文件名
	   {
			   index=str.Find(_T("Version"));
			   if(index!=-1)//找版本号
			   {
				   index=str.Find(_T("=V"));
				   strVersionOld=str.Mid(index+2,str.ReverseFind(';')-index-2);
				   fclose( fp );
				   return strVersionOld;
			   }
	   }
   }  
  fclose( fp );
  return strVersionOld;
}
//记录文件更新信息
int UpdateInfo(CString Info,CString TagetFile)
{		
	SYSTEMTIME st; 
	GetLocalTime(&st); 

	char buf[512];
	char buf1[512];
	CString2Char(TagetFile,buf,sizeof(buf));
	FILE *fp;
	fopen_s(&fp,buf,"a+");
	sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	CString2Char(Info,buf1,sizeof(buf1));
	strcat(buf,buf1);
	fputs(buf,fp);
	fclose(fp);
	return 0;
}
//
int UpdateListFile(CString strFileName,CString strVersionOld,CString strVersionNew ,CString ListFile)
{
	char buf[2048];
	char buf1[512];
	CString strContents;
	CString2Char(ListFile,buf1,sizeof(buf1));
	FILE *fp;
	fp=fopen(buf1,"r");
	if(fp==NULL) return -1;
	memset(buf,NULL,2048);
	fread(buf,sizeof(char),2048,fp);
	if(fclose(fp)!=0) return -1;
	strContents=buf;
	strContents.Remove(' ');
	strContents.Replace(_T("FileName=")+strFileName+_T(";Version=V")+strVersionOld,_T("FileName=")+strFileName+_T(";Version=V")+strVersionNew);
	memset(buf,NULL,2048);
	CString2Char(strContents,buf,sizeof(buf));

	fopen_s(&fp,buf1,"w");
	if(fp==NULL) return -1;  

	fwrite(buf,sizeof( char ),strlen(buf),fp);//把字符串内容写入到文件
	fclose(fp);

	return 0;
}

Data_Packet TCP_Packet[MAX_TCP_PACKET];

bool AddTCP_Data(BYTE* pSendData,int ilen,int iStart,int iEnd)
{
	if(ilen>250) return false;
	BYTE*  pData;
	bool bAvilabile=false;
	int  ip=0;
	for (int i=iStart;i<iEnd+1;i++)
	{
		if (TCP_Packet[i].bNeedDeal==false && TCP_Packet[i].bUsing==false)
		{
			ip=i;
			bAvilabile=true;
			break;
		}
	}
	if (bAvilabile)
	{
		pData=TCP_Packet[ip].Data;
		TCP_Packet[ip].bUsing=true;

		memcpy(pData,pSendData,ilen);
		TCP_Packet[ip].isize=ilen;

		TCP_Packet[ip].bUsing=false;
		TCP_Packet[ip].bNeedDeal=true;
		return true;
	}
	return false;
}


#ifdef USING_LOG


Data_Packet Log_Packet[MAX_LOG_PACKET];

bool AddLog_Data(BYTE* pSendData,int ilen,int iStart,int iEnd)
{

	if (ilen>200)
	{
		return false;
	}
	BYTE*  pData;
	bool bAvilabile=false;
	int  ip=0;
	for (int i=0;i<MAX_LOG_PACKET;i++)
	{
		if (Log_Packet[i].bNeedDeal==false && Log_Packet[i].bUsing==false)
		{
			ip=i;
			bAvilabile=true;
			break;
		}
	}
	if (bAvilabile)
	{
		pData=Log_Packet[ip].Data;
		Log_Packet[ip].bUsing=true;

		memcpy(pData,pSendData,ilen);
		Log_Packet[ip].isize=ilen;

		Log_Packet[ip].bUsing=false;
		Log_Packet[ip].bNeedDeal=true;
		return true;
	}
	return false;
}


bool AddLog_Data(CString strContent,int iStart,int iEnd)
{
	char buf[256];
	CString2Char(strContent,buf,sizeof(buf));
	int ilen=strlen(buf);
	if (ilen>200)
	{
		return false;
	}
	BYTE *pSendData=(BYTE*)buf;

	bool bAvilabile=false;
	int  ip=0;
	for (int i=0;i<MAX_LOG_PACKET;i++)
	{
		if (Log_Packet[i].bNeedDeal==false && Log_Packet[i].bUsing==false)
		{
			ip=i;
			bAvilabile=true;
			break;
		}
	}
	if (bAvilabile)
	{	
		BYTE *pData=Log_Packet[ip].Data;
		Log_Packet[ip].bUsing=true;

		memcpy(pData,pSendData,ilen);
		Log_Packet[ip].isize=ilen;

		Log_Packet[ip].bUsing=false;
		Log_Packet[ip].bNeedDeal=true;
		return true;
	}
	return false;
}
#endif