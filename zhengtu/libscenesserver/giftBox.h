#ifndef _GIFTBOX_H
#define _GIFTBOX_H

#include <vector>
#include <map>
#include <list>


class boxCircle;

//箱子处理

enum boxType
{
	golden = 0, //金箱
	silver,//银箱
};


struct qualitys
{
	unsigned int quality;
	unsigned int qualityodds;
	unsigned int maxnum;
	unsigned int numkind;
};

struct boxitem
{
	unsigned int itemid;
	unsigned int size;
	unsigned int maxnumber;
	unsigned int price;
};

struct giftBox
{
	
	boxType _boxType;
	boxitem _boxitem;
	std::vector<struct qualitys> _qualitys;
	
};

struct boxObject
{
	unsigned int obj_id;
	unsigned int num;
	unsigned int level;
	unsigned int kind;
	unsigned int itemkind;
	unsigned int itemlevel;
	unsigned int material;
	unsigned char sex;
};


struct boxItem
{
	unsigned int qualityID;
	//std::map<unsigned int,std::vector<boxObject>*> _boxObjects;
	struct _levelObj
	{
		operator int() const 
		{
			return (int)_obj->needlevel;
		}
		zObjectB *_obj;
		boxObject obj;
	};
	std::vector<_levelObj> levelObjectsmale;//与等级相关的物品,男性	
	std::vector<_levelObj> levelObjectsfamale;//与等级相关的物品,女性	

    

};



class globalBox : public SingletonBase<globalBox>
{
	friend class SingletonBase<globalBox>;
	friend class boxCircle;
public:	
	int init();

	const giftBox& getGlodenBox()
	{
		return glodenBox;
	}

	const giftBox& getSilverBox()
	{
		return silverBox;
	}


private:
	void addBoxItem(boxItem _boxItem);



private:
	globalBox(){};
	~globalBox(){};

	
private:
	struct giftBox glodenBox;
	struct giftBox silverBox;
	std::map<unsigned int,struct boxItem> _boxItems;
		
};


#endif