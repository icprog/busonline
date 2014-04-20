#ifndef _LOG_H_
#define _LOG_H_
#include "containers.h"
#include "globe.h"


class CLog
{
public:
	CLog(void);
	~CLog(void);
protected :
	BOOL fstate; //文件状态 TRUE 占用 ，FALSE 空闲
	enum threadState {CLOSED=0, CLOSING, RUNNING};
	threadState m_WriteThreadState;
public:
	BOOL WriteLog(char *pContent,DWORD len);
	BOOL WriteLog(CString Content);
	BOOL WriteLog(char *pContent,DWORD len,CString TagetFile);
	BOOL WriteLog(CString Content,CString TagetFile);
	BOOL OpenFile();
	BOOL SetFileName();
	bool WriteLogPacket();
	static DWORD WINAPI WriteThreadFun(LPVOID pParam);
public:
	HANDLE m_WriteThread;
	FILE *fp;
	CString m_strFileName;

	CTList<Data_Packet*> list_LogData;
	bool AddLogData(Data_Packet* logData);
	BOOL DeleteOutdatedFile(CString strPath = _T("\\NAND\\*.log"),UINT nDates = 7);
	BOOL Close();
};
#endif