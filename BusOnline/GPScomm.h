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
	void GPSPara();//GPS����
public:
	char GPRMC_Buf[100];
	BYTE TCP_GPRMC_Buf[120];
	HANDLE hEvent_GpsReved;//���յ�������GPS���ݰ�
	unsigned char RecvState;
	int iPos;
	//ͨѶ�߳̾��
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
		char UTCTime[10]; // ʱʱ�ַ�����.������
		char Status;          // A/V
		char Latitude[9];     //ddmm.mmmm
		char NS;              //N/S
		char Longitude[10];   //dddmm.mmmm
		char EW;              //E/W
		char Speed[5];        //����000.0~999.9��
		char Course[5];       //����000.0~359.9��
		char UTCDate[6];     // ������������
	}stru_GPSRMC;
	stru_GPSRMC GPSRMC;

};
