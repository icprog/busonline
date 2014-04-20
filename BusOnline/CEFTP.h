#pragma once
#include "cesocket.h"
#include "globe.h"
#include "Log.h"

typedef struct FileDnloadInfo{
	CString strFileName;
	DWORD dwFileSize;
	DWORD dwDnloadedSize;
}FileDnloadInfo;//断点下载记录

typedef struct FileList{
	CString  strListFileName;//文件列表名
	int nFileNum;//需要下载的文件个数，不包括文件列表文件
	CString strFileName[20];//需要下载的文件名
	CString strVersion[20];//文件版本号
	BOOL bDnloaded[20];//是否已经完成下载
};//下载文件列表


class CCEFTP :
	public CCESocket
{
public:
	CCEFTP(void);
	~CCEFTP(void);
public:
	CString markID;
public:
	CString m_FTPServer;
	CString m_FTPUserName;
	CString m_FTPPassword;
	CString m_FTPRemote;
	CString m_FTPFiletype;
	CString m_RetFile;
	CString m_FTPDir;
	UINT m_FTPPort;
	BOOL bResetDevice;//重启主机标记
	void FTPDownload(BYTE* pData,int ilen);
	void FTPClose(void);
	CString m_strFileName;
	CString m_strDnloadDir;
	FileDnloadInfo m_FileDnloadInfo; //断点下载结构体，记录文件名、已经下载字节数、文件总字节数
	FileList m_FileList;//下载文件列表

	BOOL bNewExe_Flag;	//车载电脑软件更新
	CString strDnldLineFileName;//新下载的路线名称
	CString strDnldExeFileName;//新下载的Exe文件名称

public:
	BOOL bNewConfig_Flag;//配置文件更新标记
	BOOL bNewSSI_Flag;//满意度调查器更新标记
	BOOL bDownloading;//下载状态标记
	CString strDebugInfo;//调试信息

	Data_Packet Log_Packet[3];
    bool OnReceive(char* buf, int len);
	SOCKET createDataSocket(const char * mess);//建立数据连接
	BOOL GetDnloadFileName();//获取需要下载的文件名;
	BOOL SendData(const char * buf , DWORD dwBufLen);
	void InitFileList();
	int UpdateFileList();
	SOCKET m_dataSocket;
	void Close();
	BOOL DeleteTempdFile(CString strPath ,UINT nDates);
};
