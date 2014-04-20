// MemoryCtrl.h: interface for the CMemoryCtrl class. 
// 
////////////////////////////////////////////////////////////////////// 
 
#if !defined(AFX_MEMORYCTRL_H__B4A843F4_A41B_4FB3_9240_7F5CB0FA9FD9__INCLUDED_) 
#define AFX_MEMORYCTRL_H__B4A843F4_A41B_4FB3_9240_7F5CB0FA9FD9__INCLUDED_ 
 
#if _MSC_VER > 1000 
#pragma once 
#endif // _MSC_VER > 1000 
 
class CMemoryCtrl : public CWnd   
{ 
public: 
	CMemoryCtrl(); 
	virtual ~CMemoryCtrl(); 
	void GetMemUsgesNt(); 
	unsigned long m_ulNewUsges;   
	DWORD mem ; 
	DWORD totalmem ; 
	DWORD viual ; 
	DWORD TotalPageFile; 
	DWORD TotalVirtual; 
	DWORD AvailPageFile ; 
    DWORD AvailPhys; 	 
}; 
 
#endif // !defined(AFX_MEMORYCTRL_H__B4A843F4_A41B_4FB3_9240_7F5CB0FA9FD9__INCLUDED_)