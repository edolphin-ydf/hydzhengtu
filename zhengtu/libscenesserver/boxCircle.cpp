#include <zebra/ScenesServer.h>
/*#include <zebra/csBox.h>
#include <zebra/csTurn.h>
*/
#include "boxCircle.h"



void _swap(unsigned short &a,unsigned short &b)
{
	unsigned short temp = a;
	a = b;
	b = temp;
}

unsigned int boxCircle::getRandomIndex(unsigned int mod,unsigned int seed)
{
	zRTime tt;
	srand(tt.msecs() + ( seed + 32582657) + seed);
	//srand(rand());

	return rand() % mod;
}


//返回> =   target的最小数的下标
template   <class   T>
int   findAbove(T   &_array,int   begin,int   end, int   target)
{

        int   mid   =   (end-begin)/2   +   begin;


        if(end   -   begin   >=   2)
        {
			if(_array[mid]   >=   target   &&   _array[mid-1]   <   target)
				return   mid;
			else   if(_array[mid]   <   target)
				return   findAbove(_array,mid+1,end,target);
			else  
				return   findAbove(_array,begin,mid,target);
         }
        else
        {
			if(_array[begin]   >=   target)
				return   begin;
			else   if(_array[end]   >=   target)
				return   end;
			else
				return   -1;
        }

}


//返回 <target的最大数的下标
template   <class   T>
int   findBelow(T   &_array,int   begin,int   end, int   target)
{

        int   mid   =   (end-begin)/2   +   begin;


        if(end   -   begin   >=   2)
        {
			if(_array[mid]   <   target   &&   _array[mid+1]   >=   target)
				return   mid;
			else   if(_array[mid]   <   target)
				return   findBelow(_array,mid+1,end,target);
			else  
				return   findBelow(_array,begin,mid,target);
        }
        else
        {
			if(_array[end]   <   target)
				return   end;
			else   if(_array[begin]   <   target)
				return   begin;
			else
				return   -1;
        }

}


box *boxCircle::generateOneBox(bool isGloden,int sex,int level)
{
	box *_box = new box;
	/*{
		DWORD items[17];
		DWORD level[17]; 
	};*/

	//Zebra::logger->error("in generate----------");

	unsigned int size;
	if(isGloden)
	{
		for(int i = 0; i < 17;++i)
		{
			unsigned int quality = 	boxCircle::getInstance().glodenBoxCircle[getRandomIndex(100,i+1)];
			std::map<unsigned int,struct boxItem> *_boxItems = &(globalBox::getInstance()._boxItems);
			struct boxItem _boxItem = (*_boxItems)[quality];

			if(sex == 1)
			{
				level -= 10;
				int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsmale,0,_boxItem.levelObjectsmale.size()-1,70);
					
				if(_index == -1)
				{
					unsigned int len = _boxItem.levelObjectsmale.size();
					unsigned int index = getRandomIndex(len,i+1);
					_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
						
				}
				else
				{
					unsigned int len = _boxItem.levelObjectsmale.size() - _index;
					unsigned int index = getRandomIndex(len,i+1);
					index += _index;

					_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
				}
				
				/*if(level >= 80)
				{
					int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsmale,0,_boxItem.levelObjectsmale.size()-1,70);
					
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
						
					}
					else
					{
						unsigned int len = _boxItem.levelObjectsmale.size() - _index;
						unsigned int index = getRandomIndex(len,i+1);
						index += _index;

						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
					}

				}
				else
				{
					int _index = findBelow<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsmale,0,_boxItem.levelObjectsmale.size()-1,70);
					
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
						
					}
					else
					{
						unsigned int len = _index - 0 + 1;
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;
					}


						
					
				}*/

			}

			else
			{
				level -= 10;
				int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsfamale,0,_boxItem.levelObjectsfamale.size()-1,70);
					
				if(_index == -1)
				{
					unsigned int len = _boxItem.levelObjectsmale.size();
					unsigned int index = getRandomIndex(len,i+1);
					_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
						
				}
				else
				{
					unsigned int len = _boxItem.levelObjectsfamale.size() - _index;
					unsigned int index = getRandomIndex(len,i+1);
					index += _index;

					_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;
				}
				/*if(level >= 80)
				{
					int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsfamale,0,_boxItem.levelObjectsfamale.size()-1,70);
					
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
						
					}
					else
					{
						unsigned int len = _boxItem.levelObjectsfamale.size() - _index;
						unsigned int index = getRandomIndex(len,i+1);
						index += _index;

						_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;
					}
				}
				else
				{


					int _index = findBelow<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsfamale,0,_boxItem.levelObjectsfamale.size()-1,70);
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;	
						
					}
					else
					{
						unsigned int len = _index - 0 + 1;
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;
					}
						
					
				}	*/			
			}

		}
	}
	else
	{
		for(int i = 0; i < 17;++i)
		{
			unsigned int quality = 	boxCircle::getInstance().silverBoxCircle[getRandomIndex(10,i+1)];//(_silverBox->_qualitys)[10].quality;
			//Zebra::logger->error("quality = %u",quality);
			std::map<unsigned int,struct boxItem> *_boxItems = &(globalBox::getInstance()._boxItems);
			struct boxItem _boxItem = (*_boxItems)[quality];
			if(sex == 1)
			{
				

				level -= 10;
				int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsmale,0,_boxItem.levelObjectsmale.size()-1,70);
					
				if(_index == -1)
				{
					unsigned int len = _boxItem.levelObjectsmale.size();
					unsigned int index = getRandomIndex(len,i+1);
					_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;						
				}
				else
				{
					
					unsigned int len = _boxItem.levelObjectsmale.size() - _index;
					unsigned int index = getRandomIndex(len,i+1);
					index += _index;
					_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;
				}
				/*if(level >= 80)
				{
					int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsmale,0,_boxItem.levelObjectsmale.size()-1,70);
					
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;						
					}
					else
					{
					
						unsigned int len = _boxItem.levelObjectsmale.size() - _index;
						unsigned int index = getRandomIndex(len,i+1);
						index += _index;
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;
					}
				}
				else
				{
					int _index = findBelow<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsmale,0,_boxItem.levelObjectsmale.size()-1,70);
						
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;						
					}
					else
					{
						unsigned int len = _index - 0 + 1;
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsmale[index].obj;
					}
						
					
				}*/

			}

			else
			{
				level -= 10;
				int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsfamale,0,_boxItem.levelObjectsfamale.size()-1,70);
					
				if(_index == -1)
				{
					unsigned int len = _boxItem.levelObjectsmale.size();
					unsigned int index = getRandomIndex(len,i+1);
					_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;						
				}
				else
				{
					unsigned int len = _boxItem.levelObjectsfamale.size() - _index;
					unsigned int index = getRandomIndex(len,i+1);
					index += _index;
					_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;
				}
				/*if(level >= 80)
				{
					int _index = findAbove<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsfamale,0,_boxItem.levelObjectsfamale.size()-1,70);
					
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;						
					}
					else
					{
						unsigned int len = _boxItem.levelObjectsfamale.size() - _index;
						unsigned int index = getRandomIndex(len,i+1);
						index += _index;
						_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;
					}
				}
				else
				{
					int _index = findBelow<std::vector<boxItem::_levelObj> >(_boxItem.levelObjectsfamale,0,_boxItem.levelObjectsfamale.size()-1,70);
					
					if(_index == -1)
					{
						unsigned int len = _boxItem.levelObjectsmale.size();
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;						
					}
					else
					{
						unsigned int len = _index - 0 + 1;
						unsigned int index = getRandomIndex(len,i+1);
						_box->_boxObjects[i] = _boxItem.levelObjectsfamale[index].obj;
					}
						
					
				}				*/
			}


			//unsigned int len = _boxItem._boxObjects.size();
			//Zebra::logger->error("len = %u",len);
			//unsigned int index = getRandomIndex(len,i+1);
			//Zebra::logger->error("index = %u",index);
			//_box->items[i] = _boxItem._boxObjects[index].obj_id;
			//Zebra::logger->error("id = %u",_boxItem._boxObjects[index].obj_id);
			//_box->level[i] = _boxItem._boxObjects[index].level;
			//_box->_boxObjects[i] = _boxItem._boxObjects[index];
		}
	}


	/*for(int i = 0; i < 17; ++i)
	{
		Zebra::logger->error("print id = %u",_box->_boxObjects[i].obj_id);
	}*/

	return _box;
}

