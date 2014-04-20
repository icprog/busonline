#pragma once
#include "serial.h"
#include "globe.h"
DWORD CommReadSSIThread(LPVOID lpParam);

class CSSIcomm :
	public CSerial
{
public:
	CSSIcomm(void);
	~CSSIcomm(void);
public:
	int iPos;
	char SSI_Buf[50];//���յ��ֽ�
	BYTE TCP_SSI_Buf[60];
	unsigned char SSI_SendBuf[512];//������ȵ��������͵��ֽ�

	unsigned char RecvState;
	bool OpenSSIComm();
	void Comm_ProcessDo(BYTE bRecv);

public:
	HANDLE m_comThreadHandle;
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;

	enum STATE_UPDATE
	{
		CHECH, //��ѯ����ȵ������Ƿ���Ҫ����
		SEND,//����������������
		FINISH  //�������
	};
	int UpdateState;
	BOOL bUpdate;// TRUE ����������FAlSE������
	BOOL UpdateSSISoftWare(CString strFileName=(CString)g_SSIUpateName);
	void ResetUpdateState();
	void AddMessage(CString strMessage);//�����Ϣ�����Ͷ���
	bool FormatSSIData(char* pSSI_Buf,int ilen);
	BOOL Close();
	BOOL QueryDriverInfo();//��ѯ˾����Ϣ
public:
	unsigned char ReplyCMD; //����ȵ������ظ�����
	unsigned char CMD;//������������
	unsigned int nDataSize;
	unsigned int nRemainingBytes;

	int numRead;
	char sendbuf[130];
	unsigned int FrameNO;//���͵�����֡���
	unsigned int nBytesSend;
	BOOL bReply;//����ʱ����
	FILE *fp;//�����ļ����
	CString strDebugInfo;//��ʾ������״̬
};
