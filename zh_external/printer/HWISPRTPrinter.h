
/************************************************************************
 *
 * SPRT printer SO for linux;
 * Created by qzfeng 2015/09/09
 *
 ***********************************************************************/


#ifndef HWISPRTPRINTER_H
#define HWISPRTPRINTER_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>
#include <string.h>
//#include <libusb-1.0/libusb.h>
#include <pthread.h> 
#include <stdbool.h>
#include <errno.h>

#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
/*
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
*/

//----------------ztongli.2016.12.19-------------------
//通用接口整合的结构体
typedef struct AttributeList
{
	int flag;
	unsigned char* cmdBuf;
	int bufLen;
	int sleepTime;
}AL;
//使用宏来确定打开端口
#define USB_PORT    0
#define SERIAL_PORT 1
#define NET_PORT	2

//----------------ztongli.2016.12.19-------------------



#define VID 0x0483          // usb 参数;
#define PID 0x5720          // usb 参数;
#define SERVER_PORT 9100    // 网口端口参数，WX_20160811


// 打印机发送指令缓冲区及发送指令长度（字节）;
extern unsigned char gCmdBuf[1024];
extern int gCmdBufLen;
// 打印机接收缓冲区及接收返回字节长度;
extern unsigned char gRcvBuf[1024];
extern int gRcvBufLen;

extern int socketFd;	                       //网口句柄，WX_20160811

////////////////////////////////网口函数声明,WX_20160811,Begin///////////////////////////////////
// 网口开口函数; 返回值: 0 成功，其他失败；
int Sprt_net_open(char *ipAddress);

// 网口关口函数; 返回值: 0 成功，其他失败；
int Sprt_net_close();

// 网口接收函数; pBufLen为期望接收到的字节个数; 成功：返回值为实际读取的字节数,其他失败;
int Sprt_net_read(unsigned char *pBuf,int pBufLen);

// 网口发送函数; 成功：返回0，其他失败;
int Sprt_net_write(unsigned char *pBuf,int pBufLen);
////////////////////////////////////网口函数声明,WX_20160811,End/////////////////////////////////


// 发送函数; 成功：返回值等于pBufLen，其他失败;
int Sprt_usb_write(unsigned char *pBuf,int pBufLen,int sleepTime);
// 接收函数; pBufLen为期望接收到的字节个数; 成功：返回值为实际读取的字节数,其他失败;
int Sprt_usb_read(unsigned char *pBuf,int pBufLen,int sleepTime);

// 初始化全局变量;
void InitGlobalVar();
// 开口函数; 返回值: 0 成功，其他失败；
int Sprt_usb_open();
// 关口函数; 返回值: 0 成功，其他失败；
int Sprt_usb_close();

#define WRITEBYTEMAX			1024					// 一次最大发送usb字节数;

extern pthread_mutex_t gPUsb_mutex;						// usb锁;
extern pthread_mutex_t gPLog_mutex;						// 日志锁;

#define LOGFILE			"sprtLog.log"
#define MAXLOGFILESIZE		1024*1024					// 最大日志大小
void DoLog(unsigned char *pLogMsg,int logMsgLen);		// 日志记录函数; 
extern unsigned char gLogBuf[1024];		// 日志记录缓冲区;



extern int serialFd;					// 串口句柄; qzfeng 2016/05/11
// 串口开口函数; 返回值: 0 成功，其他失败；
// Dev:设备文件路径名；如：/dev/ttyS0;
int Sprt_serial_open(char *Dev);

// 串口关口函数; 返回值: 0 成功，其他失败；
int Sprt_serial_close();

// 串口接收函数; pBufLen为期望接收到的字节个数; 成功：返回值为实际读取的字节数,其他失败;
int Sprt_serial_read(unsigned char *pBuf,int pBufLen,int sleepTime);

// 串口发送函数; 成功：返回值等于pBufLen，其他失败;
int Sprt_serial_write(unsigned char *pBuf,int pBufLen);

/**
*设置串口通信速率
*speed  类型 int  串口速度,可取值：
* 38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,19200,  9600, 4800, 2400, 1200,  300；
*/
void Sprt_set_speed(int speed);

