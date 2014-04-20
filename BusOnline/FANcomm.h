#pragma once
#include "serial.h"
#include "globe.h"
#include "Log.h"

DWORD CommReadFANThread(LPVOID lpParam);
class CFANcomm :
	public CSerial
{
public:
	int RecvState;
	int iPos;
	char FAN_Buf[100];
	BYTE TCP_FAN_Buf[120];

	BYTE FAN_SendBuf[512];//向电子风扇发送
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;
	unsigned int Index;//轮询数据的索引，对应下标
	BOOL bRecvFlag[16];//轮询返回值成功标志
	WORD DATA[16];/*电子风扇数据 
					下标
					0://PWM当前值
					1://PWM设定值 
					2://温度值
					3://电流值
					4://电压值
					5://速度值
					6://开关值
					7://报警值
					8://次控温度
					9://版本号
					10:// Value_MainKind主控次控标志
					11:// Value_ControlKind主控读取次控温度标志
					12:// Value_ComKind风扇类型标志
				*/

	//CLog m_Log;
public:
	CFANcomm(void);
	~CFANcomm(void);
	bool OpenFANComm();
	void Comm_ProcessDo(BYTE bRecv);
	bool FormatFANData(char* pFAN_Buf,int ilen);
	void DataPolling();
	void SetDataValue(char* pFAN_Buf);
	BOOL Close();
public:
	HANDLE m_comThreadHandle;
};
