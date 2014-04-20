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

struct RingStruct//�洢����ʱ��
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


	int iNoBufInf;//��ǰ��Ϣ��
	BYTE flag_NewInf;//�е�������Ϣ
	BYTE InfNoRead;//δ����Ϣ����
	BYTE InfCount;//����Ϣ������
	BYTE CurInf;
	BYTE LineNO[16];//��·����
	BYTE StationCount;//��վ��
	BYTE StationCur;//��ǰվ���

	//ID��ʶ
	CString markID;

	BOOL bUsing;//CBusInfo�������ڱ�����
public:
	void SetRingInf(BYTE* pData,int nDataLen);//���ô�����Ϣ
	void SetDespathInf(BYTE* pData,int nDataLen);//���õ�����Ϣ

public:
	bool FormatRingTime(BYTE* pbuf,int i);
	bool Read4Record(BYTE bMonth,BYTE bDay,BYTE bHour,BYTE bMin,int nTag);
	bool Write4Record(BYTE bMonth,BYTE bDay,BYTE bHour,BYTE bMin,int nTag);
	bool Write4Send(BYTE* pBuf);
	bool Read4Send();



	//���ı��ļ���ȡ��������
	BOOL ReadFromTXT();
	//д��������ݵ��ı�
	BOOL WriteToTXT();
	//�Ա�ϵͳʱ�������ʱ��
	BOOL CompareTime(RingStruct*);
	//�洢����ʱ��
	RingStruct RingData[40];
	bool bTXTReaded;
	BOOL bTimeShowed;

	//�������
	BOOL bIsRecv;
};
