#pragma once
#define  MAXITEMNUM 100

typedef struct
{
	UINT nIndex;
	CString strContent;
}MenuStruct;

#define MaxItems_0 5
#define IndexFirst_0 0x00
#define IndexEnd_0 0x04

#define MaxItems_2 20
#define IndexFirst_2 0x20
#define IndexEnd_2 0x33

#define MaxItems_4 8
#define IndexFirst_4 0x40
#define IndexEnd_4 0x47

#define MaxItems_6 2
#define IndexFirst_6 0x60
#define IndexEnd_6 0x61

#define MaxItems_8 2
#define IndexFirst_8 0x80
#define IndexEnd_8 0x81

#define MaxItems_A 2
#define IndexFirst_A 0xA0
#define IndexEnd_A 0xA1

#define BOTTOM 0xFF
#define INVILID 0xEF

class CLCDMenu
{
public:
	CLCDMenu(void);
	~CLCDMenu(void);
public:
	UINT nIndex;//当前状态索引号
	UINT nMaxItems;//本级菜单最大条目数
	UINT nFirst;//本级菜单的开始索引
	UINT nEnd;//本级菜单的结束索引
	UINT nOk;//按下“确定”键时转向的状态索引号
	UINT nEsc;//按“返回”键时转向时状态索引号
	UINT nDown;//按下“向下”键时转向的状态索引号
	UINT nUp;//按下“向上”键时转向的状态索引号
	VOID (*CurrentOperate)();//当前状态应该执行的功能
public:
	 MenuStruct Menu[MAXITEMNUM];
	 void MenuInit();//初始化菜单
	 CString FindContentByIndex(UINT nIndex);
	 void LoadLCDMenu( UINT nIndex,BYTE* pLcdContent);//加载菜单
	 UINT FindIndexByContent(CString strContent);
	 void SetLCDMenuPara(UINT nIndex);
	 char* UnicodeStrToAnsiStr( CString strUni, int* pStrByteLen );
};