/**
*	设置串口数据位，停止位和效验位
*	databits 类型  int 数据位   取值 为 7 或者8
*	stopbits 类型  int 停止位   取值为 1 或者2
*	parity  类型  int  效验类型 取值为N,E,O,,S
* 函数返回：<0 失败；=0成功；
*/
int Sprt_set_parity(int databits,int stopbits,int parity);



// 接口要求实现的函数;
/*****************************************************************************************************/

/* 
 * 功能：打印机切纸;；
 * 入口参数：
 * 					m:切纸方式, 0:全切,1:半切;
 *					n：切纸前走纸n点行,n=0为缺省3点;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_CutPaper(int m,int n);


/* 
 * 功能：打印机进纸（按点）;
 * 入口参数：
 * 					lineDots：进纸点数;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_FeedLines(int lineDots);

/* 
 * 功能：初始化打印机状态，清空打印缓冲区 ESC @;
 * 入口参数：
 * 					无;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_ReSet();

/* 
 * 功能：设置左边距 GS L nl nH （设置成[( nL + nH × 256)  × 横向移动单位)]英寸）;
 * 入口参数：
 * 					pos：距离;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_SetPrintPosition(int pos);

/* 
 * 功能：打印机进纸（按点）;
 * 入口参数：
 * 					lineDots：进纸点数;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_FeedLines(int lineDots);

/* 
 * 功能：对齐方式 ESC a n;
 * 入口参数：
 * 					align：0,左对齐，1,居中，2,右对齐;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_AlignType(int align);
/* 
 * 功能：打印数据并向前走纸若干行 ESC d n;
 * 入口参数：
 * 					lines：行数;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Output_PrintBuffAndFeedLines(int lines);

/* 
 * 功能：按类型实时查询打印机状态；
 * 入口参数：
 * 					type：1,查询钱箱状态；2,查询纸仓盖状态；3,查询切刀状态；4,查询缺纸状态;
 * 出口参数：
 *					无;
 * 返回值：
 *					0:正常,1:缺纸,2:纸仓盖开,3:切刀错误,4:钱箱高电平;
 *					<0: 错误；
 *
 */
int POS_Status_RTQueryTypeStatus(int type);

