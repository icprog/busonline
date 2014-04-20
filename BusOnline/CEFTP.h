#pragma once
#include "cesocket.h"
#include "globe.h"
#include "Log.h"

typedef struct FileDnloadInfo{
	CString strFileName;
	DWORD dwFileSize;
	DWORD dwDnloadedSize;
}FileDnloadInfo;//�ϵ����ؼ�¼

typedef struct FileList{
	CString  strListFileName;//�ļ��б���
	int nFileNum;//��Ҫ���ص��ļ��������������ļ��б��ļ�
	CString strFileName[20];//��Ҫ���ص��ļ���
	CString strVersion[20];//�ļ��汾��
	BOOL bDnloaded[20];//�Ƿ��Ѿ��������
};//�����ļ��б�


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
	BOOL bResetDevice;//�����������
	void FTPDownload(BYTE* pData,int ilen);
	void FTPClose(void);
	CString m_strFileName;
	CString m_strDnloadDir;
	FileDnloadInfo m_FileDnloadInfo; //�ϵ����ؽṹ�壬��¼�ļ������Ѿ������ֽ������ļ����ֽ���
	FileList m_FileList;//�����ļ��б�

	BOOL bNewExe_Flag;	//���ص����������
	CString strDnldLineFileName;//�����ص�·������
	CString strDnldExeFileName;//�����ص�Exe�ļ�����

public:
	BOOL bNewConfig_Flag;//�����ļ����±��
	BOOL bNewSSI_Flag;//����ȵ��������±��
	BOOL bDownloading;//����״̬���
	CString strDebugInfo;//������Ϣ

	Data_Packet Log_Packet[3];
    bool OnReceive(char* buf, int len);
	SOCKET createDataSocket(const char * mess);//������������
	BOOL GetDnloadFileName();//��ȡ��Ҫ���ص��ļ���;
	BOOL SendData(const char * buf , DWORD dwBufLen);
	void InitFileList();
	int UpdateFileList();
	SOCKET m_dataSocket;
	void Close();
	BOOL DeleteTempdFile(CString strPath ,UINT nDates);
};
