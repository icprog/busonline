//************************************************
//项目：
//文件名：Serial.h 
//功能：串口类头文件
//
//修改备注
//...
//************************************************

#pragma once

#define COMM_XON_LIMIT 100
#define COMM_XOFF_LIMIT 100

#define COMM_BUFFER_IN	4096
#define COMM_BUFFER_OUT 1024

#define WM_COMM_RECV	WM_USER+100



enum FLOWCONTROL
{
	FC_NONE,
	FC_RTSCTS,
	FC_XONXOFF
};

enum PARITY
{    
	PT_NONE,
	PT_ODD,
	PT_EVEN,
	PT_MARK,
	PT_SPACE
};

enum STOPBITS
{
	SB_ONE,
	SB_ONEPOINTFIVE,
	SB_TWO
};


class CSerial
{
public:
	CSerial(void);
	~CSerial(void);
protected:
	HANDLE m_hComm;
public:
	BOOL Comm_Open(int nComm, DWORD dwBaut = 9600, PARITY parity = PT_NONE, BYTE btDataBits = 8, STOPBITS stopbits = SB_ONE, FLOWCONTROL fc = FC_NONE);
	BOOL Comm_IsOpen(void);
	void Comm_Close(void);
	HANDLE Comm_GetHandle(void);
	BOOL Comm_SetBaut(DWORD dwBaut = 9600, PARITY parity = PT_NONE, BYTE btDataBits = 8, STOPBITS stopbits = SB_ONE, FLOWCONTROL fc = FC_NONE);
	BOOL Comm_SetBuffer(DWORD dwIn, DWORD dwOut);
	BOOL Comm_Clear(DWORD dwFlags = PURGE_TXCLEAR | PURGE_RXCLEAR);
	BOOL Comm_Read(BYTE* pbtBuffer, DWORD dwCount, LPDWORD lpdwRead);
	BOOL Comm_Write(BYTE* pbtBuffer, DWORD dwCount, LPDWORD lpdwWritten);
    BOOL Comm_WriteSyncPort(const BYTE*buf , DWORD bufLen);
	BOOL Comm_BytesInBuffer(DWORD* dwIn, DWORD* dwOut);
	BOOL Comm_GetModemStatus(LPDWORD lpdwStatus);
	void Comm_Process(BYTE bRecv);
	virtual void Comm_ProcessDo(BYTE bRecv);
	HWND m_hWnd;
	void Comm_SetWnd(HWND hWnd);
};
