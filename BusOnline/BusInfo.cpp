#include "StdAfx.h"
#include "BusInfo.h"

CBusInfo::CBusInfo(void)
{
	for(int i=0;i<40;i++)
	{
		RingData[i].bMonth=0;
		RingData[i].bDay=0;
		RingData[i].bHour;
		RingData[i].bMinute;
		RingData[i].nRingNum=0;
		RingData[i].bValue=FALSE;
		RingData[i].bRinged=FALSE;
		RingData[i].bRinging=FALSE;
	}

	bUsing = FALSE;
	for (int i = 0; i < MAXINFONUM; ++i)
	{
		InfPacket[i].InfID="";
		InfPacket[i].InfClass="";
		InfPacket[i].InfState="";
		InfPacket[i].InfCmdType="";
		InfPacket[i].InfReplyNo="";
		InfPacket[i].InfTime="";
		InfPacket[i].InfContent="";
		InfPacket[i].bRead=true;
		InfPacket[i].bReply=true;
		InfPacket[i].bOK=false;
	}

	RingInf.InfID="";
	RingInf.InfClass="";
	RingInf.InfState="";
	RingInf.InfCmdType="";
	RingInf.InfReplyNo="";
	RingInf.InfTime="";
	RingInf.InfContent="";
	RingInf.bRead=true;
	RingInf.bReply=true;
	RingInf.bOK=false;

	iNoBufInf=0;//当前信息号
	flag_NewInf=false;//有调度新信息
	InfNoRead=0;//未读信息条数
	InfCount=0;//有信息总条数
	CurInf=0;//当前信息号
	memset(LineNO,NULL,16);
	LineNO[16];//线路名称
	StationCount=0;//总站数
	StationCur=0;//当前站点号


	bIsRecv = FALSE;

	ReadFromTXT();



	
}

CBusInfo::~CBusInfo(void)
{

}
void CBusInfo::SetRingInf(BYTE* pData,int nDataLen)
{
	if(pData[1]=='W' && pData[2]=='A' && pData[3]=='R')
	{
		CString strMessage=(CString)pData;
		strMessage=strMessage.Right(strMessage.GetLength()-5);//去掉"(WAR:"
		strMessage=strMessage.Left(strMessage.GetLength()-1);//去掉最后的")"

		RingInf.InfID=strMessage.Left(strMessage.Find(';'));//ID
		markID=RingInf.InfID;//ID标记
		strMessage=strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		RingInf.InfClass=strMessage.Left(strMessage.Find(';'));//分类
			strMessage=strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		RingInf.InfState=strMessage.Left(strMessage.Find(';'));//传输状态
		strMessage=strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		RingInf.InfCmdType=strMessage.Left(strMessage.Find(';'));//指令类型
			strMessage=strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));
		
		RingInf.InfReplyNo=strMessage.Left(strMessage.Find(';'));////是否回复
			strMessage=strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		RingInf.InfTime=strMessage.Left(strMessage.Find(';'));////是时间
			strMessage=strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

		RingInf.InfContent=strMessage;//信息

		RingInf.bRead=false;
		RingInf.bReply=false;
		RingInf.bOK=true;

	}
}
void CBusInfo::SetDespathInf(BYTE* pData,int nDataLen)//设置调度信息
{
	if(pData[1]=='D' && pData[2]=='P' && pData[3]=='H')
	{
		//(DPH:20121220111111001;00;00;11;00;41:00;全程[00][东山口--同德围])
			CString strMessage=(CString)pData;
			strMessage=strMessage.Right(strMessage.GetLength()-5);//去掉"(DPH:"
			strMessage=strMessage.Left(strMessage.GetLength()-1);//去掉最后的")"

			DispathInf.InfID = strMessage.Left(strMessage.Find(';'));
			markID=DispathInf.InfID;//ID标记
			strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

			DispathInf.InfClass = strMessage.Left(strMessage.Find(';'));
			strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

			DispathInf.InfState = strMessage.Left(strMessage.Find(';'));
			strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

			DispathInf.InfCmdType = strMessage.Left(strMessage.Find(';'));
			strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

			DispathInf.InfReplyNo = strMessage.Left(strMessage.Find(';'));
			strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));

			DispathInf.InfTime = strMessage.Left(strMessage.Find(';'));
			strMessage = strMessage.Right(strMessage.GetLength()-(strMessage.Find(';')+1));
			
			DispathInf.InfContent = strMessage;
	}
}



