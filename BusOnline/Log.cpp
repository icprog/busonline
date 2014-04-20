
#include <stdio.h>
#include <algorithm>  
#include "StdAfx.h"
#include  < assert.h > 
#include  < time.h > 
#include  < stdio.h > 
#include  < windows.h >
#include "Log.h"
#include <stdlib.h>

CLog::CLog(void)
{
	DeleteOutdatedFile();//删除过期的数据的日志文件
	fstate = FALSE;
	m_WriteThread= CreateThread(NULL, 0, WriteThreadFun, this, 0, NULL );
	fp = NULL;
	SetFileName();
	OpenFile();
}

DWORD WINAPI CLog::WriteThreadFun(LPVOID pParam)
{
	CLog *parent = (CLog*) pParam;
	parent->m_WriteThreadState = RUNNING;
	do 
	{
		parent->WriteLogPacket();
		Sleep(1);
	} while (parent->m_WriteThreadState == RUNNING);
	parent->m_WriteThreadState = CLOSED;
	parent->m_WriteThread = NULL;
	return 0;
}
bool CLog::AddLogData(Data_Packet* logData)
{
	list_LogData.AddTail(logData);
	return true;
}

bool CLog::WriteLogPacket()
{

	
	if(!list_LogData.IsEmpty()) 
	{
		Data_Packet* pData;
		memset(pData->Data,NULL,60);

		pData = list_LogData.GetHead();
		if (pData->bNeedDeal==true &&pData->bUsing==false)
		{
			pData->bUsing=true;
			WriteLog((char*)pData->Data,pData->isize);
			pData->bNeedDeal=false;
			pData->bUsing=false;
		}
		while(!list_LogData.IsLast())
		{
			pData = list_LogData.GetNext();
			if (pData->bNeedDeal==true &&pData->bUsing==false)
			{
				pData->bUsing=true;
				WriteLog((char*)pData->Data,pData->isize);
				pData->bNeedDeal=false;
				pData->bUsing=false;
			}
		}
	}
	return true;
}
CLog::~CLog(void)
{
	if (fp!=NULL)
	{
		fclose(fp);
	}

}

BOOL CLog::OpenFile()
{
	
	if (fp != NULL)
	{
		fflush(fp);
		fclose(fp);
		fp = NULL;
	}
	fp = _wfopen(m_strFileName,L"a+");
	if (fp == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CLog::Close()
{
	TerminateThread(m_WriteThread,0);
	m_WriteThread = NULL;
	if (fp != NULL)
	{
		fflush(fp);
		fclose(fp);
		fp = NULL;
	}
	return TRUE;
}

BOOL CLog::WriteLog(CString Content)
{
	if(fp == NULL)
	{
		SetFileName();
		if(OpenFile() != TRUE)
		{
			WriteLog("文件打开失败",strlen("文件打开失败"),L"error.log");
		}
	}
	char buf[512];
	SYSTEMTIME st; 
	GetLocalTime(&st); 
	sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fwrite(buf,sizeof(char),strlen(buf),fp);

	CString2Char(Content,buf,sizeof(buf));
	fwrite(buf,sizeof(char),strlen(buf),fp);
	fwrite("\r\n",sizeof(char),strlen("\r\n"),fp);
	fflush(fp);
	return TRUE;
}

BOOL  CLog::WriteLog(char *pContent,DWORD len)
{

	if (len>200)
	{
		return FALSE;
	}
	if(fp == NULL)
	{
		SetFileName();
		if(OpenFile() != TRUE)
		{
			WriteLog("文件打开失败",strlen("文件打开失败"),L"error.log");
		}
	}

	char buf[512];
	SYSTEMTIME st; 
	GetLocalTime(&st); 
	sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fwrite(buf,sizeof(char),strlen(buf),fp);
	fwrite(pContent,sizeof(char),len,fp);
	fwrite("\r\n",sizeof(char),strlen("\r\n"),fp);
	fflush(fp);
	return TRUE;
}
BOOL CLog::WriteLog(CString Content,CString TagetFile)
{
	SYSTEMTIME st; 

	GetLocalTime(&st); 

	FILE *fp = NULL;
	fp = _wfopen(TagetFile,L"a+");
	if(fp == NULL)
	{
		return -1;
	}

	char buf[512];
	sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fwrite(buf,sizeof(char),strlen(buf),fp);

	CString2Char(Content,buf,sizeof(buf));
	fwrite(buf,sizeof(char),strlen(buf),fp);
	fwrite("\r\n",sizeof(char),strlen("\r\n"),fp);
	fflush(fp);
	fclose(fp);
	return TRUE;
}
BOOL CLog::WriteLog(char *pContent,DWORD len,CString TagetFile)
{
	char buf[512];
	SYSTEMTIME st; 

	GetLocalTime(&st); 

	FILE *fp = NULL;
	fp = _wfopen(TagetFile,L"a+");
	if(fp == NULL)
	{
		return -1;
	}
	sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fwrite(buf,sizeof(char),strlen(buf),fp);
	fwrite(pContent,sizeof(char),len,fp);
	fwrite("\r\n",sizeof(char),strlen("\r\n"),fp);
	fflush(fp);
	fclose(fp);
	return 0;
}
BOOL CLog::SetFileName()
{
	SYSTEMTIME st; 
	GetLocalTime(&st); 
	char buf[256];
	sprintf(buf,"%04d%02d%02d.log",st.wYear,st.wMonth,st.wDay);
	m_strFileName = L"\\NAND\\";
	m_strFileName += (CString)buf;
	return TRUE;
}
//删除过期的文件
BOOL CLog::DeleteOutdatedFile(CString strPath ,UINT nDates)
{
	WIN32_FIND_DATAW wfd;

	//CString strPath =L"\\NAND\\*.log";//查找Log文件。

	HANDLE hFile = FindFirstFile(strPath,&wfd);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	do{
		SYSTEMTIME st; 
		GetLocalTime(&st); 

		FILETIME ft;

		SystemTimeToFileTime(&st,&ft);

		ft.dwHighDateTime =ft.dwHighDateTime - wfd.ftCreationTime.dwHighDateTime;
		ft.dwLowDateTime = ft.dwLowDateTime - wfd.ftCreationTime.dwLowDateTime;

		if ( ft.dwHighDateTime/201 >= nDates )
		{
			//删除超过 nDates 天的日志文件
			CString strFoundFilename = wfd.cFileName;
			DeleteFile(L"\\NAND\\"+strFoundFilename);
		}
	}while(FindNextFile(hFile,&wfd));
	return TRUE;
}


