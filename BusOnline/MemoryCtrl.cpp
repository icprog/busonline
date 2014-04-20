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
    m_ulNewUsges     = MemStat.dwMemoryLoad;   //ռ���ڴ�ٷֱ�   
    //The approximate percentage of total physical memory that is in use   
    mem = MemStat.dwAvailPhys ;//���ã�����ʣ���С��Avai-��Ч     ���������ڴ�����   
    //Size of physical memory available,   
    totalmem = MemStat.dwTotalPhys;//��                            �ܵ������ڴ�����   
    //Total size of physical memory,   
    viual = MemStat.dwAvailVirtual;//����                          �û����̿�����������ڴ�����   
    //Size of physical memory available, in bytes.    
    TotalPageFile =MemStat.dwTotalPageFile;                 
    //Size of the committed memory limit   
    TotalVirtual  =MemStat.dwTotalVirtual;   
    //Total size of the user mode portion of the virtual address space of the calling process   
    AvailPageFile =MemStat.dwAvailPageFile;   
    //Size of available memory to commit,   
    AvailPhys  =MemStat.dwAvailPhys;        
}   