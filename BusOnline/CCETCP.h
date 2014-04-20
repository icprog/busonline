#ifndef __CCETCP_H__
#define __CCETCP_H__

#include "CESocket.h"
#include "containers.h"
#include "BusInfo.h"
#include "CEFTP.h"


// extern CLog *pLog;



class CCETCP:public CCESocket
{
public:
	CCETCP();
	~CCETCP();
public:


	CBusInfo *m_pBusInfo;
	void GetBusInfo(CBusInfo* pBusInfo)
	{
		m_pBusInfo=pBusInfo;
	};
	//����ϵͳʱ��
	BOOL RecvTime(BYTE* pData,int nDataLen);
	void ReadData(const char * buf, DWORD dwBufLen);
    //��ȡ����IP
	CString GetLocalIP();
	
    //���Ӻ�̨������
	bool LinkServer();
	//�Ƿ��Ѿ����ӷ�����
	bool IsConnect();
	bool SendTCPPacket();

	HANDLE m_SendThread;
	CTList<Data_Packet*> list_TCPData;
	bool AddTCPData(Data_Packet* tcpData);
	static DWORD WINAPI SendThreadFun(LPVOID pParam);
	threadState m_SendThreadState;

	CString m_strHostIP;
	CString m_strBusCode;
	DWORD m_dwPort;
	void SetConfig(CString BusCode,CString HostIP, DWORD Port);
	void GetConfig();

	CString m_strSendComment;//����������͵�����
private:
    virtual bool OnReceive(char* buf, int len);
	bool TCPDataDeal(BYTE* pData,int ilen);


public:
	//�洢��������
	BOOL SplitData(BYTE* pData,int nDataLen);
	BOOL Close();

public:
	CCEFTP m_FTP;
public:
	DWORD m_dwRecvBytes;
	DWORD m_dwSendBytes;
};
#endif//__CCETCP_H__