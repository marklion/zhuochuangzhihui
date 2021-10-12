#include <stdio.h>
#include "HWISPRTPrinter.h"
//include <libusb.h>

void main()
{

	//export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

	int ret = -1;

	//char *ipAddress  = "192.168.1.114"; //����,WX_20160811
	/*char ip[256];
	bzero(ip, 256);
	printf("Please Input IP: ");
	scanf("%s", ip);*/
	//2016.12.19 17:25 OK

	//char *dev  = "/dev/ttyS0"; //����1	ok 2016.12.20 11:22

	//setAttribute(USB_PORT);
	ret = OpenPort(NET_PORT, "192.168.0.108"); //2016.12.21 14:23 OK ʹ�û���POSIMP88VESP

	if (ret < 0)
	{
		printf("OpenPort() fail ret=%d\n", ret);
		return;
	}
	printf("OpenPort() ok ret=%d\n", ret);

	//������֧�֣�38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,19200,  9600, 4800, 2400, 1200,  300;
	/*Sprt_set_speed(115200);
	printf("Sprt_set_speed() ok \n");
  
	 //����8λ��У��??1λֹͣλ;
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
	// ��ʼ����ӡ��;
	if (ret < 0)
	{
		printf("POS_Control_ReSet() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_ReSet() ok ret=%d\n", ret);
	/*************************************************************************/
	int type = 4;							  // 1,��ѯǮ��״̬��2,��ѯֽ�ָ�״̬��3,��ѯ�е�״̬��4,��ѯȱֽ״??;
	ret = POS_Status_RTQueryTypeStatus(type); // ��ѯȱֽ״??;
	// 0:����,1:ȱֽ,2:ֽ�ָǿ�,3:�е�����,4:Ǯ��ߵ�??;
	// <0: ����??
	if (ret < 0)
	{
		printf("POS_Status_RTQueryTypeStatus() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	if (ret == 1) // ȱֽ??
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
	//0:�޲��ɻָ���??&û���Զ��ָ�����,1:�в��ɻָ���??,2:���Զ��ָ������??,3:���Զ��ָ������??&�в��ɻָ���??,-3ͨ��ʱ��ӡ�����ص�������??,-1��ʾ����ָ����??
	if (ret < 0)
	{
		printf("QueryErrState ERR,ret:%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("QueryErrState OK,status: %d\n", ret);

	//----------------2016.12.13.ztongli--------------------------

	// ���ô�ӡ��������;
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

	// ���ô�ӡ��������;
	ret = POS_Control_SetPrintFontC(isDoubleWidth, isDoubleHeight, isUnderLine);
	if (ret < 0)
	{
		printf("POS_Control_SetPrintFontC() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_SetPrintFontC() ok ret=%d\n", ret);

	char *p = "��һ�����ּ�����1234\n�ڶ������ּ�����567890\n";
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

	// �ָ����ô�ӡ��������;
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
	// �ָ����ô�ӡ��������;
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

	int align = 1; //0,����룬1,����??2,�Ҷ�??;
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

	// ��ӡQR��ά�룻
	int iType = 2; // iType=0 ѡ��PDF417,iType=1 ѡ��DATA MATRIX,iType=2 ѡ��QR CODE;
	int v = 0;	   // 0 ?? v ?? 40	��ʾͼ�ΰ汾??(0���Զ�ѡ��)??
	int r = 76;	   // 76,77,81,72	��ʾ����ȼ�(L:7%, M:15%,Q:25%,H:30%)??
	int k = 6;	   // 1 ?? k ?? 6	��ʾ����Ŵ���??
	char *dataStr = "QR���??";
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

	align = 0; //0,����룬1,����??2,�Ҷ�??;
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
	ret = POS_Control_SetPrintPosition(pos); // ������߾ࣻ
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

	// ��ӡһά���룻
	int m = 73; // ��������??0 ?? m ?? 6 ??65 ?? m ?? 73
	int w = 2;	// ����ĺ���ģ���ȣ�2 ?? w ?? 6 ��Ӧ��ͬ��mm,�磺2��Ӧ0.25??6��Ӧ0.75
	int h = 50; // �����������
	int n = 2;	// 0 ����??,1 �������Ϸ���??,2 �������·���??,3 �����ϡ��·�����ӡ;
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

	// ���Դ�ӡ??
	ret = POS_Control_OppositeColor(1);
	if (ret < 0)
	{
		printf("POS_Control_OppositeColor() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_OppositeColor() ok ret=%d\n", ret);
	p = "һ�з����ı�����\n";
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
	p = "һ�з����ı�����û�лس������Ȼ��棬����ӡ";
	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	ret = POS_Output_PrintBuffAndFeedLines(2); // ��ӡ���ݲ���ǰ��ֽ�����У��ַ��У���
	if (ret < 0)
	{
		printf("POS_Output_PrintBuffAndFeedLines() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintBuffAndFeedLines() ok ret=%d\n", ret);
	// �ָ����Դ�ӡ??
	ret = POS_Control_OppositeColor(0);
	if (ret < 0)
	{
		printf("POS_Control_OppositeColor() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_OppositeColor() ok ret=%d\n", ret);

	p = "�������з����ı����ݼ�����ӡ��\n";
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

	// ��ӡһ�кڰ�λͼ����������ٷ��ͻس����в��ܴ�??; �ù��ܿ���������??
	unsigned char bytes[256] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
								0x0d, 0x0a}; // ע��һ��Ҫ�ӻس���??;
	ret = POS_Output_PrintBitGraph(1, 10, bytes, 12);
	if (ret < 0)
	{
		printf("POS_Output_PrintBitGraph() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintBitGraph() ok ret=%d\n", ret);

	// ��ӡ1��ֺ���;
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
	// ����ʮ�������ֽڵ���ӡ����
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

	p = "��ӡ������ɣ�׼����ֽ\n";
	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);

	ret = POS_Control_CutPaper(0, 0); // ��ֽ??
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
