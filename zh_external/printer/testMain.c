#include <stdio.h>
#include "HWISPRTPrinter.h"
#include <iconv.h>

char *convert_utf(const char * utf)
{
	char *ret = NULL;
	iconv_t cd = iconv_open( "gb2312","utf-8");
	if (cd != (iconv_t)-1)
	{
		size_t in_len = strlen(utf);
		size_t out_len = in_len * 3;
		char *pout = calloc(1UL, out_len);
		char *pin = utf;
		if (-1 == iconv(cd, &pin, &in_len, &pout, &out_len))
		{
			free(pout);
		}
		else
		{
			ret = pout;
		}
		iconv_close(cd);
	}

	return ret;
}

void main(int argc, char **argv)
{
	int ret = -1;

	ret = OpenPort(NET_PORT, argv[1]); //2016.12.21 14:23 OK ʹ�û���POSIMP88VESP

	if (ret < 0)
	{
		printf("OpenPort() fail ret=%d\n", ret);
		return;
	}
	printf("OpenPort() ok ret=%d\n", ret);

	int type = 4;
	ret = POS_Status_RTQueryTypeStatus(type);
	if (ret < 0)
	{
		printf("POS_Status_RTQueryTypeStatus() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	if (ret == 1)
	{
		printf("POS_Status_RTQueryTypeStatus() no paper!!! ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Status_RTQueryTypeStatus() ok ret=%d\n", ret);

	ret = QueryErrState();
	if (ret < 0)
	{
		printf("QueryErrState ERR,ret:%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("QueryErrState OK,status: %d\n", ret);

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

	ret = POS_Control_SetPrintFontC(isDoubleWidth, isDoubleHeight, isUnderLine);
	if (ret < 0)
	{
		printf("POS_Control_SetPrintFontC() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);

		return;
	}
	printf("POS_Control_SetPrintFontC() ok ret=%d\n", ret);

	char *p = convert_utf(argv[2]);
	int len = strlen(p);

	ret = POS_Output_PrintString(p);
	if (ret < 0)
	{
		printf("POS_Output_PrintString() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Output_PrintString() ok ret=%d\n", ret);
	ret = POS_Control_CutPaper(0, 0);
	if (ret < 0)
	{
		printf("POS_Control_CutPaper() fail ret=%d\n", ret);
		ret = ClosePort();
		printf("ClosePort() finished, ret=%d\n", ret);
		return;
	}
	printf("POS_Control_CutPaper() ok ret=%d\n", ret);

	ret = ClosePort();
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
