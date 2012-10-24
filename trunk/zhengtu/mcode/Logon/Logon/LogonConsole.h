
#ifndef __LOGONCONSOLE_H
#define __LOGONCONSOLE_H

#include "Common.h"
#include "CThreads.h"

class LogonConsoleThread : public ThreadBase
{
	public:
		MCodeNet::Threading::AtomicBoolean kill;
		LogonConsoleThread(const char* name);
		~LogonConsoleThread();
		bool run();
};

////////////////////////////////////////////////////////////////
/// @class LogonConsole
/// @brief ��¼����������̨
///
/// @note ��������Ȼ��ͨ��������Ʒ�������һЩ����
class LogonConsole :  public Singleton < LogonConsole >
{
		friend class LogonConsoleThread;

	public:						// Public methods:
		void Kill();

	protected:					// Protected methods:
		LogonConsoleThread* _thread;

		// Process one command
		void ProcessCmd(char* cmd);

		// quit | exit
		void TranslateQuit(char* str);
		void ProcessQuit(int delay);
		void CancelShutdown(char* str);

		// help | ?
		void TranslateHelp(char* str);
		void ProcessHelp(char* command);

		void TranslateRehash(char* str);

		void NetworkStatus(char* str);

		void Info(char* str);

		void CreateAccount(char* str);
};

#define sLogonConsole LogonConsole::getSingleton()

#endif
