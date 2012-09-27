//********************************************
//	IniSet ��غ���
//  ������2000��4��7��
//********************************************
#ifndef _GP_INI_
#define _GP_INI_

#define ERROR_DATA -99999999

//�����ļ���
class CIniSet{
private:
	char FileName[MAX_PATH];	//�ļ���
	int DataLen;			//�ļ�����
	char *Data;				//�ļ�����

	int IndexNum;			//������Ŀ��[]����Ŀ��
	int *IndexList;			//������λ���б�

	int Point;				//��ǰָ��
	int Line, Word;			//��ǰ����

public:
	CIniSet();
	CIniSet(char *);		//��ʼ���������ļ�
	~CIniSet();				//�ͷ��ڴ�
	char *GetData();		//�����ļ�����
	int GetLines(int);			//�����ļ�������

	bool Open(char *);		//�������ļ�
	bool Save(char *filename=NULL);		//���������ļ�

private:
	void InitIndex();			//��ʼ������
	int FindIndex(char *);		//���ر���λ��
	int FindData(int, char *);	//��������λ��
	int GotoNextLine(int); 		//����
	char *ReadDataName(int &);	//��ָ��λ�ö�һ��������
	char *ReadText(int);		//��ָ��λ�ö��ַ���

	bool AddIndex(char *);		//����һ������
	bool AddData(int, char *, char *);	//�ڵ�ǰλ�ü���һ������
	bool ModityData(int, char *, char *); //�ڵ�ǰλ���޸�һ�����ݵ�ֵ

public:
	int ReadInt(char *, char *);	//��һ������
	char *ReadText(char *, char *);	//��һ���ַ���(ɾ�����ص��ַ���)
	int ReadInt(char *, int );		//��ָ�����ж�һ����
	char *ReadText(char *, int);	//��ָ�����ж�һ�ַ���
	char *ReadData(char *, int);	//��ָ���ж�һ�ַ�����

	bool WriteInt(char *, char *, int);	//дһ������
	bool WriteText(char *, char *, char *);	//дһ���ַ���

	int GetContinueDataNum(char *);			//������������������INDEX����һ�����У�
};

#endif