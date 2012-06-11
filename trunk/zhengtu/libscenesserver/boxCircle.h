
/******************************

brief:宝盒生成


*******************************/



#ifndef _BOXCIRCLE_H
#define _BOXCIRCLE_H

#include "giftBox.h"


//生成的一个宝盒,内含17个物品的id
struct box
{
	//DWORD items[17];
	//DWORD level[17];
	boxObject _boxObjects[17];
};



//根据配置文件定义的概率,利用转盘的方法选取出装备填充到宝盒上

class boxCircle : public SingletonBase<boxCircle>
{
	friend class SingletonBase<boxCircle>;
		
public:
	//根据配置文件生成一个宝盒
	box *generateOneBox(bool isGloden,int sex,int level);


public:
    
	//创建金盒子和银盒子的转盘
	void createBoxCircle();
	
	//获得一个随机的索引
	unsigned int getRandomIndex(unsigned int mod,unsigned int seed);


	


public:
	unsigned short silverBoxCircle[11];//银宝盒转盘
	unsigned short glodenBoxCircle[100];//金宝盒转盘


private:
	boxCircle()
	{
		//createBoxCircle();
	}
	~boxCircle(){}

};


#endif