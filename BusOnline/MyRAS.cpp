#include "StdAfx.h"
#include "MyRAS.h"
#include <ras.h>
#include <raserror.h>

HRASCONN CRAS_CE::m_hRasConn = NULL;
CWnd* CRAS_CE::m_pEventWnd = NULL;
BOOL CRAS_CE::m_bConnected = FALSE;
BOOL CRAS_CE::m_bDialing = FALSE;

CRAS_CE::CRAS_CE(void)
{
	m_iEntriesCount = 0;
	m_psEntriesNames = NULL;
}

CRAS_CE::~CRAS_CE(void)
{
	if(m_psEntriesNames)
	{
		delete [] m_psEntriesNames;
	}
}
BOOL CRAS_CE::Initialize(CWnd* pEventWnd)
{
	m_pEventWnd = pEventWnd;
    return TRUE;	
}

CString CRAS_CE::GetEntry(int iIndex)  const
{
	if (iIndex < 0 || m_iEntriesCount <= iIndex )
	{
		return NULL;
	}

	return m_psEntriesNames[iIndex];
}

BOOL CRAS_CE::Dial(CString sEntry, CString sUser, CString sPassword)
{
	m_bDialing = TRUE;
	DWORD dwRet = ERROR_SUCCESS;

	RASDIALPARAMS rasdialparams;
	ZeroMemory(&rasdialparams, sizeof(RASDIALPARAMS));
	rasdialparams.dwSize = sizeof(RASDIALPARAMS);

	rasdialparams.szPhoneNumber[0] = TEXT('\0');
	rasdialparams.szCallbackNumber[0] = TEXT('\0');
	wcscpy(rasdialparams.szEntryName, sEntry);
	wcscpy (rasdialparams.szUserName, sUser);
	wcscpy (rasdialparams.szPassword, sPassword);
	wcscpy (rasdialparams.szDomain, L"");

	//dwRet = RasDial (NULL, NULL, &rasdialparams, 0xFFFFFFFF, (LPVOID)m_pEventWnd->GetSafeHwnd(), &m_hRasConn);
	dwRet = RasDial (NULL, NULL, &rasdialparams, 0L,NULL, &m_hRasConn);
	if (dwRet)
	{	
		m_bDialing = FALSE;
		return FALSE;
	}
	m_bConnected = TRUE;
	return TRUE;
}

BOOL CRAS_CE::HangUp(void)
{
	DWORD dwRet;
	RASCONNSTATUS rStatus;
	ZeroMemory(&rStatus, sizeof(RASCONNSTATUS));
	rStatus.dwSize = sizeof(RASCONNSTATUS);
	dwRet = RasGetConnectStatus(m_hRasConn, &rStatus);
	if (dwRet != ERROR_INVALID_HANDLE)
	{
		RasHangUp(m_hRasConn);
		MSG msg;
		while (dwRet != ERROR_INVALID_HANDLE)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			RasHangUp(m_hRasConn);
			dwRet = RasGetConnectStatus(m_hRasConn, &rStatus);
		}
	}
	Sleep(3000);
    return TRUE;
}
RASCONNSTATE CRAS_CE::GetConnectionStatus(void)
{
	RASCONNSTATUS rasconnstatus;
	DWORD dwRet;

	ZeroMemory(&rasconnstatus, sizeof(RASCONNSTATUS));
	rasconnstatus.dwSize = sizeof(RASCONNSTATUS);
	dwRet = RasGetConnectStatus(m_hRasConn, &rasconnstatus);
	RASCONNSTATE rasconnstate = rasconnstatus.rasconnstate;
	return rasconnstate;
}
BOOL CRAS_CE::isConnected(void)
{
	return m_bConnected;
}

BOOL CRAS_CE::isDialing(void)
{
	return m_bDialing;
}
BOOL CRAS_CE::SetConnect()
{
	m_bDialing=FALSE;
    m_bConnected=FALSE;
	return TRUE;
}