BOOL CBusInfo::ReadFromTXT()
{
	for(int i=0;i<40;i++)
	{
		RingData[i].bMonth=0;
		RingData[i].bDay=0;
		RingData[i].bHour;
		RingData[i].bMinute;
		RingData[i].nRingNum=0;
		RingData[i].bValue=FALSE;
		RingData[i].bRinged=FALSE;
		RingData[i].bRinging=FALSE;
	}
	FILE* fp = fopen("\\NAND\\RingData.txt","rb");
	if (fp!=NULL)
	{
		LONG lFileSize;
		fseek(fp, 0L, SEEK_END);
		lFileSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET); //将文件指针返回开头位置
		for(int i=0;i < lFileSize/6;i++)
		{
			fseek(fp, i*6L, SEEK_SET);
			BYTE pData[5];
			fread(&pData,1,5,fp);
			RingData[i].bMonth = pData[0];
			RingData[i].bDay = pData[1];
			RingData[i].bHour = pData[2];
			RingData[i].bMinute = pData[3];
			RingData[i].nRingNum = pData[4];
			RingData[i].bValue = TRUE;
			//TRACE(L" Read form txt is %x %x %x %x %x ",RingData[i].bMonth,RingData[i].bDay,RingData[i].bHour,RingData[i].bMinute,RingData[i].nRingNum);
		}
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

BOOL CBusInfo::WriteToTXT()
{
	FILE* fp=fopen("\\NAND\\RingData.txt","wb");
	BYTE pSign = 0xFF;
	for(int i=0;i<40;i++)
	{
		if(RingData[i].bValue)
		{
			fwrite(&RingData[i].bMonth,1,1,fp);
			fwrite(&RingData[i].bDay,1,1,fp);
			fwrite(&RingData[i].bHour,1,1,fp);
			fwrite(&RingData[i].bMinute,1,1,fp);
			fwrite(&RingData[i].nRingNum,1,1,fp);
			fwrite(&pSign,1,1,fp);
		}
	}
	fclose(fp);
	return TRUE;
}


//对比系统时间
BOOL CBusInfo::CompareTime(RingStruct* CurData)
{

	//0时0分即不打铃
	if (CurData->bHour ==0 && CurData->bMinute ==0)
	{
		return FALSE;
	}
	SYSTEMTIME sysTime;    
	GetLocalTime(&sysTime);

	int nMonth1,nMonth2,nDay1,nDay2;
	nMonth1 = (int)CurData->bMonth;
	nMonth2 = (int)sysTime.wMonth;
	nDay1 = (int)CurData->bDay;
	nDay2 = (int)sysTime.wDay;
	//if (CurData->bMonth == sysTime.wMonth && CurData->bDay == sysTime.wDay)
	if(nMonth1 == nMonth2 && nDay1 == nDay2)
	{
		int nTemp1 = CurData->bHour*60 + CurData->bMinute;
		int nTemp2 = sysTime.wHour*60+sysTime.wMinute;
		int nTemp3;
		if (nTemp1 > nTemp2)
		{
			nTemp3 = nTemp1 - nTemp2;
		}
		else
			nTemp3 = nTemp2 - nTemp1;
		//TRACE(L" &&&word is %d %d %d\n ",nTemp1,nTemp2,nTemp3);
		if (nTemp3 <= 3)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}
//-----------------------------------------------------------------------------
bool CBusInfo::FormatRingTime(BYTE* pbuf,int i)
{
	SYSTEMTIME RingTime;
	GetLocalTime(&RingTime);
	BYTE* pTemp = pbuf;
	if (i==1) //打铃已响
	{
		*(pTemp + 0) = '(';
		*(pTemp + 1) = 0x7E;
		*(pTemp + 2) = 0x7E;
		*(pTemp + 3) = 0x49;
		*(pTemp + 4) = 0x05;
		*(pTemp + 5) = 0x05;
		*(pTemp + 6) = 0x00;
		*(pTemp + 7) = (BYTE)(RingTime.wYear - 2000);
		*(pTemp + 8) = (BYTE)RingTime.wMonth;
		*(pTemp + 9) = (BYTE)RingTime.wDay;
		*(pTemp + 10) = (BYTE)RingTime.wHour;
		*(pTemp + 11) = (BYTE)RingTime.wMinute;
		WORD wSum = *(pTemp + 3) + *(pTemp + 4) + *(pTemp + 5) + *(pTemp + 6) + *(pTemp + 7) + *(pTemp + 8) + *(pTemp + 9) + *(pTemp + 10) + *(pTemp + 11);
		*(pTemp + 12) = LOBYTE(wSum);
		*(pTemp + 13) = HIBYTE(wSum);
		*(pTemp + 14) = 0x55;
		*(pTemp + 15) = ')';

	}
	else if (i==2) //打铃确认
	{
		*(pTemp + 0) = '(';
		*(pTemp + 1) = 0x7E;
		*(pTemp + 2) = 0x7E;
		*(pTemp + 3) = 0x49;
		*(pTemp + 4) = 0x06;
		*(pTemp + 5) = 0x05;
		*(pTemp + 6) = 0x00;
		*(pTemp + 7) = (BYTE)(RingTime.wYear - 2000);
		*(pTemp + 8) = (BYTE)RingTime.wMonth;
		*(pTemp + 9) = (BYTE)RingTime.wDay;
		*(pTemp + 10) = (BYTE)RingTime.wHour;
		*(pTemp + 11) = (BYTE)RingTime.wMinute;
		WORD wSum = *(pTemp + 3) + *(pTemp + 4) + *(pTemp + 5) + *(pTemp + 6) + *(pTemp + 7) + *(pTemp + 8) + *(pTemp + 9) + *(pTemp + 10) + *(pTemp + 11);
		*(pTemp + 12) = LOBYTE(wSum);
		*(pTemp + 13) = HIBYTE(wSum);
		*(pTemp + 14) = 0x55;
		*(pTemp + 15) = ')';
	}
	return true;
}




bool CBusInfo::Write4Send(BYTE* pBuf)
{
	BYTE* bTemp = pBuf;
	FILE* fp = fopen("\\NAND\\DataSend.txt","ab");
	fwrite(bTemp,1,16,fp);
	fclose(fp);
	return true;
}
bool CBusInfo::Read4Send()
{
	FILE* fp = fopen("\\NAND\\DataSend.txt","rb");
	if (fp != NULL)
	{
		int addCount = 0;
		LONG lFileSize;
		fseek(fp, 0L, SEEK_END);
		lFileSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET); //将文件指针返回开头位置
		for (int i = 0;i < lFileSize/16;i++)
		{
			fseek(fp, i*16L, SEEK_SET);
			BYTE bTemp[16];
			fread(&bTemp,1,16,fp);
			if (AddTCP_Data(bTemp,16,DBstart,DBend))
			{
				addCount++;
			}
		}
		if (addCount == lFileSize/16)
		{
			FILE* ffp = fopen("\\NAND\\DataSend.txt","wb");
			fclose(ffp);
		}
		fclose(fp);
		return true;
	}
	return false;
}
bool CBusInfo::Write4Record(BYTE bMonth,BYTE bDay,BYTE bHour,BYTE bMin,int nTag)
{
	FILE* fp = fopen("\\NAND\\Ring_Record.txt","ab+");
	BYTE bSign = 0x01;
	BYTE bSign2 = 0x02;
	BYTE tMonth,tDay,tHour,tMin;
	tMonth = bMonth;
	tDay = bDay;
	tHour = bHour;
	tMin = bMin;

	if(nTag == 1)
	{
		fwrite(&tMonth,1,1,fp);
		fwrite(&tDay,1,1,fp);
		fwrite(&tHour,1,1,fp);
		fwrite(&tMin,1,1,fp);
		fwrite(&bSign,1,1,fp);
	}
	else if(nTag ==2)
	{
		fwrite(&tMonth,1,1,fp);
		fwrite(&tDay,1,1,fp);
		fwrite(&tHour,1,1,fp);
		fwrite(&tMin,1,1,fp);
		fwrite(&bSign2,1,1,fp);
	}

	fclose(fp);
	return true;
}
bool CBusInfo::Read4Record(BYTE bMonth,BYTE bDay,BYTE bHour,BYTE bMin,int nTag)
{
	FILE* fp = fopen("\\NAND\\Ring_Record.txt","rb");
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	if(fp != NULL)
	{
		BYTE bMD[5] = {0,0,0,0,0};
		fread(&bMD,1,5,fp);
		if (sysTime.wMonth != bMD[0] || sysTime.wDay != bMD[1])
		{
			FILE* ffp = fopen("\\NAND\\Ring_Record.txt","wb");
			fclose(ffp);
			return true;
		}
		LONG lFileSize;
		fseek(fp,0L,SEEK_END);
		lFileSize = ftell(fp);
		fseek(fp,0L,SEEK_SET);
		for(int i = 0;i<lFileSize/5;i++)
		{
			fseek(fp,i*5L,SEEK_SET);
			BYTE bTemp[5] = {0,0,0,0,0};
			fread(&bTemp,1,5,fp);
			if(bTemp[2] == bHour && bTemp[3] == bMin && bTemp[4] == nTag)
			{
				fclose(fp);
				return false;
			}
		}
		fclose(fp);
	}
	return true;
}
