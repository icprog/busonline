// MemoryCtrl.cpp: implementation of the CMemoryCtrl class.   
//   
//////////////////////////////////////////////////////////////////////   
   
#include "stdafx.h"   
#include "memory.h"   
#include "MemoryCtrl.h"   
   
#ifdef _DEBUG   
#undef THIS_FILE   
static char THIS_FILE[]=__FILE__;   
#define new DEBUG_NEW   
#endif   
   
//////////////////////////////////////////////////////////////////////   
// Construction/Destruction   
//////////////////////////////////////////////////////////////////////   
   
CMemoryCtrl::CMemoryCtrl()   
{   
   
}   
   
CMemoryCtrl::~CMemoryCtrl()   
{   
   
}   
void CMemoryCtrl::GetMemUsgesNt()   
{   
    MEMORYSTATUS MemStat;   
    MemStat.dwLength = sizeof(MEMORYSTATUS);   
    GlobalMemoryStatus(&MemStat);   
    m_ulNewUsges     = MemStat.dwMemoryLoad;   //占用内存百分比   
    //The approximate percentage of total physical memory that is in use   
    mem = MemStat.dwAvailPhys ;//可用（就是剩余大小）Avai-有效     可用物理内存数量   
    //Size of physical memory available,   
    totalmem = MemStat.dwTotalPhys;//总                            总的物理内存数量   
    //Total size of physical memory,   
    viual = MemStat.dwAvailVirtual;//虚拟                          用户进程可用最大虚拟内存数量   
    //Size of physical memory available, in bytes.    
    TotalPageFile =MemStat.dwTotalPageFile;                 
    //Size of the committed memory limit   
    TotalVirtual  =MemStat.dwTotalVirtual;   
    //Total size of the user mode portion of the virtual address space of the calling process   
    AvailPageFile =MemStat.dwAvailPageFile;   
    //Size of available memory to commit,   
    AvailPhys  =MemStat.dwAvailPhys;        
}   