/* 
 * 功能：设置打印西文字体;
 * 入口参数：
 * 					isFont：0,压缩字体(9x17)；1,正常字体(12x24);
 * 					isBold：0,正常；1,加粗;
 * 									加粗模式对字符和汉字都有效，除加粗模式外，其他模式只对字符有效。
 * 					isDoubleWidth：0,正常；1,倍宽;
 * 					isDoubleHeight：0,正常；1,倍高;
 * 					isUnderLine：0,正常；1,有下划线;
 *					
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_SetPrintFontE( int isFont,
				    									 int isBold,
				    									 int isDoubleWidth,
				    									 int isDoubleHeight,
				    									 int isUnderLine
				    								 );

/* 
 * 功能：设置汉字字体;
 * 入口参数：
 * 					isDoubleWidth：0,正常；1,倍宽;
 * 					isDoubleHeight：0,正常；1,倍高;
 * 					isUnderLine：0,正常；1,有下划线;
 *					
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_SetPrintFontC( int isDoubleWidth,
    									 				 int isDoubleHeight,
    									 				 int isUnderLine
				    								 );
				    								 
				    								 
/* 
 * 功能：打印二维条码;
 * 入口参数：
 * 		条码类型：  n=0 选择PDF417,n=1 选择DATA MATRIX,n=2 选择QR CODE;
 * 					v：
 *					r：
 * 					各参数含义和范围根据GS Z选择的二维码类型不同而不同。
 * 					参数v, r不同的条码，其参数含义不同。
 *					① PDF417二维条码
 *					1 ≤ v ≤ 30	表示每行字符数。不同的机型由于纸宽不同，v的最大值应该在该机型允许的最大值之内。
 *					0 ≤ r ≤ 8	表示纠错等级。
 *					② DATA MATRIX二维条码
 *					0 ≤v ≤ 144	表示图形高(0：自动选择)。
 *					8 ≤ r ≤ 144	表示图形宽(v=0时，无效)。
 *					③ QR CODE二维条码
 *					0 ≤ v ≤ 40	表示图形版本号(0：自动选择)。
 *					r =76,77,81,72	表示纠错等级(L:7%, M:15%,Q:25%,H:30%)。
 *
 * 					k：1 ≤ k ≤ 6	表示纵向放大倍数。
 * 					dataStr:表示条码数据
 *					
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Output_PrintBar2code(int iType ,
    									 int v,
    									 int r,
    									 int k,
    									 unsigned char *dataStr
    									 );				    								 
				    								 
/* 
 * 功能：控制钱箱 ESC p m t1 t2;
 * 入口参数：
 * 					m：钱箱号 0 表示钱箱输出插座引脚2， 1 表示钱箱输出插座引脚5;
 * 					t1：脉冲开启的时间为 [ t1 × 2 ms];
 * 					t2：脉冲关闭的时间为 [ t2 × 2 ms];
 * 			  			读入t1、t2,确定本次开钱箱的脉冲宽度;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_CashDraw(int m,int t1,int t2);
		
/* 
 * 功能：选择/取消黑白反显打印模式;
 * 入口参数：
 * 					n：0 取消反显打印,1 选择反显打印;
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Control_OppositeColor(int n);
		
/* 
 * 功能：一维条码打印;
 * 入口参数：
 * 			m：条码类型，0 ≤ m ≤ 6 或65 ≤ m ≤ 73 ,详见打印机说明书;
 * 			w：宽度,条码的横向模块宽度：2 ≤ w ≤ 6 对应不同的mm,如：2对应0.25，6对应0.75
 * 			h：高度, 条码纵向点数;
 *			n：文字位置; 0 不打印,1 在条码上方打印,2 在条码下方打印,3 条码上、下方都打印;
 * 			hriTextStr：文本信息;
 * 出口参数：
 *			无;
 * 返回值：
 *			0: 正常;
 *			<0: 错误；
 *
 */
int POS_Output_PrintBar1code(int m,int w,int h,int n,unsigned char  *hriTextStr);
				    								 
/* 
 * 功能：打印一行黑白位图，后面必须再发送回车换行才能打印;该功能可用作画线；
 * 入口参数：
 * 		m:位图模式,0:8点单密度, 1: 8点双密度,32:24点单密度, 33:24点双密度;可选择固定为0；
 * 		n：指令要求位图数据长度(纵向排列，一行一行打印);是横向的点数；如8点就是如果10个字节就是10，如果是24点就是如果是30个字节就是10;
 * 		bytes：位图数据;
 *		bmpByteLength:位图数据字节数；
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Output_PrintBitGraph(int m,int n,unsigned char *bmpByte,int bmpByteLength);

/* 
 * 功能：打印用户磁盘上的图像文件； 暂不支持！！！
 * 入口参数：

 *					imagePath: 图像文件路径（若只有文件名则使用当前路径，若指定全路径则使用指定的路径）; 
 * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0:其他错误；
 * 说明：
 *					根据指定的文件路径读取文件，解析出图像信息并打印;
 *					仅支持单色位图文件;
 *					
 */
int POS_Output_PrintBmpDirect(unsigned char *imagePath);

/* 
 * 功能：发送给打印机一行字符串;
 * 入口参数：
 * 		printText:字符串以'\0'结尾；注意需以回车换行字符为结束才能立即打印；
  * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Output_PrintString(unsigned char *printText);

/* 
 * 功能：发送给打印机字节数组;
 * 入口参数：
 * 		printByte:字节数组；
 *    printByteSize:字节数组大小；
  * 出口参数：
 *					无;
 * 返回值：
 *					0: 正常;
 *					<0: 错误；
 *
 */
int POS_Output_PrintData(unsigned char *printByte, int printByteSize);

////////////////////////////////////WX_20160908,begin////////////////////////////////////
//ESC/POS增加的接口函数:

