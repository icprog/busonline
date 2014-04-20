#pragma once
#include "globe.h"
#define  MAXINFONUM 10
typedef struct
{
	CString InfID ;
	CString InfClass;
	CString InfState;
	CString InfCmdType;
	CString InfReplyNo;
	CString InfTime;
	CString InfContent;
	BOOL    bRead;
	BOOL    bReply;
	BOOL    bOK;

}stru_InfPacket;

struct RingStruct//存储打铃时间
{
	BYTE bMonth;
	BYTE bDay;
	BYTE bHour;
	BYTE bMinute;
	BYTE nRingNum;
	BOOL bRinging;
	BOOL bRinged;
	BOOL bValue;
};

class CBusInfo
{
public:
	CBusInfo(void);
	~CBusInfo(void);
public:
	stru_InfPacket InfPacket[MAXINFONUM];
	stru_InfPacket RingInf;
	stru_InfPacket DispathInf;


	int iNoBufInf;//当前信息号
	BYTE flag_NewInf;//有调度新信息
	BYTE InfNoRead;//未读信息条数
	BYTE InfCount;//有信息总条数
	BYTE CurInf;
	BYTE LineNO[16];//线路名称
	BYTE StationCount;//总站数
	BYTE StationCur;//当前站点号

	//ID标识
	CString markID;

	BOOL bUsing;//CBusInfo对象正在被处理
public:
	void SetRingInf(BYTE* pData,int nDataLen);//设置打铃信息
	void SetDespathInf(BYTE* pData,int nDataLen);//设置调度信息

public:
	bool FormatRingTime(BYTE* pbuf,int i);
	bool Read4Record(BYTE bMonth,BYTE bDay,BYTE bHour,BYTE bMin,int nTag);
	bool Write4Record(BYTE bMonth,BYTE bDay,BYTE bHour,BYTE bMin,int nTag);
	bool Write4Send(BYTE* pBuf);
	bool Read4Send();



	//从文本文件读取打铃数据
	BOOL ReadFromTXT();
	//写入打铃数据到文本
	BOOL WriteToTXT();
	//对比系统时间与打铃时间
	BOOL CompareTime(RingStruct*);
	//存储打铃时间
	RingStruct RingData[40];
	bool bTXTReaded;
	BOOL bTimeShowed;

	//打铃测试
	BOOL bIsRecv;
};
