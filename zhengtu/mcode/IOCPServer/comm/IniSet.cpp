//********************************************
//	IniSet 相关函数
//  创建于2000年4月7日
//********************************************
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "IniSet.h"
#include "ReadFile.h"

#define SAFE_DELETE(A)	if( (A) != NULL ) { delete (A); (A) = NULL; }
#define SAFE_DELETE_ARRY(A)	if( (A) != NULL ) { delete [] (A); (A) = NULL; }
#define SAFE_RELEASE(A)	if( (A) != NULL ) { (A) -> Release(); (A) = NULL; }

//**************************
//显示调试信息
void ShowMessage(char *msg,...)
{
	va_list va;
	char str[256];

	va_start(va,msg);
	vsprintf(str,msg,va);
	va_end(va);

	MessageBox(NULL, str, "Message",MB_OK);
}


/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

//初始化
CIniSet::CIniSet()
{
	DataLen=0;
	Data=NULL;
	IndexNum=0;
	IndexList=NULL;
}

//初始化
CIniSet::CIniSet(char *filename)
{
	DataLen=0;
	Data=NULL;
	IndexNum=0;
	IndexList=NULL;
	Open(filename);
}

//析构释放
CIniSet::~CIniSet()
{
	if( DataLen != 0 && Data != NULL )
	{
		SAFE_DELETE_ARRY( Data );
	}

	if( IndexNum != 0 && IndexList != NULL )
	{
		SAFE_DELETE_ARRY( IndexList );
	}
}

//读入文件
bool CIniSet::Open(char *filename)
{
	strcpy(FileName, filename);

	SAFE_DELETE_ARRY( Data );

	CFileMgr *file = CFileMgr::Open(filename);
	if (file == NULL)
	{
		OutputDebugString("打开配置文件出错");
		return false;
	}

	DataLen = file->GetFileLength();	//获取文件长度
	if( DataLen > 0 )		//文件存在
	{
		Data=new char[DataLen];
		file->Read(Data, DataLen, 1);
		//初始化索引
		InitIndex();
	}
	else
	{
		DataLen=1;
		Data=new char[DataLen];
		memset(Data, 0, 1);
		InitIndex();
	}

	file->Close();
	return true;
}

//写入文件
bool CIniSet::Save(char *filename)
{
	if( filename==NULL )
		filename=FileName;

	FILE *fp;
	fp=fopen(filename, "wb");
	if( fp==NULL )
	{
		ShowMessage("Can't save %s ",filename);
		return false;
	}

	fwrite(Data, DataLen, 1, fp);
	fclose(fp);

	return true;
}