/*
	* 功能：旋转打印
	* 入口参数：n(0,不旋转；1,旋转)
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Control_SetRotaryPrint(int n);

/*
	* 功能：倒置打印
	* 入口参数：n(0,不倒置打印；1,倒置打印)
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Control_SetInvertedPrint(int n);

/*
	* 功能：打印预下载位图
	* 入口参数：n(位图号:0——7)
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Output_PrintFlashBmp(int n);

/*
	* 功能：打印测试页
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Control_PrintTestpage();

/*
	* 功能：设置行高
	* 入口参数：n(行高)
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Control_SetLineSpace(int n);

/*
	* 功能：黑标定位
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Control_BlackMark();

/*
	* 功能：打印本地文档
	* 入口参数：lpFilePath(本地文档路径)
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Output_SendLocalFileA(unsigned char *lpFilePath);//未实现，先留个接口

/*
	* 功能：查询打印任务状态
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Status_QueryTaskStatus();

/*
	* 功能：查询打印机ID号
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int POS_Input_PrinterId();

//CPCL增加的接口函数:

/*
	* 功能：打印
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_Print();

/*
	* 功能：找缝隙
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_Form();

/*
	* 功能：打印PDF417二维码
	* 入口参数：
*			x:条码水平方向起始位置;
*			y:条码垂直方向起始位置;
*			XDn: 最小单元横向宽度，范围：1到32，默认：2
*			YDn: 最小单元纵向高度，范围：1到32，默认：6
*			Cn:每行字符数。范围：1到30，默认：3
*			Sn:纠错等级。范围：1到8，默认：1;
*			data: 条码数据;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_Draw2Barcode_PDF417(int x, int y, int XDn, int YDn, int Cn, int Sn, char *data);

/*
	* 功能：打印QR二维码
	* 入口参数：
*			x:条码水平方向起始位置;
*			y:条码垂直方向起始位置;
*			Mn: QR条码模式.n范围：1或者2，默认：2
*			Un: 放大倍数.n范围：1到6，默认：6
*			Sn:纠错等级:
*				H-超高可靠性等级(Level H)
*				Q-高可靠性等级(Level Q)
*				M-标准等级(Level M)
*				L-高密度等级(Level L)
*			data: 条码数据;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_Draw2Barcode_QR(int x, int y, int Mn, int Un, char *Sn, char *data);

/*
	* 功能：打印条码
	* 入口参数：
*      value: 0:打印横向条码,1:打印纵向条码;
*       type:条码类型，一维条码的类型有:
*				Type值	条码类型
*				UPCA	UPC-A
*				UPCE	UPC-E
*				EAN13	JAN13 (EAN13)
*				EAN8	JAN 8 (EAN8)
*				39		CODE39
*				CODABAR	CODABAR
*				93		CODE93
*				128		CODE128(Auto)
*			width: 条码窄条宽宽度.
*			ratio: 宽条宽和窄条宽比率,范围：0 —— 30
*			height:条码高度
*			x:条码水平方向起始位置;
*			y:条码垂直方向起始位置;
*			data: 条码数据;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_DrawBarcode(int value,char *type,int width,int ratio,int height,int x, int y, char *data);

/*
	* 功能：图框命令
	* 入口参数：
*			x0:顶部左角X坐标;
*			y0:顶部左角Y坐标;
*			x1: 底部右角X坐标
*			y1: 底部右角Y坐标
*			width:图框宽度
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_DrawBox(int x0,int y0,int x1,int y1,int width);

/*
	* 功能：画线命令
	* 入口参数：
*     x0::左上角的横（x）坐标;
*			y0:左上角的纵（y）坐标;
*			x1:右上角水平的横（x）坐标
*			y1:右上角水平的纵（y）坐标
*			width:线条宽度
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_DrawLine(int x0,int y0,int x1,int y1,int width);

/*
	* 功能：文本命令
	* 入口参数：
*           value: 0:水平,1:逆时针旋转90°,2:逆时针旋转180°,3:逆时针旋转270°;
*           font:选择字体号:
*				24:字符(12*24)，汉字(24*24);
*				55:字符(8*16)，汉字(16*16)
*			size:	字符高度选择	字符宽度选择
*					size	纵向放大	size	横向放大
*					0		1（正常）	0		1 （正常）
*					1		2（2倍高）	10		2 （2倍宽）
*					2		3			20		3
*					3		4			30		4
*					4		5			40		5
*					5		6			50		6
*					6		7			60		7
*					7		8			70		8
*			x:水平打印起始位置;
*			y:垂直打印起始位置
*			data:打印的文本内容
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_DrawText(int value,int font,int size,int x,int y,char *data);

/*
	* 功能：反显区域(以画线的方式，线所经过的区域取反处理)
	* 入口参数：
*			x0:左上角X坐标;
*			y0:左上角Y坐标;
*			x1: 右上角的水平X坐标
*			y1: 右上角的水平Y坐标
*			width:反显内容宽度
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_InverseLine(int x0,int y0,int x1,int y1,int width);

/*
	* 功能：设置标签大小等参数
	* 入口参数：
*           offset: 相对机头零点的偏移量；
*           height: 标签最大高度，点为单位;
*			 qty:打印标签的数量;
*           width: 标签最大宽度，点为单位，目前缺省576点即72毫米；主要是为了德邦旋转打印，通常不用设置，"0"或NULL为不设置，
*					但如果需要旋转，则必须设置该参数，否则旋转不起作用；
*
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_PageSetup(int offset,int height,int qty, int width);

/*
	* 功能：设置当前行内容对齐方式
	* 入口参数：
*			align:0：居左，1：居中，2：居右;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_SetAlign(int align);

/*
	* 功能：设置字体加粗
	* 入口参数：
*			value:1:加粗,0:取消;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_SetBold(int value);

/*
	* 功能：HRI字符命令（设置一维条码文字自动打印）
	* 入口参数：
*   offset:HRI字符与条码的上下相对位移;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_SetHRI(char *offset);

/*
	* 功能：设置反白显示字符
	* 入口参数：
*			value:表示是否反白显示字符,1：反白显示,0：取消反白显示;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_SetInverseText(int value);

/*
	* 功能：整页顺时针旋转90°命令；调用labelPrn_pageSetup()函数，需设置宽度参数，否则不起作用
	* 入口参数：
*			value:1：旋转，0：不旋转;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_SetPageRoate(int value);

/*
	* 功能：设置字符/汉字间距
	* 入口参数：
*			spacing:字符之间的距离，默认字符间距是0;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_SetSpaceing(int spacing);

/*
	* 功能：设置是否打印下划线
	* 入口参数：
*			value:1：打印1点下划线，2：打印2点下划线，0：取消打印下划线;
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_SetUnderLineText(int value);

/*
	* 功能：打印单色位图
	* 入口参数：
*			type:0:打印扩展图形,1:打印压缩图形
*			x: 水平起始位置
*			y:垂直起始位置
*			lpFilePath:图片路径和文件名
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int CPCL_PrintBMP(int type,int x,int y,char *lpFilePath);

//TSPL增加的接口函数:
/*
	* 功能：清除缓冲区内容
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_CLS();

/*
	* 功能：设置标签纸宽度和高度
	* 入口参数：
*			pageWidth: 标签纸宽度
*			pageHeight:标签纸高度
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_PageSetup(int pageWidth,int pageHeight);

/*
	* 功能：画线
	* 入口参数：
*			x:左上角水平方向起点，以点（dot）表示(不可超过标签宽度)
*			y:左上角垂直方向起点，以点（dot）表示(不可超过标签高度)
			lineLength:线宽，以点（dot）表示
			lineHeight:线高，以点（dot）表示 
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_DrawLine(int x, int y, int lineLength, int lineHeight);

/*
	* 功能：打印矩形框体
	* 入口参数：
				x_start:矩形水平方向左上角x起始位置以点（dot）表示
				y_start:矩形垂直方向左上角y起始位置以点（dot）表示
				x_end:矩形水平方向右下角x结束位置以点（dot）表示
				y_end:矩形垂直方向右下角y结束位置以点（dot）表示
				Linethickness:矩形线宽，以点（dot）表示
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_DrawBorder(int x_start, int y_start, int x_end, int y_end, int Linethickness);

/*
	* 功能：画圆
	* 入口参数：
				x:圆心X坐标(dot)		
				y:圆心Y坐标(dot)
				diameter:圆的直径大小(dot)
				thickness:圆线的宽度(dot)
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_DrawCircle(int x, int y, int diameter, int thickness);

/*
	* 功能：打印文字
	* 入口参数：
				x:文字 X 方向起始点坐标(dot)		
				y:文字 Y 方向起始点坐标(dot)
				font:0(简体中文 24×24Font),1(繁体中文 24×24Font)
				rotation:文字旋转角度（顺时针方向）
								0           0 度
							 90           90 度
							180           180 度
							270           270 度
				xMultiplication:X 方向放大倍率 1——4
				yMultiplication:Y 方向放大倍率 1——4
				content:要打印的文字内容
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_PrintText(int x, int y, int font, int rotation, int xMultiplication, int yMultiplication, char *content);

/*
	* 功能：打印一维码
	* 入口参数：
				x:欲绘制条码的水平坐标左上角起点，以点（dot）表示		
				y:欲绘制条码的垂直坐标左上角起点，以点（dot）表示
				type:条码类型
						128	  Code 128,条形码subset采用自动选择A,B,C
				 EAN128 	EAN 128
             39 	Auto switch full ASCII and Standard code 39 for plus models.
             93 	Code 93
          EAN13 	EAN13
           EAN8 	EAN 8
           CODA 	Codabar
           UPCA	  UPCA
         UPCE+5 	UPC-E with 5 digits add-on
				height:条形码高度，以点（dot）表示
 humanreadable:
							0，人眼不可识
              1，人眼可识
      rotation:条形码旋转角度，顺时针方向
      				0            不旋转
             90            顺时针防线旋转90度
            180           顺时针防线旋转180度
            270           顺时针防线旋转270度
        narrow:窄条宽度,以点（dot）表示,1<=narrow<=3
          wide:宽条宽度,以点（dot）表示,1<=wide<=7
       content:条码内容
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_PrintBarCode(int x, int y, char *type, int height, int humanreadable, int rotation, int narrow, int wide, char *content);

/*
	* 功能：打印二维码
	* 入口参数：
				x:欲绘制条码的水平坐标左上角起点，以点（dot）表示		
				y:欲绘制条码的垂直坐标左上角起点，以点（dot）表示
				type:条码类型
          QRCODE,DMATRIX,PDF417
        level:ECC LEVEL 选择QRCODE纠错等级(L:7% M:15% Q:25% H:30%)      当选择DATAMATRIX或者PDF417时，表示条码高度
        width:选择QRCode时表示二维码方块宽度1~6，选择DATAMATRIX或这PDF417时表示条码宽度
        rotate:二维码的旋转角度
              0            不旋转
             90            顺时针防线旋转90度
            180            顺时针防线旋转180度
            270            顺时针防线旋转270度
           当选择DATAMATRIX时没有作用
        content:二维码的内容
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_PrintBarCode2D(int x, int y, char *type, char *level, int width, int rotate, char *content);

/*
	* 功能：打印位图
	* 入口参数：
				x:点阵影像的水平起始位置(dot)		
				y:点阵影像的垂直起始位置(dot)
				mode:影像绘制模式 (0:OVERWRITE ；1:OR ；2:XOR)
				lpFilePath:位图本地路径
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_DrawBitmap(char *x, char *y, char *mode, char *lpFilePath);

/*
	* 功能：区域内打印位图,不支持；
	* 入口参数：
				area_start_x：区域左上角x坐标(dot)
				area_start_y：区域左上角Y坐标(dot)
				area_end_x：区域的右下角X坐标(dot)
				area_end_y：区域的右下角y坐标(dot)
				xAlign：水平方向的位置
				yAlign:竖直方向的位置
				mode:影像绘制模式 (0:OVERWRITE ；1:OR ；2:XOR)
				lpFilePath:位图本地路径
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
//int TSPL_DrawBitmap(char *area_start_x, char *area_start_y, char *area_end_x, char *area_end_y , char *xAlign, char *yAlign, char *mode, char *lpFilePath);

/*
	* 功能：打印
	* 入口参数：
				mSets:打印的份数	
				mCopys:每一份中每一标签页打印多少份
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_Print(int mSets, int mCopys);

/*
	* 功能：查询打印机状态
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；-1（缺纸）；-2（开盖）;-3（其他错误）
*/
int TSPL_GetPrinterStatus();

