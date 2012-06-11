
/******************************

brief:��������


*******************************/



#ifndef _BOXCIRCLE_H
#define _BOXCIRCLE_H

#include "giftBox.h"


//���ɵ�һ������,�ں�17����Ʒ��id
struct box
{
	//DWORD items[17];
	//DWORD level[17];
	boxObject _boxObjects[17];
};



//���������ļ�����ĸ���,����ת�̵ķ���ѡȡ��װ����䵽������

class boxCircle : public SingletonBase<boxCircle>
{
	friend class SingletonBase<boxCircle>;
		
public:
	//���������ļ�����һ������
	box *generateOneBox(bool isGloden,int sex,int level);


public:
    
	//��������Ӻ������ӵ�ת��
	void createBoxCircle();
	
	//���һ�����������
	unsigned int getRandomIndex(unsigned int mod,unsigned int seed);


	


public:
	unsigned short silverBoxCircle[11];//������ת��
	unsigned short glodenBoxCircle[100];//�𱦺�ת��


private:
	boxCircle()
	{
		//createBoxCircle();
	}
	~boxCircle(){}

};


#endif