#include "StdAfx.h"
#include "LCDMenu.h"

CLCDMenu::CLCDMenu(void)
{
	MenuInit();
}

CLCDMenu::~CLCDMenu(void)
{
}
void CLCDMenu::MenuInit()
{
	Menu[0].nIndex = 0x00;
	Menu[0].strContent = _T("固定信息");
	Menu[1].nIndex = 0x01;
	Menu[1].strContent = _T("调度信息");
	Menu[2].nIndex = 0x02;
	Menu[2].strContent = _T("司机考勤");
	Menu[3].nIndex = 0x03;
	Menu[3].strContent = _T("进出总站");
	Menu[4].nIndex = 0x04;
	Menu[4].strContent = _T("主机参数");

	Menu[5].nIndex = 0x20;
	Menu[5].strContent = _T("故障开始");
	Menu[6].nIndex = 0x21;
	Menu[6].strContent = _T("故障结束");
	Menu[7].nIndex = 0x22;
	Menu[7].strContent = _T("申请加气");
	Menu[8].nIndex = 0x23;
	Menu[8].strContent = _T("加气结束");
	Menu[9].nIndex = 0x24;
	Menu[9].strContent = _T("保养开始");
	Menu[10].nIndex = 0x25;
	Menu[10].strContent = _T("保养结束");
	Menu[11].nIndex = 0x26;
	Menu[11].strContent = _T("前方路堵");
	Menu[12].nIndex = 0x27;
	Menu[12].strContent = _T("司机吃饭");
	Menu[13].nIndex = 0x28;
	Menu[13].strContent = _T("服务纠纷");
	Menu[14].nIndex = 0x29;
	Menu[14].strContent = _T("交通事故");
	Menu[15].nIndex = 0x2A;
	Menu[15].strContent = _T("治安报警");
	Menu[16].nIndex = 0x2B;
	Menu[16].strContent = _T("包车开始");
	Menu[17].nIndex = 0x2C;
	Menu[17].strContent = _T("包车结束");
	Menu[18].nIndex = 0x2D;
	Menu[18].strContent = _T("交警指挥绕行");
	Menu[19].nIndex = 0x2E;
	Menu[19].strContent = _T("乘客不适");
	Menu[20].nIndex = 0x2F;
	Menu[20].strContent = _T("司机不适");
	Menu[21].nIndex = 0x30;
	Menu[21].strContent = _T("事故开始");
	Menu[22].nIndex = 0x31;
	Menu[22].strContent = _T("事故结束");
	Menu[23].nIndex = 0x32;
	Menu[23].strContent = _T("其他情况开始");
	Menu[24].nIndex = 0x33;
	Menu[24].strContent = _T("其他情况结束");

	Menu[25].nIndex = 0x60;
	Menu[25].strContent = _T("上班签到");
	Menu[26].nIndex = 0x61;
	Menu[26].strContent = _T("下班签退");

	Menu[31].nIndex = 0x80;
	Menu[31].strContent = _T("到达总站");
	Menu[32].nIndex = 0x81;
	Menu[32].strContent = _T("开出总站");

	Menu[33].nIndex = 0xA0;
	Menu[33].strContent = _T("主机编号");
	Menu[34].nIndex = 0xA1;
	Menu[34].strContent = _T("软件版本");
}
void CLCDMenu::LoadLCDMenu( UINT nIndex,BYTE* pLcdContent)//加载菜单
{
	CString strTemp;
	memset(pLcdContent,' ',64);
	int nByteLen;
	char * pChar;
	int nLen;

	//菜单标题
	if(nEsc == INVILID)//顶级菜单
	{
		strTemp = _T("功能表          ");
	}
	else
	{
		strTemp += FindContentByIndex(nEsc);
		strTemp += _T("             ");

	}
	nByteLen = strTemp.GetLength()*2;
	pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
	nLen = 16;
	memcpy( pLcdContent, pChar, nLen );


	if (nIndex >= IndexFirst_0 && nIndex <= IndexEnd_0)
	{
		strTemp = _T("确认            ");
	}
	else if (nIndex >= IndexFirst_2 && nIndex <= IndexEnd_A)
	{
		strTemp = _T("确认        返回");
	}
	else 
	{
		strTemp = _T("            返回");
	}

	nByteLen = strTemp.GetLength()*2;
	pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
	nLen = 16;
	memcpy( pLcdContent+3*16, pChar, nLen );


	strTemp = FindContentByIndex(nIndex);
	nByteLen = strTemp.GetLength()*2;
	if (nByteLen < 16)
	{
		for (int i = 0; i < (16-nByteLen)/2; ++i) 
		{
			strTemp =L" " + strTemp;
		}
	}
	strTemp += L"         ";
	nByteLen = strTemp.GetLength()*2;
	pChar = UnicodeStrToAnsiStr(strTemp, &nByteLen );
	nLen = 16;
	memcpy( pLcdContent+16, pChar, nLen );

	delete pChar;


}
CString CLCDMenu::FindContentByIndex(UINT nIndex)
{
	CString strContent = _T("");
	for (int i = 0; i < MAXITEMNUM; ++i)
	{
		if (Menu[i].nIndex == nIndex)
		{
			strContent = Menu[i].strContent;
			break;
		}
	}
	return strContent;
}
UINT CLCDMenu::FindIndexByContent(CString strContent)
{ 
	UINT nIndex = 0xFF;
	for (int i = 0; i < MAXITEMNUM; ++i)
	{
		if (Menu[i].strContent.Compare(strContent) == 0)
		{
			nIndex = Menu[i].nIndex;
		}
	}
	return nIndex;
}
void CLCDMenu::SetLCDMenuPara(UINT nIndex)
{
	this->nIndex = nIndex;
	if (nIndex >= IndexFirst_0 && nIndex <= IndexEnd_0)
	{
		nMaxItems = MaxItems_0;
		nFirst = IndexFirst_0;
		nEnd = IndexEnd_0;
		nOk = 0x20 * (nIndex + 1);
		nEsc = INVILID;
	}
	else if (nIndex >= IndexFirst_2 && nIndex <= IndexEnd_2)
	{
		nMaxItems = MaxItems_2;
		nFirst = IndexFirst_2;
		nEnd = IndexEnd_2;
		nOk = BOTTOM;
		nEsc = 0x00;
	}
	else if (nIndex >= IndexFirst_4 && nIndex <= IndexEnd_4)
	{
		nMaxItems = MaxItems_4;
		nFirst = IndexFirst_4;
		nEnd = IndexEnd_4;
		nOk = BOTTOM;
		nEsc = 0x01;
	}
	else if (nIndex >= IndexFirst_6 && nIndex <= IndexEnd_6)
	{
		nMaxItems = MaxItems_6;
		nFirst = IndexFirst_6;
		nEnd = IndexEnd_6;
		nOk = BOTTOM;
		nEsc = 0x02;
	}
	else if (nIndex >= IndexFirst_8 && nIndex <= IndexEnd_8)
	{
		nMaxItems = MaxItems_8;
		nFirst = IndexFirst_8;
		nEnd = IndexEnd_8;
		nOk = BOTTOM;
		nEsc = 0x03;
	}
	else if (nIndex >= IndexFirst_A && nIndex <= IndexEnd_A)
	{
		nMaxItems = MaxItems_A;
		nFirst = IndexFirst_A;
		nEnd = IndexEnd_A;
		nOk = BOTTOM;
		nEsc = 0x04;
	}
//上下键的索引
	if (nIndex == nFirst)
		nUp = nEnd;
	else
		nUp = nIndex - 1;

	if (nIndex == nEnd)
		nDown= nFirst;
	else
		nDown = nIndex + 1;

	if (nIndex == BOTTOM)
	{
		nUp = INVILID;
		nDown =INVILID;
		nOk = INVILID;
	}


}

char * CLCDMenu::UnicodeStrToAnsiStr( CString strUni, int* pStrByteLen )
{
	int nUniLen = *pStrByteLen/2;
	unsigned short  *pUniData = new unsigned short[nUniLen];
	for( int i=0; i<nUniLen; i++)
	{
		if( i < strUni.GetLength() )
			*(pUniData+i) = strUni[i];
	}

	int nAnsiLen = WideCharToMultiByte( CP_ACP, NULL, (LPCWSTR)pUniData, nUniLen, NULL, 0, NULL, NULL );
	char  *pAnsiData = new char[nAnsiLen];
	WideCharToMultiByte( CP_ACP, NULL, (LPCWSTR)pUniData, nUniLen, pAnsiData, nAnsiLen, NULL, NULL );

	delete pUniData;

	*pStrByteLen = nAnsiLen;

	return pAnsiData;
}