#pragma once

#include <ras.h>
#include <raserror.h>
class CRAS_CE
{
public:
	CRAS_CE(void);
	~CRAS_CE(void);


	BOOL HangUp(void);	
	BOOL isDialing(void);
	BOOL isConnected(void);
	BOOL Initialize(CWnd* pEventWnd);
	BOOL Dial(CString sEntry, CString sUser, CString sPassword);

	RASCONNSTATE GetConnectionStatus(void);
	CString GetEntry(int iIndex) const;
	BOOL SetConnect();
private:
	int m_iEntriesCount;

	CString *m_psEntriesNames;

	static HRASCONN m_hRasConn;
	static CWnd* m_pEventWnd;
	static BOOL m_bConnected;
	static BOOL m_bDialing;
};
