//#include <zebra/SceneServer.h>
#include <zebra/srvEngine.h>
#include "giftBox.h"
#include <algorithm>
#include <functional>

//#include "globalBox.h"

void globalBox::addBoxItem(boxItem _boxItem)
{
	//Zebra::logger->error("_boxItem.qualityID = %u\n",_boxItem.qualityID);

	_boxItems[_boxItem.qualityID] = _boxItem;
	//_boxItems.push_back(_boxItem);
}

int globalBox::init()
{
	zXMLParser configParser;
	Zebra::logger->error("读取箱子配置文件!\n");
	if(!configParser.initFile("./conf/103_GiftBoxConfig.xml"))
		return -1;


	xmlNodePtr rootNode = configParser.getRootNode("giftboxs");
	if(NULL == rootNode)
		return -1;

	xmlNodePtr currentNode = configParser.getChildNode(rootNode,NULL);
	
	while(NULL != currentNode)
	{
		if(!xmlStrcmp(currentNode->name,(const xmlChar *)"item"))
		{


			boxItem _boxItem;
			
			configParser.getNodePropNum(currentNode,"quality",&_boxItem.qualityID,sizeof(_boxItem.qualityID));
			

			xmlNodePtr subNode = configParser.getChildNode(currentNode,"object");

			if(NULL == subNode)
				continue;







			while(subNode != NULL)
			{


				if(!xmlStrcmp(subNode->name,(const xmlChar *)"object"))
				{
					boxObject _boxObject;
				//_boxObject.obj_id = 
				
					configParser.getNodePropNum(subNode,"obj_id",&_boxObject.obj_id,sizeof(_boxObject.obj_id));
					configParser.getNodePropNum(subNode,"num",&_boxObject.num,sizeof(_boxObject.num));
					configParser.getNodePropNum(subNode,"level",&_boxObject.level,sizeof(_boxObject.level));
					configParser.getNodePropNum(subNode,"kind",&_boxObject.kind,sizeof(_boxObject.kind));
					configParser.getNodePropNum(subNode,"itemkind",&_boxObject.itemkind,sizeof(_boxObject.itemkind));
					configParser.getNodePropNum(subNode,"itemlevel",&_boxObject.itemlevel,sizeof(_boxObject.itemlevel));
					configParser.getNodePropNum(subNode,"material",&_boxObject.material,sizeof(_boxObject.material));
					configParser.getNodePropNum(subNode,"sex",&_boxObject.sex,sizeof(_boxObject.sex));

					//Zebra::logger->error("obj_id = %u\n",_boxObject.obj_id);
					
					//std::map<unsigned int,struct boxItem>::iterator it = _boxItem._boxObjects.find();

					//if(it == _boxItem._boxObjects.end())
					//	_boxItem._boxObjects[_boxObject.kind] = new std::vector<boxObject>;

					//_boxItem._boxObjects[_boxObject.kind]->push_back(_boxObject);
					//addBoxItem(_boxItem);



					boxItem::_levelObj lobj;
					//lobj.level = _boxObject.itemlevel;
					lobj.obj = _boxObject;

					if( _boxObject.sex == 1)
						{
							lobj._obj = objectbm.get(_boxObject.obj_id);
							if(lobj._obj)
								_boxItem.levelObjectsmale.push_back(lobj);

						}
					else if(_boxObject.sex == 2)
						{
							lobj._obj = objectbm.get(_boxObject.obj_id);
							if(lobj._obj)
								_boxItem.levelObjectsfamale.push_back(lobj);
						}
					else
						{
							lobj._obj = objectbm.get(_boxObject.obj_id);
							if(lobj._obj)
							{
								_boxItem.levelObjectsmale.push_back(lobj);
								_boxItem.levelObjectsfamale.push_back(lobj);
							}
						}
					

					
					//_boxItem.levelObjects.push_back(lobj);
					//_boxItem._boxObjects.push_back(_boxObject);
				}

				subNode = configParser.getNextNode(subNode,NULL);

				
			}
			//_boxItem.levelObjectsfamale.
			stable_sort(_boxItem.levelObjectsfamale.begin(),_boxItem.levelObjectsfamale.end());

			//Zebra::logger->debug("quality %d",_boxItem.qualityID);
			//std::vector<boxItem::_levelObj>::iterator it = _boxItem.levelObjectsfamale.begin();
			//std::vector<boxItem::_levelObj>::iterator end = _boxItem.levelObjectsfamale.end();
			//for( ; it != end; ++it)
			//{
			//	Zebra::logger->debug("needlevel %d",it->_obj->needlevel);
			//}

			stable_sort(_boxItem.levelObjectsmale.begin(),_boxItem.levelObjectsmale.end()); 
			addBoxItem(_boxItem);
		}
		else if(!xmlStrcmp(currentNode->name,(const xmlChar *)"giftbox"))
		{


			giftBox *targetBox = NULL;
			
			int boxID;
			configParser.getNodePropNum(currentNode,"id",&boxID,sizeof(boxID));


			if(boxID == 1)
				targetBox = &glodenBox;
			else
				targetBox = &silverBox;

			xmlNodePtr subNode = configParser.getChildNode(currentNode,NULL);
			while(subNode != NULL)
			{


				if(!xmlStrcmp(subNode->name,(const xmlChar *)"qualitys"))
				{
					//boxObject _boxObject;
					//_boxObject.obj_id = 
				
					qualitys _qualitys;

					configParser.getNodePropNum(subNode,"quality",&_qualitys.quality,sizeof(_qualitys.quality));
					configParser.getNodePropNum(subNode,"qualityodds",&_qualitys.qualityodds,sizeof(_qualitys.qualityodds));
					configParser.getNodePropNum(subNode,"maxnum",&_qualitys.maxnum,sizeof(_qualitys.maxnum));
					configParser.getNodePropNum(subNode,"numkind",&_qualitys.numkind,sizeof(_qualitys.numkind));
					//Zebra::logger->error("quality = %u\n",_qualitys.quality);
					
					targetBox->_qualitys.push_back(_qualitys);
				}else if(!xmlStrcmp(subNode->name,(const xmlChar *)"boxitem"))
				{
					configParser.getNodePropNum(subNode,"itemid",&targetBox->_boxitem.itemid,sizeof(targetBox->_boxitem.itemid));
					configParser.getNodePropNum(subNode,"size",&targetBox->_boxitem.size,sizeof(targetBox->_boxitem.size));
					configParser.getNodePropNum(subNode,"maxnumber",&targetBox->_boxitem.maxnumber,sizeof(targetBox->_boxitem.maxnumber));
					configParser.getNodePropNum(subNode,"price",&targetBox->_boxitem.price,sizeof(targetBox->_boxitem.price));
					//Zebra::logger->error("itemid = %u\n",targetBox->_boxitem.itemid);
				}

				subNode = configParser.getNextNode(subNode,NULL);

			}
		}

		currentNode = configParser.getNextNode(currentNode,NULL);

	}


	/*std::map<unsigned int,struct boxItem>::iterator boxItemIt = _boxItems.begin();
	std::map<unsigned int,struct boxItem>::iterator boxItemEnd = _boxItems.end();
	for( ; boxItemIt != boxItemEnd; ++boxItemIt)
	{
		Zebra::logger->error("------------------------- qualityID = %u -----------------------\n",(boxItemIt->second).qualityID);

		std::map<unsigned int,std::vector<boxObject>*>::iterator objIt = (boxItemIt->second)._boxObjects.begin();
		std::map<unsigned int,std::vector<boxObject>*>::iterator objEnd = (boxItemIt->second)._boxObjects.end();
		for( ; objIt != objEnd; ++objIt)
		{
			Zebra::logger->error("kind = %u\n",objIt->first);

			std::vector<boxObject>::iterator itv = objIt->second->begin();
			std::vector<boxObject>::iterator itend = objIt->second->end();

			for( ; itv != itend; ++itv)
				Zebra::logger->error("obj_id = %u\n",(*itv).obj_id);
		}
	}*/
	

	return 0;
}