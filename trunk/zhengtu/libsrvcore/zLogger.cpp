/**
* \brief Zebra��Ŀ��־ϵͳ�����ļ�
*
*/
#include <zebra/srvEngine.h>

/**
* \brief ����һ��zLogger 
*
* \param  name zLogger�����֣�����������������־�е�ÿһ��
*/
zLogger::zLogger(char *name)
{
	fp_console = stdout;
	fp_file    = NULL;

	m_name     = name;
	m_file     = "";
	m_level    = LEVEL_ERROR;
	m_day      = 0;
}

/**
* \brief ��������
*/
zLogger::~zLogger()
{
	if (NULL != fp_file)
	{
		fclose(fp_file);
		fp_file = NULL;
	}
}

/**
* \brief �Ƴ�����̨Log���
*/
void zLogger::removeConsoleLog()
{
	msgMut.lock();
	fp_console = NULL;
	msgMut.unlock();
}

/**
* \brief ��һ�������ļ�Log���
*
* \param file Ҫ������ļ�����Logger���Զ������ʱ���׺ 
* \return ��
*/
void zLogger::addLocalFileLog(const std::string &file)
{
	msgMut.lock();
	m_day  = 0;
	m_file = file;
	msgMut.unlock();
}

/**
* \brief ����д��־�ȼ�
* \param  zLevelPtr ��־�ȼ�.�μ� #zLogger::zLevel
*/
void zLogger::setLevel(const zLevel level)
{
	msgMut.lock();
	m_level = level;
	msgMut.unlock();
}

/**
* \brief ����д��־�ȼ�
* \param  level ��־�ȼ�
*/
void zLogger::setLevel(const std::string &level)
{
	if ("off" == level) setLevel(LEVEL_OFF);
	else if ("fatal" == level) setLevel(LEVEL_FATAL);
	else if ("error" == level) setLevel(LEVEL_ERROR);
	else if ("warn" == level) setLevel(LEVEL_WARN);
	else if ("info" == level) setLevel(LEVEL_INFO);
	else if ("debug" == level) setLevel(LEVEL_DEBUG);
	else if ("all" == level) setLevel(LEVEL_ALL);
}

void zLogger::logtext(const zLevel level,const char * text)
{
	if (m_level > level) return;
	log(level,"%s",text);  
}

