/**
* \brief 存储有效服务器的列表
* 有效服务器列表存储在xml文件中,服务器启动的时候读取这些信息到内存,
* 当一个管理服务器连接过来的时候,可以根据这些信息判断这个连接是否合法的。
*/
#include "FLServer.h"

bool ServerACL::add(const ACLZone &zone)
{
	Zebra::logger->debug("ServerACL::add");
	Container::const_iterator it = datas.find(zone.gameZone);
	if (it == datas.end())
	{
		std::pair<Container::iterator,bool> p = datas.insert(Container::value_type(zone.gameZone,zone));
		return p.second;
	}
	else
		return false;
}

bool ServerACL::init()
{
	Zebra::logger->debug("ServerACL::init");
	zRWLock_scope_wrlock scope_wrlock(rwlock);
	datas.clear();

	zXMLParser xml;
	if (!xml.initFile(Zebra::global["confdir"] + "zoneInfo.xml"))
	{
		Zebra::logger->error("加载zoneInfo.xml失败");
		return false;
	}

	xmlNodePtr root = xml.getRootNode("zoneInfo");
	if (root)
	{
		xmlNodePtr node = xml.getChildNode(root,"zone");
		while(node)
		{
			if (0 == strcmp((char *)node->name,"zone"))
			{
				ACLZone zone;
				xml.getNodePropNum(node,"game",&zone.gameZone.game,sizeof(zone.gameZone.game));
				xml.getNodePropNum(node,"zone",&zone.gameZone.zone,sizeof(zone.gameZone.zone));
				xml.getNodePropStr(node,"ip",zone.ip);
				xml.getNodePropNum(node,"port",&zone.port,sizeof(zone.port));
				xml.getNodePropStr(node,"name",zone.name);
				xml.getNodePropStr(node,"desc",zone.desc);
				if (!add(zone))
				{
					Zebra::logger->warn("game=%u,zone=%u,ip=%s,port=%u,name=%s,desc=%s",
						zone.gameZone.game,
						zone.gameZone.zone,
						zone.ip.c_str(),
						zone.port,
						zone.name.c_str(),
						zone.desc.c_str());
				}
			}

			node = xml.getNextNode(node,NULL);
		}
	}

	Zebra::logger->info("加载区信息列表文件成功");
	return true;
}

void ServerACL::final()
{
	Zebra::logger->debug("ServerACL::final");
	zRWLock_scope_wrlock scope_wrlock(rwlock);
	datas.clear();
}

bool ServerACL::check(const char *strIP,const WORD port,GameZone_t &gameZone,std::string &name)
{
	Zebra::logger->debug("ServerACL::check");
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	for(Container::const_iterator it = datas.begin(); it != datas.end(); ++it)
	{
		if (0 == strcmp(it->second.ip.c_str(),strIP)
			&& it->second.port == port)
		{
			gameZone = it->first;
			name = it->second.name;
			return true;
		}
	}
	return false;
}