void boxCircle::createBoxCircle()
{
	//创建银合子
	//fprintf(stderr,"createBox\n");
	for(int i = 1; i <= 11; ++i)
	silverBoxCircle[i-1] = i;
	//洗牌
	//fprintf(stderr,"createBox2\n");
	for(int i = 0; i < 11; ++i)
	{
		unsigned int index1 = getRandomIndex(10,i);
		unsigned int index2 = getRandomIndex(10,index1);
		//Zebra::logger->error(" index %u ",index1);
		//Zebra::logger->error(" index %u ",index2);
		_swap(silverBoxCircle[index1],silverBoxCircle[index2+1]);
		_swap(silverBoxCircle[index1+1],silverBoxCircle[index2]);
	}

	/*Zebra::logger->error("------silver box----------- ");

	for(int i = 0; i < 11; ++i)
	{
		Zebra::logger->error(" %d ",silverBoxCircle[i]);
	}
	Zebra::logger->error("------end silver box----------- ");
*/


	//创建金盒子

	unsigned int total = 0;
	std::vector<struct qualitys>::iterator it = globalBox::getInstance().glodenBox._qualitys.begin();
	std::vector<struct qualitys>::iterator end = globalBox::getInstance().glodenBox._qualitys.end();

	for( ; it != end; ++it)
		total += (*it).qualityodds;


    unsigned int count = 0;
	it = globalBox::getInstance().glodenBox._qualitys.begin();
	end = globalBox::getInstance().glodenBox._qualitys.end();
	for( ; it != end; ++it)
	{
		int _t = ((*it).qualityodds  * 100) / total;
		for( int i = 0; i < _t; ++i)
			glodenBoxCircle[count++] = (*it).quality;

	}

	if(count <= 99)
	{
		it = globalBox::getInstance().glodenBox._qualitys.begin();
		while(count < 100)
			glodenBoxCircle[count++] = (*it).quality;
	}



	//洗牌

	for(int i = 0; i < 100; ++i)
	{
		unsigned int index1 = getRandomIndex(99,i);
		unsigned int index2 = getRandomIndex(99,index1);
		//Zebra::logger->error(" index %u ",index1);
		//Zebra::logger->error(" index %u ",index2);
		_swap(glodenBoxCircle[index1],glodenBoxCircle[index2+1]);
		_swap(glodenBoxCircle[index1+1],glodenBoxCircle[index2]);
	}

	/*Zebra::logger->error("------glod box----------- ");

	for(int i = 0; i < 100; ++i)
	{
		Zebra::logger->error(" %d ",glodenBoxCircle[i]);
	}

	Zebra::logger->error("------end glod box----------- ");
*/



	//globalBox::getInstance().glodenBox;
    





}