void zLogger::logva(const zLevel level,const char * pattern,va_list vp)
{
	SYSTEMTIME system;
	struct tm *now;
	time_t ltime;
	char   szName[_MAX_PATH];

	if (m_level > level) return;
	time(&ltime);
	if (NULL == (now=localtime(&ltime))) return;

	GetLocalTime(&system);

	msgMut.lock();

	if (!m_file.empty())
	{
		if (m_day != now->tm_mday)
		{
			if (NULL != fp_file)
			{
				fclose(fp_file);
			}
			m_day = now->tm_mday;
			_snprintf(szName,sizeof(szName),"%s%04d%02d%02d.log",m_file.c_str(),now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
			fp_file = fopen(szName,"at");
		}
	}                 

	if (NULL != fp_console)
	{
		fprintf(fp_console,"[%s] ",m_name.c_str());
	}
	if (NULL != fp_file)
	{
		fprintf(fp_file,"[%s] ",m_name.c_str());
	}

	if (NULL != fp_console)
	{
		fprintf(fp_console,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(fp_console,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}
	if (NULL != fp_file)
	{
		fprintf(fp_file,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(fp_file,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}

	if (NULL != fp_console)
	{
		vfprintf(fp_console,pattern,vp);
		fprintf(fp_console,"\n");
		fflush(fp_console);
	}
	if (NULL != fp_file)
	{
		vfprintf(fp_file,pattern,vp);
		fprintf(fp_file,"\n");
		fflush(fp_file);
	}

	msgMut.unlock();
}

/**
* \brief д��־
* \param  zLevelPtr ��־�ȼ��μ� #zLogger::zLevel
* \param  pattern �����ʽ��������printfһ��
* \return ��
*/
void zLogger::log(const zLevel level,const char * pattern,...)
{
	va_list vp;

	if (m_level > level) return;
	va_start(vp,pattern);
	logva(level,pattern,vp);
	va_end(vp);
}

/**
* \brief дfatal������־
* \param  pattern �����ʽ��������printfһ��
* \return ��
*/
void zLogger::fatal(const char * pattern,...)
{
	va_list vp;

	if (m_level > LEVEL_FATAL) return;
	va_start(vp,pattern);
	logva(LEVEL_FATAL,pattern,vp);
	va_end(vp);
}

/**
* \brief дerror������־
* \param  pattern �����ʽ��������printfһ��
* \return ��
*/
void zLogger::error(const char * pattern,...)
{
	va_list vp;

	if (m_level > LEVEL_ERROR) return;
	va_start(vp,pattern);
	logva(LEVEL_ERROR,pattern,vp);
	va_end(vp);
}

/**
* \brief дwarn������־
* \param  pattern �����ʽ��������printfһ��
* \return ��
*/
void zLogger::warn(const char * pattern,...)
{
	va_list vp;

	if (m_level > LEVEL_WARN) return;
	va_start(vp,pattern);
	logva(LEVEL_WARN,pattern,vp);
	va_end(vp);
}

/**
* \brief дinfo������־
* \param  pattern �����ʽ��������printfһ��
* \return ��
*/
void zLogger::info(const char * pattern,...)
{
	va_list vp;

	if (m_level > LEVEL_INFO) return;
	va_start(vp,pattern);
	logva(LEVEL_INFO,pattern,vp);
	va_end(vp);
}

/**
* \brief дdebug������־
* \param  pattern �����ʽ��������printfһ��
* \return ��
*/
void zLogger::debug(const char * pattern,...)
{
	va_list vp;

	if (m_level > LEVEL_DEBUG) return;
	va_start(vp,pattern);
	logva(LEVEL_DEBUG,pattern,vp);
	va_end(vp);
}

/*********************/
/* ���16��������    */
/*********************/
void zLogger::debug16(const char* info, const BYTE* pData, int Datasize)
{
	struct tm *now;
	time_t ltime;
	char   szName[_MAX_PATH];

	if (m_level > LEVEL_DEBUG) return;
	time(&ltime);
	if (NULL == (now=localtime(&ltime))) return;

	SYSTEMTIME system;
	GetLocalTime(&system);

	msgMut.lock();

	if (!m_file.empty())
	{
		if (m_day != now->tm_mday)
		{
			if (NULL != fp_file)
			{
				fclose(fp_file);
			}
			m_day = now->tm_mday;
			_snprintf(szName,sizeof(szName),"%s%04d%02d%02d.log",m_file.c_str(),now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
			fp_file = fopen(szName,"at");
		}
	}
	if (NULL != fp_console)
	{
		fprintf(fp_console,"[%s] ",m_name.c_str());
	}
	if (NULL != fp_file)
	{
		fprintf(fp_file,"[%s] ",m_name.c_str());
	}

	if (NULL != fp_console)
	{
		fprintf(fp_console,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(fp_console,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}
	if (NULL != fp_file)
	{
		fprintf(fp_file,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(fp_file,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}

	if (NULL != fp_console)
	{
		fprintf(fp_console, "%s ���� = %u:\n",info, Datasize );
		for(int i = 0; i < Datasize; i ++)
		{
			fprintf(fp_console,"%2.2X ", pData[i]);
		}
		fprintf(fp_console,"\n\n");
		fflush(fp_console);
	}
	if (NULL != fp_file)
	{
		fprintf(fp_file, "%s ���� = %u :\n",info, Datasize);
		for(int i = 0; i < Datasize;i ++)
		{
			fprintf(fp_file,"%2.2X ", pData[i]);
		}
		fprintf(fp_file,"\n\n");
		fflush(fp_file);
	}

	msgMut.unlock();
}