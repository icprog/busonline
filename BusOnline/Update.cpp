#include "StdAfx.h"
#include "Update.h"
#include "globe.h"

CUpdate::CUpdate(void)
{
}

CUpdate::~CUpdate(void)
{
}
void CUpdate::UpdataFromSDCardCfg()
{
	WIN32_FIND_DATA fdNew;
	CString strSDDir;
	strSDDir=g_SDCardDir;
	CString SdCfgPath;
	CString strSetingName;
	strSetingName=g_SettingFile;
	SdCfgPath=strSDDir+strSetingName;
	HANDLE hNewFind = ::FindFirstFile( SdCfgPath, &fdNew );
	if ( hNewFind != INVALID_HANDLE_VALUE )
	{
		CString strCurPath;
		strCurPath= g_WorkDir;
		strCurPath = strCurPath + strSetingName;
		WIN32_FIND_DATA fdCur;
		HANDLE hCurFind = ::FindFirstFile( strCurPath, &fdCur );
		if ( hCurFind != INVALID_HANDLE_VALUE )
		{
			int rc = DeleteFile( strCurPath );
			if( !rc )
				rc = GetLastError( );
			::FindClose(hCurFind);
		}

		//�������ƶ�����ǰִ���ļ�·��
		int rc = CopyFile( SdCfgPath, strCurPath ,0);
		if( !rc )
			rc = GetLastError( );
		::FindClose(hNewFind);
	}
}
