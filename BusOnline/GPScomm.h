#pragma once
#include "serial.h"
#include "CCETCP.h"


DWORD CommReadGPSThread(LPVOID lpParam);
class CGPScomm :
	public CSerial
{
public:
	CGPScomm(void);
	~CGPScomm(void);
	void GPSPara();//GPS解析
public:
	char GPRMC_Buf[100];
	BYTE TCP_GPRMC_Buf[120];
	HANDLE hEvent_GpsReved;//接收到完整的GPS数据包
	unsigned char RecvState;
	int iPos;
	//通讯线程句柄
	HANDLE m_comThreadHandle;
    virtual void Comm_ProcessDo(BYTE bRecv);
public:
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;
public:
	bool OpenGPSComm();
	void StrucGPRMInit();
	bool GPSSetTime(char UTCime[10],char UTCDate[6]);
	bool AddLog_GpsData(BYTE* pSendData,int ilen);
	bool FormatGPSData(char *pCAN_Buf,int ilen);
	BOOL Close();
	typedef struct _Struct_GPSRMC
	{
		char UTCTime[10]; // 时时分分秒秒.秒秒秒
		char Status;          // A/V
		char Latitude[9];     //ddmm.mmmm
		char NS;              //N/S
		char Longitude[10];   //dddmm.mmmm
		char EW;              //E/W
		char Speed[5];        //速率000.0~999.9节
		char Course[5];       //航向000.0~359.9度
		char UTCDate[6];     // 日日月月年年
	}stru_GPSRMC;
	stru_GPSRMC GPSRMC;

};