//计算出所有的索引位置
void CIniSet::InitIndex()
{
	IndexNum=0;

	for(int i=0; i<DataLen; i++)
	{
		//找到,要以[开头，而且段与段之间要用回车分开
		if( Data[i]=='[' && (i==0||(i>0&&Data[i-1]=='\n')) )
		{
			IndexNum++;
		}
	}

	//申请内存
	SAFE_DELETE_ARRY( IndexList );
	if( IndexNum>0 )
		IndexList=new int[IndexNum];

	int n=0;
	//初始化
	for(i=0; i<DataLen; i++)
	{
		if( Data[i]=='[' && ((i>0&&Data[i-1]=='\n' )|| i==0) )
		{
			IndexList[n]=i+1;		//保存到索引表的位置是 [ 的下一个字符的位置
			n++;
		}
	}
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

//返回指定标题位置
int CIniSet::FindIndex(char *string)
{
	for(int i=0; i<IndexNum; i++)
	{
		char *str=ReadText( IndexList[i] );		//遍历索引表，读所以的索引
		if( strcmp(string, str) == 0 )			//比较，如果向同就返回标题的位置
		{
			SAFE_DELETE_ARRY( str );
			return IndexList[i];				//索引所在的位置，是 [ 的字符的下一个位置
		}
		SAFE_DELETE_ARRY( str );
	}
	return -1;
}

//返回指定数据的内容的位置
int CIniSet::FindData(int index, char *string)
{
	int p=index;	//指针

	while(1)
	{									//跳过标题(索引)所在的行
		p=GotoNextLine(p);				//下一行的位置
		char *name=ReadDataName(p);		//读一个名字，注意p是按地址的方式传参的
		if( strcmp(string, name)==0 )	//相同就返回位置，数据内容开始的位置
		{
			SAFE_DELETE_ARRY( name );
			return p;
		}

		SAFE_DELETE_ARRY( name );
		if( p>=DataLen ) return -1;		//没有找到
	}
	return -1;
}

//提行
int CIniSet::GotoNextLine(int p)
{
	for(int i=p; i<DataLen; i++)
	{
		if( Data[i]=='\n' )				//回车换行
			return i+1;

	}
	return i;
}

//在指定位置读一数据名称
char *CIniSet::ReadDataName(int &p)			//引用参数，将通过p返回，而其值是
{											//数据的内容的位置
	char chr;
	char *Ret;
	int m=0;

	Ret= new char[256];
	memset(Ret, 0, 256);

	for(int i=p; i<DataLen; i++)
	{
		chr=Data[i];

		//结束
		if( chr == '\r' )
		{
			p=i+1;			
			return Ret;
		}
		
		//结束
		if( chr == '=' || chr == ';' )
		{
			p=i+1;						//指向数据的内容，跳过了分隔符
			return Ret;
		}
		
		Ret[m]=chr;
		m++;
	}
	return Ret;
}

//在指定位置读一字符串
char *CIniSet::ReadText(int p)
{
	char chr;
	char *Ret;
	int n=p, m=0;

	int EndLine=GotoNextLine(p);		//下一行的位置，只读一行的内容
	Ret = new char[EndLine-p+1];	//分配内存
	memset(Ret, 0, EndLine-p+1);		//清0

	for(int i=0; i<DataLen-p; i++)
	{
		chr=Data[n];

		//结束			用分号，回车，TAB或者是]结束
		if( chr == ';' || chr == '\r' || chr == '\t' || chr == ']' )
		{
			return Ret;
		}
		
		Ret[m]=chr;		//读入到缓冲区
		m++;
		n++;
	}
	return Ret;
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

//以普通方式读一字符串数据
char *CIniSet::ReadText(char *index, char *name)
{
	int n=FindIndex(index);				//通过索引名字找到位置
	if( n == -1 )
	{
		ShowMessage("Can't find [%s] in file '%s'!", index, FileName);
		return "";
	}

	int m=FindData(n, name);			//通过索引的位置和数据名字找到数据内容的位置
	if( m==-1 )
	{
		ShowMessage("Can't find [%s]-'%s' in file '%s'!", index, name, FileName);
		return "";
	}

	return ReadText(m);					//读出该内容
}
	
//在指定的行读一字符串，设计这个函数是用于读出一批数据，
//而且可以不关心具体的名字，可以用循环的方式读
char *CIniSet::ReadText(char *index, int lines)
{
	int n=FindIndex(index);				//通过索引名字找到位置
	if( n == -1 )
	{
		ShowMessage("Can't find [%s] in file '%s'!", index, FileName);
		return "";
	}

	//跳到指定行数
	n=GotoNextLine(n);
	for(int i=0; i<lines; i++)
	{
		if( n<DataLen )
			n=GotoNextLine(n);
	}

	//读数据
	while( n<=DataLen )
	{
		if( Data[n] == '=' )		//找到分隔符 =
		{
			n++;					//移动到=的下一个字符
			return ReadText(n);		//读出内容
		}
		if( Data[n] == '\r' )
		{
			return "";				//没有找到
		}
		n++;
	}

	return "";
}

//以普通方式读一整数数据
int CIniSet::ReadInt(char *index, char *name)
{
	int n=FindIndex(index);			//…………
	if( n == -1 )
	{
		ShowMessage("Can't find [%s] in file <%s>",index, FileName);
		return ERROR_DATA;
	}

	int m=FindData(n, name);		//看看上面的注释
	if( m==-1 )
	{
		ShowMessage("Can't find [%s] '%s' in file <%s>",index, name, FileName);
		return ERROR_DATA;
	}

	char *str=ReadText(m);			//………………
	int ret=atoi(str);				//转化成整数
	SAFE_DELETE_ARRY(str);
	return ret;
}

//在指定的行读一整数，同样是为了成批的读取，而且可以不关心具体的名字
int CIniSet::ReadInt(char *index, int lines)
{
	int n=FindIndex(index);			//不想多说了
	if( n == -1 )
	{
		ShowMessage("Can't find [%s] in file <%s>",index, FileName);
		return ERROR_DATA;
	}

	//跳到指定行数
	n=GotoNextLine(n);
	for(int i=0; i<lines; i++)
	{
		if( n<DataLen )
			n=GotoNextLine(n);
	}

	//读数据
	while( n<DataLen )
	{
		if( Data[n] == '=' )		//应该是明白了吧，不明白就看看上面的注释
		{
			n++;
			char *str=ReadText(n);	//…………
			int ret=atoi(str);
			SAFE_DELETE_ARRY(str);
			return ret;
		}
		if( Data[n] == '\r' )
		{
			return ERROR_DATA;
		}
		n++;
	}

	return ERROR_DATA;
}

//在指定的行读一数据名称，目的…………
char *CIniSet::ReadData(char *index, int lines)
{
	int n=FindIndex(index);			//没有语言了
	if( n == -1 )
	{
		ShowMessage("Can't find [%s] in file <%s>",index, FileName);
		return NULL;
	}

	//跳到指定行数
	n=GotoNextLine(n);
	for(int i=0; i<lines; i++)
	{
		if( n<DataLen )
			n=GotoNextLine(n);
	}

	return ReadDataName(n);			//读出名字返回
}

//以普通方式写一字符串数据。索引，名字，内容
bool CIniSet::WriteText(char *index, char *name, char *string)
{
	int n=FindIndex(index);			//我真的不想说了
	if( n == -1 )	//新建索引
	{
		AddIndex(index);			//加入一个一个索引
		n=FindIndex(index);			//找到其位置
		AddData(n, name, string);	//在当前位置n加一个数据
		return true;
	}

	//存在索引
	int m=FindData(n, name);
	if( m==-1 )		//新建数据
	{
		AddData(n, name, string);	//在当前位置n加一个数据
		return true;
	}

	//存在数据
	ModityData(n, name, string);	//修改一个数据

	return true;
}

//以普通方式写一整数
bool CIniSet::WriteInt(char *index, char *name, int num)
{
	char string[32];
	sprintf(string, "%d", num);

	int n=FindIndex(index);			//……
	if( n == -1 )	//新建索引
	{
		AddIndex(index);			//看上面的注释
		n=FindIndex(index);
		AddData(n, name, string);	//在当前位置n加一个数据
		return true;
	}

	//存在索引
	int m=FindData(n, name);
	if( m==-1 )		//新建数据
	{
		AddData(n, name, string);	//在当前位置n加一个数据
		return true;
	}

	//存在数据
	ModityData(n, name, string);	//修改一个数据

	return true;
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

//加入一个索引
bool CIniSet::AddIndex(char *index)
{
	char str[256];
	memset(str, 0, 256);
	int n=FindIndex(index);			//找找看有没有一样的

	if( n == -1 )	//新建索引
	{
		sprintf(str,"\r\n[%s]\r\n",index);		//注意格式，索引要分开
		//重新分配内存
		char *newBuf = new char[DataLen+strlen(str)];
		memcpy(newBuf, Data, DataLen);
		SAFE_DELETE_ARRY(Data);
		Data = newBuf;
		sprintf(&Data[DataLen], "%s", str);		//接在后面
		DataLen+=strlen(str);					//更新长度

		InitIndex();							//重新建立索引表
		return true;
	}
	
	return false;	//已经存在
}

//在当前位置加入一个数据
bool CIniSet::AddData(int p, char *name, char *string)
{
	char *str;
	int len=strlen(string);
	str=new char[len+256];						//留出空间
	memset(str, 0, len+256);
	sprintf(str,"%s=%s\r\n",name,string);		//化成tiamo=1的格式
	len=strlen(str);							//重新算长度
	
	p=GotoNextLine(p);	//提行
	//Data=(char *)realloc(Data, DataLen+len);	//重新分配内存
	//重新分配内存
	char *newBuf = new char[DataLen+len];
	memcpy(newBuf, Data, DataLen);
	SAFE_DELETE_ARRY(Data);
	Data = newBuf;
	
	char *temp=new char[DataLen-p];				//后面的内容放到temp里面
	memcpy(temp, &Data[p], DataLen-p);
	memcpy(&Data[p+len], temp, DataLen-p);		//把后面的搬到末尾
	memcpy(&Data[p], str, len);					//插入新加的内容
	DataLen+=len;								//更新数据长度

	SAFE_DELETE_ARRY( temp );
	SAFE_DELETE_ARRY( str );
	return true;
}

//在当前位置修改一个数据的值
bool CIniSet::ModityData(int p, char *name, char *string)
{
	int n=FindData(p, name);					//找到数据。注意：返回的n是内容的位置

	char *t=ReadText(n);						//读出数据的内容
	p=n+strlen(t);								//移动到数据的末尾的下一个位置
	SAFE_DELETE_ARRY(t);

	int newlen=strlen(string);					//新的数据长度
	int oldlen=p-n;								//t的长度，也就是原来的数据长度

	//Data=(char *)realloc(Data, DataLen+newlen-oldlen);	//重新分配内存
	//重新分配内存
	char *newBuf = new char[DataLen+newlen-oldlen];
	memcpy(newBuf, Data, DataLen);
	SAFE_DELETE_ARRY(Data);
	Data = newBuf;

	char *temp=new char[DataLen-p];						//相同的方法
	memcpy(temp, &Data[p], DataLen-p);
	memcpy(&Data[n+newlen], temp, DataLen-p);			//把后面的搬到末尾
	memcpy(&Data[n], string, newlen);
	DataLen+=newlen-oldlen;								//更新数据长度

	SAFE_DELETE_ARRY( temp );
	return true;
}

//返回文件内容
char *CIniSet::GetData()
{
	return Data;
}

//获得文件的行数
int CIniSet::GetLines(int cur)
{
	int n=1;
	for(int i=0; i<cur; i++)
	{
		if( Data[i]=='\n' )					//找到\n，就是新的一行
			n++;
	}
	return n;
}

//返回连续的行数，就是标题(索引)这个段共有多少行，一般是用来循环记数的
int CIniSet::GetContinueDataNum(char *index)
{
	int num=0;
	int n=FindIndex(index);				//恩…………
	n=GotoNextLine(n);					//跳过标题(索引)
	while(1)
	{									//要是一行的开头就是下面的字符的话，就结束
		if( Data[n] == '\r' || Data[n] == -3 || Data[n] == EOF 
			|| Data[n] == ' ' || Data[n] == '/' || Data[n] == '\t' || Data[n] == '\n' )
		{
			return num;
		}
		else
		{
			num++;						//加一
			n=GotoNextLine(n);			//到下一行的开头
			if( n >= DataLen )	
				return num;			//文件结束
		}
	}
}
