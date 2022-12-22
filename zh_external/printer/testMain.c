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
			perror("iconv");
		}
		else
		{
			ret = pout;
		}
		iconv_close(cd);
	}

	return ret;
}
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0)
		return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
		return -1;
	iconv_close(cd);
	return 0;
}
//UNICODE码转为GB2312码
int u2g(char *inbuf, int inlen, char *outbuf, int outlen)
{
	return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
//GB2312码转为UNICODE码
int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}
int main(int argc, char **argv)
{
	int ret = -1;

	ret = OpenPort(SERIAL_PORT, argv[1]);
	if (ret >= 0)
	{
		int type = 4;
		ret = POS_Status_RTQueryTypeStatus(type);
		if (ret == 0)
		{
			int isFont = 1;
			int isBold = 1;
			int isDoubleWidth = 1;
			int isDoubleHeight = 1;
			int isUnderLine = 1;
			ret = POS_Control_SetPrintFontE(isFont, isBold, isDoubleWidth, isDoubleHeight, isUnderLine);
			ret |= POS_Control_SetPrintFontC(isDoubleWidth, isDoubleHeight, isUnderLine);
			if (ret == 0)
			{
				POS_Control_AlignType(1);
				char header_content[] = "卓创智汇自动称重";
				char header_gbk[1024];
				u2g(header_content, strlen(header_content), header_gbk, sizeof(header_gbk));
				ret |= POS_Output_PrintString(header_gbk);

				char tmpp[4096];
				u2g(argv[2], strlen(argv[2]), tmpp, sizeof(tmpp));

				ret |= POS_Output_PrintString(tmpp);

				if (argc >= 4)
				{
					char qr_gbk[2048];
					u2g(argv[3], strlen(argv[3]), qr_gbk, sizeof(qr_gbk));
					ret |= POS_Output_PrintBar2code(2, 76, 0, 3, qr_gbk);
				}
				ret |= POS_Output_PrintBuffAndFeedLines(5);
				if (0 == ret)
				{
					ret = POS_Control_CutPaper(0, 0);
					if (0 == ret)
					{
					}
					else
					{
						printf("POS_Control_CutPaper() fail ret=%d\n", ret);
					}
				}
				else
				{
					printf("POS_Output_PrintString() fail ret=%d\n", ret);
				}
			}
			else
			{
				printf("POS_Control_SetPrintFontE() fail ret=%d\n", ret);
				printf("POS_Control_SetPrintFontC() fail ret=%d\n", ret);
			}
		}
		else if (1 == ret)
		{
			printf("POS_Status_RTQueryTypeStatus() no paper!!! ret=%d\n", ret);
		}
		else
		{
			printf("POS_Status_RTQueryTypeStatus() fail ret=%d\n", ret);
		}
		ClosePort();
	}
	else
	{
		printf("OpenPort() fail ret=%d\n", ret);
	}

	return ret;
}
