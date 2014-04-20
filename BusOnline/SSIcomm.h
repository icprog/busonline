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
	char SSI_Buf[50];//接收的字节
	BYTE TCP_SSI_Buf[60];
	unsigned char SSI_SendBuf[512];//向满意度调查器发送的字节

	unsigned char RecvState;
	bool OpenSSIComm();
	void Comm_ProcessDo(BYTE bRecv);

public:
	HANDLE m_comThreadHandle;
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;

	enum STATE_UPDATE
	{
		CHECH, //查询满意度调查器是否需要升级
		SEND,//发送升级程序数据
		FINISH  //完成升级
	};
	int UpdateState;
	BOOL bUpdate;// TRUE 正在升级，FAlSE不升级
	BOOL UpdateSSISoftWare(CString strFileName=(CString)g_SSIUpateName);
	void ResetUpdateState();
	void AddMessage(CString strMessage);//添加消息到发送队列
	bool FormatSSIData(char* pSSI_Buf,int ilen);
	BOOL Close();
	BOOL QueryDriverInfo();//查询司机信息
public:
	unsigned char ReplyCMD; //满意度调查器回复命令
	unsigned char CMD;//车载主机命令
	unsigned int nDataSize;
	unsigned int nRemainingBytes;

	int numRead;
	char sendbuf[130];
	unsigned int FrameNO;//发送的数据帧序号
	unsigned int nBytesSend;
	BOOL bReply;//升级时返回
	FILE *fp;//升级文件句柄
	CString strDebugInfo;//显示升级的状态
};
