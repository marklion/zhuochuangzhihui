#include <stdio.h>
#include "HWISPRTPrinter.h"
//include <libusb.h>

void main()
{

	//export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

	int ret = -1;

	//char *ipAddress  = "192.168.1.114"; //网口,WX_20160811
	/*char ip[256];
	bzero(ip, 256);
	printf("Please Input IP: ");
	scanf("%s", ip);*/
	//2016.12.19 17:25 OK

	//char *dev  = "/dev/ttyS0"; //串口1	ok 2016.12.20 11:22

	//setAttribute(USB_PORT);
	ret = OpenPort(NET_PORT, "192.168.0.108"); //2016.12.21 14:23 OK 使用机器POSIMP88VESP

	if (ret < 0)
	{
		printf("OpenPort() fail ret=%d\n", ret);
		return;
	}
	printf("OpenPort() ok ret=%d\n", ret);

	//波特率支持：38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,19200,  9600, 4800, 2400, 1200,  300;
	/*Sprt_set_speed(115200);
	printf("Sprt_set_speed() ok \n");
  
	 //设置8位不校验??1位停止位;
	ret=Sprt_set_parity(8,1,'N');
	if( ret<0)
	{	
		printf("Sprt_set_Parity() fail ret=%d\n",ret);
		ret=ClosePort();
		printf("ClosePort() finished, ret=%d\n",ret);
		
		return;
	}
	printf("Sprt_set_Parity() ok ret=%d\n",ret);*/
	/*************************************************************************/
	// 初始化打印机;
	if (ret < 0)
	{
		printf("POS_Control_ReSet() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_ReSet() ok ret=%d\n", ret);
	/*************************************************************************/
	int type = 4;							  // 1,查询钱箱状态；2,查询纸仓盖状态；3,查询切刀状态；4,查询缺纸状??;
	ret = POS_Status_RTQueryTypeStatus(type); // 查询缺纸状??;
	// 0:正常,1:缺纸,2:纸仓盖开,3:切刀错误,4:钱箱高电??;
	// <0: 错误??
	if (ret < 0)
	{
		printf("POS_Status_RTQueryTypeStatus() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	if (ret == 1) // 缺纸??
	{
		printf("POS_Status_RTQueryTypeStatus() no paper!!! ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Status_RTQueryTypeStatus() ok ret=%d\n", ret);

	/*************************************************************************/

	//----------------2016.12.13.ztongli--------------------------
	ret = QueryErrState();
	//0:无不可恢复错??&没有自动恢复错误,1:有不可恢复错??,2:有自动恢复错误出??,3:有自动恢复错误出??&有不可恢复错??,-3通信时打印机返回的数据有??,-1表示发送指令有??
	if (ret < 0)
	{
		printf("QueryErrState ERR,ret:%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("QueryErrState OK,status: %d\n", ret);

	//----------------2016.12.13.ztongli--------------------------

	// 设置打印西文字体;
	int isFont = 1;
	int isBold = 1;
	int isDoubleWidth = 1;
	int isDoubleHeight = 1;
	int isUnderLine = 1;
	ret = POS_Control_SetPrintFontE(isFont, isBold, isDoubleWidth, isDoubleHeight, isUnderLine);
	if (ret < 0)
	{
		printf("POS_Control_SetPrintFontE() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_SetPrintFontE() ok ret=%d\n", ret);

	// 设置打印汉字字体;
	ret = POS_Control_SetPrintFontC(isDoubleWidth, isDoubleHeight, isUnderLine);
	if (ret < 0)
	{
		printf("POS_Control_SetPrintFontC() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_SetPrintFontC() ok ret=%d\n", ret);

	char *p = "第一行文字加数字1234\n第二行文字加数字567890\n";
	int len = strlen(p);
	//	ret=Sprt_serial_write(p,len);
	//	printf("ret=%d\n",ret);

	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	/*************************************************************************/

	// 恢复设置打印西文字体;
	isFont = 1;
	isBold = 0;
	isDoubleWidth = 0;
	isDoubleHeight = 0;
	isUnderLine = 0;
	ret = POS_Control_SetPrintFontE(isFont, isBold, isDoubleWidth, isDoubleHeight, isUnderLine);
	if (ret < 0)
	{
		printf("POS_Control_SetPrintFontE()2 fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_SetPrintFontE()2 ok ret=%d\n", ret);
	// 恢复设置打印汉字字体;
	ret = POS_Control_SetPrintFontC(isDoubleWidth, isDoubleHeight, isUnderLine);
	if (ret < 0)
	{
		printf("POS_Control_SetPrintFontC()2 fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_SetPrintFontC()2 ok ret=%d\n", ret);

	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	/*************************************************************************/

	int align = 1; //0,左对齐，1,居中??2,右对??;
	ret = POS_Control_AlignType(align);
	if (ret < 0)
	{
		printf("POS_Control_AlignType() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_AlignType() ok ret=%d\n", ret);

	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	/*************************************************************************/

	// 打印QR二维码；
	int iType = 2; // iType=0 选择PDF417,iType=1 选择DATA MATRIX,iType=2 选择QR CODE;
	int v = 0;	   // 0 ?? v ?? 40	表示图形版本??(0：自动选择)??
	int r = 76;	   // 76,77,81,72	表示纠错等级(L:7%, M:15%,Q:25%,H:30%)??
	int k = 6;	   // 1 ?? k ?? 6	表示纵向放大倍数??
	char *dataStr = "QR码测??";
	ret = POS_Output_PrintBar2code(iType, v, r, k, dataStr);
	if (ret < 0)
	{
		printf("POS_Output_PrintBar2code() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintBar2code() ok ret=%d\n", ret);
	/*************************************************************************/

	align = 0; //0,左对齐，1,居中??2,右对??;
	ret = POS_Control_AlignType(align);
	if (ret < 0)
	{
		printf("POS_Control_AlignType() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_AlignType() ok ret=%d\n", ret);

	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	/*************************************************************************/

	int pos = 30;
	ret = POS_Control_SetPrintPosition(pos); // 设置左边距；
	if (ret < 0)
	{
		printf("POS_Control_SetPrintPosition() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_SetPrintPosition() ok ret=%d\n", ret);

	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	/*************************************************************************/

	// 打印一维条码；
	int m = 73; // 条码类型??0 ?? m ?? 6 ??65 ?? m ?? 73
	int w = 2;	// 条码的横向模块宽度：2 ?? w ?? 6 对应不同的mm,如：2对应0.25??6对应0.75
	int h = 50; // 条码纵向点数
	int n = 2;	// 0 不打??,1 在条码上方打??,2 在条码下方打??,3 条码上、下方都打印;
	char *hriTextStr = "{BNo.123456";
	ret = POS_Output_PrintBar1code(m, w, h, n, hriTextStr);
	if (ret < 0)
	{
		printf("POS_Output_PrintBar1code() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintBar1code() ok ret=%d\n", ret);

	/*************************************************************************/

	// 反显打印??
	ret = POS_Control_OppositeColor(1);
	if (ret < 0)
	{
		printf("POS_Control_OppositeColor() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_OppositeColor() ok ret=%d\n", ret);
	p = "一行反显文本数据\n";
	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);

	/*************************************************************************/
	p = "一行反显文本数据没有回车换行先缓存，不打印";
	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	ret = POS_Output_PrintBuffAndFeedLines(2); // 打印数据并向前走纸若干行（字符行）；
	if (ret < 0)
	{
		printf("POS_Output_PrintBuffAndFeedLines() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintBuffAndFeedLines() ok ret=%d\n", ret);
	// 恢复反显打印??
	ret = POS_Control_OppositeColor(0);
	if (ret < 0)
	{
		printf("POS_Control_OppositeColor() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_OppositeColor() ok ret=%d\n", ret);

	p = "走了两行反显文本数据继续打印。\n";
	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	/*************************************************************************/

	// 打印一行黑白位图，后面必须再发送回车换行才能打??; 该功能可用作画线??
	unsigned char bytes[256] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
								0x0d, 0x0a}; // 注意一定要加回车换??;
	ret = POS_Output_PrintBitGraph(1, 10, bytes, 12);
	if (ret < 0)
	{
		printf("POS_Output_PrintBitGraph() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintBitGraph() ok ret=%d\n", ret);

	// 打印1点粗横线;
	unsigned char lineDots[576 + 2] = {0};
	int i = 0;
	for (i = 0; i < (sizeof(lineDots) - 2); i++)
	{
		lineDots[i] = 0x08;
	}
	lineDots[i++] = 0x0d;
	lineDots[i++] = 0x0a;
	printf("POS_Output_PrintBitGraph() i=%d\n", i);
	ret = POS_Output_PrintBitGraph(1, i, lineDots, i);
	if (ret < 0)
	{
		printf("POS_Output_PrintBitGraph() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintBitGraph() ok ret=%d\n", ret);

	/*************************************************************************/
	// 发送十六进制字节到打印机；
	unsigned char data[14] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x0d, 0x0a};
	ret = POS_Output_PrintData(data, 12);
	if (ret < 0)
	{
		printf("POS_Output_PrintData() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintData() ok ret=%d\n", ret);
	/*************************************************************************/

	p = "打印测试完成，准备切纸\n";
	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);

	ret = POS_Control_CutPaper(0, 0); // 切纸??
	if (ret < 0)
	{
		printf("POS_Control_CutPaper() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_CutPaper() ok ret=%d\n", ret);

	//---------------------------------------------------------
	ret = ClosePort();
	//ret=ClosePort();
	if (ret < 0)
	{
		printf("ClosePort() fail ret=%d\n", ret);
		return;
	}
	else
	{
		printf("ClosePort() ok ret=%d\n", ret);
	}
}
