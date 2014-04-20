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

	BYTE FAN_SendBuf[512];//����ӷ��ȷ���
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;
	unsigned int Index;//��ѯ���ݵ���������Ӧ�±�
	BOOL bRecvFlag[16];//��ѯ����ֵ�ɹ���־
	WORD DATA[16];/*���ӷ������� 
					�±�
					0://PWM��ǰֵ
					1://PWM�趨ֵ 
					2://�¶�ֵ
					3://����ֵ
					4://��ѹֵ
					5://�ٶ�ֵ
					6://����ֵ
					7://����ֵ
					8://�ο��¶�
					9://�汾��
					10:// Value_MainKind���شοر�־
					11:// Value_ControlKind���ض�ȡ�ο��¶ȱ�־
					12:// Value_ComKind�������ͱ�־
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
