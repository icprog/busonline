//************************************************
//项目：
//文件名：Serial.cpp 
//功能：串口类
//
//修改备注
//...
//************************************************
#include "StdAfx.h"
#include "Serial.h"


CSerial::CSerial(void)
{
	this->m_hComm = INVALID_HANDLE_VALUE;
}

CSerial::~CSerial(void)
{
	this->m_hWnd = NULL;
}

BOOL CSerial::Comm_Open(int nComm, DWORD dwBaut, PARITY parity, BYTE btDataBits, STOPBITS stopbits, FLOWCONTROL fc)
{
	CString str;
	COMMTIMEOUTS CommTimeouts;
	
	if(this->Comm_IsOpen())
		return TRUE;

	str.Format(TEXT("COM%d:"),nComm);

	this->m_hComm = CreateFile((LPTSTR)(LPCTSTR)str,
		GENERIC_READ | GENERIC_WRITE, 
		0,
		NULL,         // Pointer to the security attribute
		OPEN_EXISTING,// How to open the serial port
		0,            // Port attributes
		NULL);        // Handle to port with attribute to copy
	if(this->m_hComm == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if(!this->Comm_SetBuffer(COMM_BUFFER_IN,COMM_BUFFER_OUT))
	{
		this->Comm_Close();
		return FALSE;
	}
	//set comm 
	if(!this->Comm_SetBaut(dwBaut,parity,btDataBits,stopbits,fc))
	{
		this->Comm_Close();
		return FALSE;
	}

	//set commtimeouts
	GetCommTimeouts(this->m_hComm, &CommTimeouts);
	
	// Change the COMMTIMEOUTS structure settings.
	CommTimeouts.ReadIntervalTimeout = MAXDWORD;  
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;  
	CommTimeouts.ReadTotalTimeoutConstant = 0;    
	CommTimeouts.WriteTotalTimeoutMultiplier = 10;  
	CommTimeouts.WriteTotalTimeoutConstant = 1000;    
	
	// Set the time-out parameters for all read and write operations
	// on the port. 
	if (FALSE == SetCommTimeouts(this->m_hComm, &CommTimeouts))
	{
		this->Comm_Close();
		return FALSE;
	}

	if(!this->Comm_Clear(PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		this->Comm_Close();
		return FALSE;
	}

	return TRUE;
}

BOOL CSerial::Comm_IsOpen(void)
{
	return (this->m_hComm != INVALID_HANDLE_VALUE);
}

void CSerial::Comm_Close(void)
{
	if(this->Comm_IsOpen())
	{
		if(!CloseHandle(this->m_hComm))
		{
			TRACE(TEXT("关闭串口错误%d\n"),::GetLastError());
		}
		this->m_hComm = INVALID_HANDLE_VALUE;
	}
}

HANDLE CSerial::Comm_GetHandle(void)
{
	return this->m_hComm;
}

BOOL CSerial::Comm_SetBaut(DWORD dwBaut, PARITY parity, BYTE btDataBits, STOPBITS stopbits, FLOWCONTROL fc)
{
	DCB dcb;

	if(!this->Comm_IsOpen())
		return FALSE;

    // set dcb
	dcb.DCBlength = sizeof(DCB);
	::GetCommState(this->m_hComm,&dcb);

	//set baut
	dcb.BaudRate = dwBaut;
	
	//set parity
	dcb.fParity = TRUE;
	switch(parity)
	{
	case PT_NONE:
		dcb.fParity = FALSE;
		dcb.Parity = NOPARITY;
		break;
	case PT_ODD:
		dcb.Parity = ODDPARITY;
		break;
	case PT_EVEN:
		dcb.Parity = EVENPARITY;
		break;

	default:
		dcb.fParity = FALSE;
		dcb.Parity = NOPARITY;
	}
	//set databits
	dcb.ByteSize = btDataBits;
	//stop bits
	switch(stopbits)
	{
	case SB_ONE:
		dcb.StopBits = ONESTOPBIT;
		break;
	case SB_ONEPOINTFIVE:
		dcb.StopBits = ONE5STOPBITS;
		break;
	case SB_TWO:
		dcb.StopBits = TWOSTOPBITS;
		break;
	default:
		dcb.StopBits = ONESTOPBIT;
	}
	//control 
	switch(fc)
	{
	case FC_NONE:
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE; 
		dcb.fDtrControl = DTR_CONTROL_ENABLE; 
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		break;
	case FC_RTSCTS:
		dcb.fOutxCtsFlow = TRUE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
		dcb.fDtrControl = DTR_CONTROL_ENABLE; 
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		break;
	case FC_XONXOFF:
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE; 
		dcb.fDtrControl = DTR_CONTROL_ENABLE; 
		dcb.fOutX = TRUE;
		dcb.fInX = TRUE;
		dcb.XonChar = 0x11;
		dcb.XoffChar = 0x13;
		dcb.XoffLim = COMM_XOFF_LIMIT;
		dcb.XonLim = COMM_XON_LIMIT;
		break;
	default:
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE; 
		dcb.fDtrControl = DTR_CONTROL_ENABLE; 
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
	}
	//
	if(!::SetCommState(this->m_hComm,&dcb))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CSerial::Comm_SetBuffer(DWORD dwIn, DWORD dwOut)
{
	if(!this->Comm_IsOpen())
	{
		return FALSE;
	}
	return ::SetupComm(this->m_hComm,dwIn,dwOut);
}

BOOL CSerial::Comm_Clear(DWORD dwFlags)
{
	if(!this->Comm_IsOpen())
	{
		return FALSE;
	}
	return ::PurgeComm(this->m_hComm,dwFlags);
}

BOOL CSerial::Comm_Read(BYTE* pbtBuffer, DWORD dwCount, LPDWORD lpdwRead)
{
	if(!this->Comm_IsOpen())
		return FALSE;
	return (::ReadFile(this->m_hComm,pbtBuffer,dwCount,lpdwRead,NULL));
}

BOOL CSerial::Comm_Write(BYTE* pbtBuffer, DWORD dwCount, LPDWORD lpdwWritten)
{
	if(!this->Comm_IsOpen())
		return FALSE;
	return (::WriteFile(this->m_hComm,pbtBuffer,dwCount,lpdwWritten,NULL));
}

BOOL CSerial::Comm_BytesInBuffer(DWORD* lpdwIn, DWORD* lpdwOut)
{
	if(!this->Comm_IsOpen())
		return FALSE;
	COMSTAT comstat;
	DWORD dwError;
	if(::ClearCommError(this->m_hComm,&dwError,&comstat))
	{
		*lpdwIn = comstat.cbInQue;
		*lpdwOut = comstat.cbOutQue;
		return TRUE;
	}
	return FALSE;
}

BOOL CSerial::Comm_GetModemStatus(LPDWORD lpdwStatus)
{
	if(!this->Comm_IsOpen())
		return FALSE;
	return(::GetCommModemStatus(this->m_hComm,lpdwStatus));
}

void CSerial::Comm_Process(BYTE bRecv)
{
	//这里调用了一个虚函数，便于从该类继承
	this->Comm_ProcessDo(bRecv);
}

void CSerial::Comm_ProcessDo(BYTE bRecv)
{
	//其它处理，特别是串口传输具有特定格式，可以放在这里处理

	if(this->m_hWnd != NULL)
	{
		//接收到了字符，如果窗口句柄不为空，发送一个收到字符消息
		//参考此消息，你可以定制你要的消息
		::SendMessage(this->m_hWnd,WM_COMM_RECV,(WPARAM)bRecv,0);
	}
}

void CSerial::Comm_SetWnd(HWND hWnd)
{
	this->m_hWnd = hWnd;
}
/*
*函数介绍：往串口写入数据
*入口参数：buf ：待写入数据缓冲区
bufLen : 待写入缓冲区长度
		 *出口参数：(无)
		 *返回值：TRUE:设置成功;FALSE:设置失败
		 */
 BOOL CSerial::Comm_WriteSyncPort(const BYTE*buf , DWORD bufLen)
{
	DWORD dwNumBytesWritten;
	DWORD dwHaveNumWritten =0 ; //已经写入多少

	int iInc = 0; //如果3次写入不成功，返回FALSE
	ASSERT(m_hComm != INVALID_HANDLE_VALUE);
	do
	{
		if (WriteFile (m_hComm,					//串口句柄 
			buf+dwHaveNumWritten,				//被写数据缓冲区 
			bufLen - dwHaveNumWritten,          //被写数据缓冲区大小
			&dwNumBytesWritten,					//函数执行成功后，返回实际向串口写的个数	
			NULL))								//此处必须设置NULL
		{
			dwHaveNumWritten = dwHaveNumWritten + dwNumBytesWritten;
			//写入完成
			if (dwHaveNumWritten == bufLen)
			{
				break;
			}
			iInc++;
			if (iInc >= 3)
			{
				return false;
			}
			Sleep(20);
		}
		else
		{
			return false;
		}
	}while (true);

	return true;		
}