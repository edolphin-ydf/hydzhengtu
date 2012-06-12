/**
 * \brief ʵ�ַ����������
 *
 * 
 */
#include <zebra/srvEngine.h>

#include <assert.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

/**
 * \brief CTRL + C���źŵĴ�����,��������
 *
 * \param signum �źű��
 */
static void ctrlcHandler(int signum)
{
  Zebra::logger->info("ctrlcHandler");
  fprintf(stderr,"ctrlcHandler\n");
  //���û�г�ʼ��zServiceʵ��,��ʾ����
  zService *instance = zService::serviceInstance();
  //������ѭ��
  instance->Terminate();
}

/**
 * \brief HUP�źŴ�����
 *
 * \param signum �źű��
 */
static void hupHandler(int signum)
{
  Zebra::logger->info("hupHandler");
  //���û�г�ʼ��zServiceʵ��,��ʾ����
  zService *instance = zService::serviceInstance();
  instance->reloadConfig();
}

zService *zService::serviceInst = NULL;

/**
 * \brief ��ʼ������������,������Ҫʵ���������
 *
 * \return �Ƿ�ɹ�
 */
bool zService::init()
{
  Zebra::logger->debug("zService::init");
  //�洢��������
  /*int i = 0;
  while(environ[i])
  {
    std::string s(environ[i++]);
    std::vector<std::string> v;
    stringtok(v,s,"=",1);
    if (!v.empty() && v.size() == 2)
      env[v[0]] = v[1];
  }*/
  //env.dump(std::cout);

  //��ʼ�������
  srand(time(NULL));
  
  return true;
}

/**
 * \brief ��������ܵ�������
 */
void zService::main()
{
  Zebra::logger->debug("zService::main");
  //��ʼ������,��ȷ�Ϸ����������ɹ�,���ý����źźͽ���ʱ���õĺ���
  if(signal(SIGTERM  , ctrlcHandler)==SIG_ERR)
  {
	fprintf(stderr,"�ź�����ʧ��\n");
  }
  
  //��ʼ����ȷ�Ϸ�������ʼ���ɹ��������������ص�����
  if (init()
  && validate())
  {
    //�������ص��߳�
    while(!isTerminate())
    {
      if (!serviceCallback())//�����������ص���������Ҫ���ڼ�������˿ڣ��������false���������򣬷���true����ִ�з���
      {                      //�����Ҫ����������������ض������棬�����������һֱѭ����������Ϊ����
        break;
      }
    }
  }

  //��������,�ͷ���Ӧ����Դ
  final();
}