/*
	* 功能：进纸或退纸
	* 入口参数：
				isFeedBack:选择进纸或退纸(0:进纸，1：退纸)
				mDot:退纸点数，当选择进纸时，此参数无效
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_SetPaperbackOrPaperFeed(int isFeedBack, int mDot);

/*
	* 功能：指定区域内反相打印
	* 入口参数：
				start_x:区域左上角X坐标（dot）
				start_y:区域左上角Y坐标（dot）
				width:区域的宽度（dot）
				height:区域的高度（dot）
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_ReverseArea(int start_x, int start_y, int width, int height);

/*
	* 功能：清除影像缓冲区数据
	* 入口参数：
				start_x:区域左上角X坐标（dot）
				start_y:区域左上角Y坐标（dot）
				width:区域的宽度（dot）
				height:区域的高度（dot）
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_EraseArea(int start_x, int start_y,int width, int height);

/*
	* 功能：设置打印机相关功能
	* 入口参数：
				command/value:
	 *            1:DIRECTION 设置打印方向；value 为0方向和字体相同，1相反
	 * 
	 *            2:FEED 控制进纸设置；1≤value≤1000 表示点数
	 * 
	 *            3:REPRINT 启用错误重新打印命令；value为0关闭此功能；1开启此功能
	 * 
	 *            4:SPEED 指定打印速度；value为每秒走纸英寸数
	 * 
	 *            5:DENSITY 指定打印浓度；0=<value<=15,数字越大浓度越大
	 * 
	 *            6:SHIFT 走纸偏移value点；value >0,走纸方向和打印方向相同；<0 相反；
	 * 
	 *            7:OFFSET 每单数据打印完成后额外走纸偏移value点
	 * 
	 *            8:FORMFEED 控制打印机进一张纸；value值随意传入
	 * 
	 *            9:HOME 寻起始位置，使缝隙对准撕纸片；value值随意传入
	 * 
	 *            10:PRINTKEY 设定按键打印命令(禁止，启用，自动设定按键打印功能) value为0关闭此功能；1开启此功能
	 * 
	 *            11:KEY1 启用按键KEY1预设功能命令(此按键功能是暂停还是进纸) value为0关闭此功能；1开启此功能
	 * 
	 *            12:KEY2 启用按键KEY2预设功能命令(此按键功能是暂停还是进纸) value为0关闭此功能；1开启此功能
	 * 
	 *            13:TEAR 启用或关闭走到撕纸位置命令(缝隙对准撕纸位) value为0关闭此功能；1开启此功能
	 * 
	 *            14:PEEL 启用剥离模式命令 value为0关闭此功能；1开启此功能
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_SetPrinter(int command, int value);

/*
	* 功能：开钱箱
	* 入口参数：
				t1:0≤t1≤255
				t2:0≤t2≤255
	*     输出由 t1 和 t2 设定的钱箱开启脉冲到由 m 指定的引脚
	* 
	*     注释： 1) 钱箱开启脉冲高电平时间为 [t1 x 2 ms],低电平时间为 [t2 x 2ms]. 2) 如果 t2
	*     <t1，低电平时间为[t1 x 2 ms]
	
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_OpenCashBox(int t1, int t2);

/*
	* 功能：查询打印机型号
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_GetPrinterName();

/*
	* 功能：设置黑标高度
	* 入口参数：
				blackHeight:黑标高度（dot）
				      value:打印完成后额外走纸距离（dot）
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_SetBline(int blackHeight, int value);

/*
	* 功能：设置标签纸间垂直间距
	* 入口参数：
				value:标签纸间垂直间距（单位：mm），mm（0≦m≤1(inch）， 0≦m≤25.4(mm）
			 offset:间隙偏移（单位：mm），offset ≤ label length (inch or mm)
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_SetGAP(int value, int offset);

/*
	* 功能：选择字符代码页
	* 入口参数：
				value:
   *            437: United States
	 * 
	 *            850: Multilingual
	 * 
	 *            852: Slavic
	 * 
	 *            860: Portuguese
	 * 
	 *            863: Canadian/French
	 * 
	 *            865: Nordic
	 * 
	 *            857: Turkish
	 * 
	 *            1250: Central Europe
	 * 
	 *            1252: Latin I
	 * 
	 *            1253: Greek
	 * 
	 *            1254: Turkish
	 * 
	 *            传入其他值：默认为437
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_SelectGodePage(int value);

/*
	* 功能：选择国际字符集
	* 入口参数：
				value:
	 *            1: USA
	 * 
	 *            2: Canadian-French
	 * 
	 *            3: Spanish (Latin America)
	 * 
	 *            33:French (France)
	 * 
	 *            34: Spanish (Spain)
	 * 
	 *            39: Italian
	 * 
	 *            42: Slovak
	 * 
	 *            44:United Kingdom
	 * 
	 *            45: Danish
	 * 
	 *            46: Swedish
	 * 
	 *            47: Norwegian
	 * 
	 *            49: German
	 * 
	 *            61: English (International)
	 * 
	 *            传入其他值：默认为1
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_SelectCountry(int value);

/*
	* 功能：控制蜂鸣器响一声
	* 入口参数：
				level:蜂鸣器响的级别 1≤level≤9
		 interval:蜂鸣器响的时间 间隔（单位ms）1≤interval≤4095
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_Beep(int level, int interval);

/*
	* 功能：打印自检页
	* 入口参数：无
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_PrintSelfTestTSP();

/*
	* 功能：设置标签的参考坐标原点，调整标签内容在标签纸上的位置
	* 入口参数：
				left:标签内容距离标签纸的左边距（mm）
		     top:标签内容距离标签纸的上边距（mm）
	* 出口参数：无
	* 返回值：0（正常）；<0（错误）；
*/
int TSPL_SetLabelReference(int left, int top);



