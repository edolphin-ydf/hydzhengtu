// [ranqd] 重载new/delete检测内存溢出

inline void Engine_WriteLogFile(const char* szString)
{
	FILE* fp;
	fp = fopen("C:\\log.txt", "a+");
	if( fp )
	{
		fprintf(fp,"%s\n", szString);
		fclose(fp);
	}
}

#define MEM_SIGN MAKEFOURCC('R','A','N','D')
// [ranqd] 内存记录标志结构
struct MEM_LOG_INFO_HEADER{
	char file[MAX_PATH];
	int  line;
	size_t  size;
	BYTE  Fill[256];
	MEM_LOG_INFO_HEADER(size_t tSize,const char* pfile, int tline)
	{
		strncpy(file,pfile,sizeof(file));
		line = tline;
		size = tSize;
	}
};
extern std::vector<void*> G_MemList;

struct MEM_LOG_INFO_LAST{
	DWORD flag;
	BYTE  Fill[256];
	MEM_LOG_INFO_LAST()
	{
		flag = MEM_SIGN;
	}
};
inline void * operator new(size_t size,  
						   const char *file, int line) 
{ 
	if( size == 0 ) size = 1;
	//char tmp[500];
	//sprintf(tmp,"%s(%d)",file,line);
	//MessageBox(NULL,tmp,"",NULL);
	for(std::vector<void*>::iterator it = G_MemList.begin();
		it != G_MemList.end(); it ++)
	{
		void* tp = (*it);
		if( (*(MEM_LOG_INFO_LAST*)((DWORD)tp + sizeof(MEM_LOG_INFO_HEADER) + ((MEM_LOG_INFO_HEADER*)tp)->size)).flag != MEM_SIGN )
		{
			char tmpc[1000];
			sprintf(tmpc,"分配时检测到%s(%d)处申请的内存被越界访问，请检查！\n",((MEM_LOG_INFO_HEADER*)tp)->file,((MEM_LOG_INFO_HEADER*)tp)->line);
			Engine_WriteLogFile(tmpc);
		}
	}
	void* p = malloc(size + sizeof(MEM_LOG_INFO_HEADER) + sizeof(MEM_LOG_INFO_LAST));
	*(MEM_LOG_INFO_HEADER*)p = MEM_LOG_INFO_HEADER(size,file,line);
	*(MEM_LOG_INFO_LAST*)((DWORD)p + sizeof(MEM_LOG_INFO_HEADER) + size ) = MEM_LOG_INFO_LAST();
	G_MemList.push_back(p);
	return (void*)((DWORD)p + sizeof(MEM_LOG_INFO_HEADER));
}; 

inline void operator delete(void *p) 
{ 
	//	MessageBox(NULL,"delete","",NULL);

	if( p == NULL ) return;
	for(std::vector<void*>::iterator it = G_MemList.begin();
		it != G_MemList.end();)
	{
		void* tp = (*it);
		if( (*(MEM_LOG_INFO_LAST*)((DWORD)tp + sizeof(MEM_LOG_INFO_HEADER) + ((MEM_LOG_INFO_HEADER*)tp)->size)).flag != MEM_SIGN )
		{
			char tmpc[1000];
			sprintf(tmpc,"释放时检测到%s(%d)处申请的内存被越界访问，请检查！\n",((MEM_LOG_INFO_HEADER*)tp)->file,((MEM_LOG_INFO_HEADER*)tp)->line);
			Engine_WriteLogFile(tmpc);
		}
		if( (DWORD)tp == (DWORD)p - sizeof(MEM_LOG_INFO_HEADER))
		{
			p = (void*)((DWORD)p - sizeof(MEM_LOG_INFO_HEADER));
			it = G_MemList.erase(it);
		}
		else
		{
			it ++;
		}
	}  
	free(p);
}; 
inline void * operator new[](size_t size,  
							 const char *file, int line) 
{ 
	if( size == 0 ) size = 1;
	//char tmp[500];
	//sprintf(tmp,"%s(%d)",file,line);
	//MessageBox(NULL,tmp,"",NULL);
	for(std::vector<void*>::iterator it = G_MemList.begin();
		it != G_MemList.end(); it ++)
	{
		void* tp = (*it);
		if( (*(MEM_LOG_INFO_LAST*)((DWORD)tp + sizeof(MEM_LOG_INFO_HEADER) + ((MEM_LOG_INFO_HEADER*)tp)->size)).flag != MEM_SIGN )
		{
			char tmpc[1000];
			sprintf(tmpc,"分配时检测到%s(%d)处申请的内存被越界访问，请检查！\n",((MEM_LOG_INFO_HEADER*)tp)->file,((MEM_LOG_INFO_HEADER*)tp)->line);
			Engine_WriteLogFile(tmpc);
		}
	}
	void* p = malloc(size + sizeof(MEM_LOG_INFO_HEADER) + sizeof(MEM_LOG_INFO_LAST));
	*(MEM_LOG_INFO_HEADER*)p = MEM_LOG_INFO_HEADER(size,file,line);
	*(MEM_LOG_INFO_LAST*)((DWORD)p + sizeof(MEM_LOG_INFO_HEADER) + size ) = MEM_LOG_INFO_LAST();
	G_MemList.push_back(p);
	return (void*)((DWORD)p + sizeof(MEM_LOG_INFO_HEADER));
}; 

inline void operator delete[](void *p) 
{ 
	//	MessageBox(NULL,"delete[]","",NULL);
	if( p == NULL ) return;
	for(std::vector<void*>::iterator it = G_MemList.begin();
		it != G_MemList.end();)
	{
		void* tp = (*it);
		if( (*(MEM_LOG_INFO_LAST*)((DWORD)tp + sizeof(MEM_LOG_INFO_HEADER) + ((MEM_LOG_INFO_HEADER*)tp)->size)).flag != MEM_SIGN )
		{
			char tmpc[1000];
			sprintf(tmpc,"释放时检测到%s(%d)处申请的内存被越界访问，请检查！\n",((MEM_LOG_INFO_HEADER*)tp)->file,((MEM_LOG_INFO_HEADER*)tp)->line);
			Engine_WriteLogFile(tmpc);
		}
		if( (DWORD)tp == (DWORD)p - sizeof(MEM_LOG_INFO_HEADER))
		{
			p = (void*)((DWORD)p - sizeof(MEM_LOG_INFO_HEADER));
			it = G_MemList.erase(it);
		}
		else
		{
			it ++;
		}
	}
	free(p);
};
#define DEBUG_NEW new(__FILE__,__LINE__)
#define new DEBUG_NEW
#pragma message("new Reload***********************************************")