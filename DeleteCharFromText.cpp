/*================================================================
 * 功能描述:从文本中读取数据，然后删除所有的\符号，写入另一个文本
 * 备注：可以学习下如何设计log
 =================================================================*/

#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <fstream>
#include <windows.h>
#include <iostream>
#include <time.h>
using namespace std;

#define MAX_FILE_PATH_LEN		128
#define MAX_LOG_BUF_LEN			256
#define READ_BUF_SIZE			1024*10
#define WRITE_BUF_SIZE			1024*10
#define OUTPUT_FILE_PATH		"D:\\output.xml"

#define st(x) do{x}while(false);

#define PrintErrMsg(msg) st(\
			LogErr(msg);\
			system("pause");\
		)

typedef enum tagEmTextColor
{
	TEXT_BLACK 			= 0,
	TEXT_BLUE			= 1,
	TEXT_GREEN			= 2,
	TEXT_LAKE_BLUE		= 3,
	TEXT_RED			= 4,
	TEXT_PURPLE			= 5,
	TEXT_YELLOW			= 6,
	TEXT_WHITE			= 7,
	TEXT_GREY			= 8
}EmTextColor;

void LogHint(const char *content, ...);//打印无错日志
void LogErr(const char *content, ...);//打印有错日志
void SetTextColor(EmTextColor emTextColor);//设置打印文字颜色


int main(int argc, char* argv[])
{   
	//check number of parameters
	if( argc <= 1 )
	{
		PrintErrMsg("[main]Lack of file path!");
		return -1;
	}
	
	//check length of file path
	int nFilePathLen = strlen(argv[1]);
	if( nFilePathLen >= MAX_FILE_PATH_LEN )
	{
		PrintErrMsg("[main]:file path is too long!");
		return -1;
	}
	
	LogHint("[main]:You got file path : %s", argv[1]);
	
	ifstream ifs;//for reading file //*** remember to close ***//
	ifs.open(argv[1], ifstream::in | ifstream::binary);
	if( ifs.fail() )
	{
		PrintErrMsg("[main]:Open input file failed!");
		ifs.close();
		return -1;
	}
	
	ofstream ofs;//for writing file  //*** remember to close ***//
	ofs.open(OUTPUT_FILE_PATH, ofstream::app | ofstream::binary);
	if( ofs.fail() )
	{
		PrintErrMsg("[main]:Open output file failed!");
		ifs.close();
		return -1;
	}
	//TODO:以后再补充检查有没有写权限
	LogHint("[main]:Result will be saved at %s", OUTPUT_FILE_PATH);
	
	//向文件中写入标题用以和之前内容区分
	char chTitle[WRITE_BUF_SIZE] = {0};
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	sprintf( chTitle, "\n\n\n<!--\nWelcome to use this tool!\nCurrentTime:%s-->\n", asctime(timeinfo) );
	ofs.write( chTitle, strlen(chTitle) );
	ofs.flush();
	
	//去除所有的'\'字符
	char chReadBuf[READ_BUF_SIZE];
	char chWriteBuf[WRITE_BUF_SIZE];
	int  nReadIndex = 0;
	int  nWriteIndex = 0;
	int  nActualReadLen = 0;
	do{
		memset(chReadBuf, 0, sizeof(chReadBuf));
		memset(chWriteBuf, 0, sizeof(chWriteBuf));
		int  nReadIndex = 0;
		int  nWriteIndex = 0;
		nActualReadLen = 0;
		
		ifs.read(chReadBuf, READ_BUF_SIZE);
		if( ifs.fail() && !ifs.eof() )//读取文件出错
		{
			PrintErrMsg("[main]");
			break;
		}
		
		for( int nReadIndex = 0; nReadIndex < ifs.gcount(); nReadIndex++ )
		{
			if( chReadBuf[nReadIndex] != '\\')
			{
				chWriteBuf[nWriteIndex] = chReadBuf[nReadIndex];
				nWriteIndex++;
			}
		}
		
		ofs.write(chWriteBuf, strlen(chWriteBuf));
		ofs.flush();
		
	}while( !ifs.eof() );
	
	LogHint("[main]:Conversion is finished!");
	
	ifs.close();
	ofs.close();
	
	//system("pause");
    return 0;
}


//打印无错日志
void LogHint(const char *content, ...)
{	
	SetTextColor(TEXT_GREEN);
	
	char chPrefix[MAX_LOG_BUF_LEN] = {"[LogHint]:\t"};
	strncat( chPrefix, content, strlen(content)+1);
	
	char chLogBuf[MAX_LOG_BUF_LEN] = {0};
	va_list args;
	va_start( args, chPrefix );
	vsprintf( chLogBuf, chPrefix, args );
	printf("%s\n", chLogBuf);
	va_end(args);
	
	SetTextColor(TEXT_WHITE);
}

//打印有错日志
void LogErr(const char *content, ...)
{
	SetTextColor(TEXT_RED);
	
	char chPrefix[MAX_LOG_BUF_LEN] = {"[LogErr]:\t"};
	strncat( chPrefix, content, strlen(content)+1);
	
	char chLogBuf[MAX_LOG_BUF_LEN] = {0};
	va_list args;
	va_start( args, chPrefix );
	vsprintf( chLogBuf, chPrefix, args );
	printf("%s\n", chLogBuf);
	va_end(args);
	
	SetTextColor(TEXT_WHITE);
}

//设置打印文字颜色
void SetTextColor(EmTextColor emTextColor)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), emTextColor);
}