//-------------------ztongli.2016.12.19------------------
/*
	*功能：将三个打开端口方法合并为一个方法
	*入口参数：
			parameter：SERIAL_PORT为端口时传入设备文件名称,NET_PORT为端口时传入IP地址,USB_PORT无需参数传入NULL
	*出口参数：无
	*返回值：无
*/
int OpenPort(int port, char* parameter);

/*
	*功能：将三个关闭端口方法合并为一个方法
	*入口参数：
	*出口参数：无
	*返回值：无
*/
int ClosePort();

/*
	*功能：将三个发送数据的方法合并为一个方法
	*入口参数：
			attr：
				Flag：可以被赋值为三个端口宏的任意一个
				cmdBuf：char类型的指针
				bufLen：发送数据的字节长度
				sleepTime：睡眠时间（ms）
	*出口参数：无
	*返回值：无
*/
int writeData(AL* attr);

/*
	*功能：将三个读取数据的方法合并为一个方法
	*入口参数：
			attr：
				Flag：可以被赋值为三个端口宏的任意一个
				cmdBuf：char类型的指针 
				bufLen：期望读取到的字节数
				sleepTime：睡眠时间（ms）
	*出口参数：无
	*返回值：无
*/
int readData(AL* attr);


/*
	*功能：设置AL结构体的端口标志位
	*入口参数：
			port:端口宏
	*出口参数：无
	*返回值：0为成功,-1为失败
*/
int setAttribute(int port);

/*
	*功能：清空AL全局结构体的个别成员
	*入口参数：
			al：自定义结构体
	*出口参数：无
	*返回值：无
*/
void emptyAttribute(AL* al);

/*
  * 2016.12.13.ztongli
	* 功能：打印机状态查询（可恢复错误、不可恢复错误状态）
	* 入口参数：无
	* 出口参数：无
	* 返回值：传输错误状态码，-3通信时打印机返回的数据有误,-1表示发送指令有误
	* 描述: 需求方要求只返回传输错误状态码，参数无需手工传递
*/
int  QueryErrState();


#ifdef __cplusplus
}
#endif


#endif

