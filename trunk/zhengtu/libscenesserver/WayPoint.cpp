#include <zebra/ScenesServer.h>

/**
* \brief ���캯��������Լ�
*/
WayPoint::WayPoint()
{
	bzero(this,sizeof(WayPoint));
}

/**
* \brief ��ʼ������xml��ת�������ļ�
* \param parser ������
* \param node xml�ڵ�
* \param countryid ����id
* \return true ���سɹ� false ����ʧ��
*/
bool WayPoint::init(zXMLParser *parser,const xmlNodePtr node,DWORD countryid)
{
	if (parser==NULL || node==NULL) return false;
	xmlNodePtr cnode=parser->getChildNode(node,NULL);
	while(cnode!=NULL)
	{
		bool ret=false;
		if (xmlStrcmp(cnode->name,(const xmlChar *)"point")==0)
		{
			zZone zone;
			if (parser->getNodePropNum(cnode,"x",&zone.pos.x,sizeof(zone.pos.x)) &&
				parser->getNodePropNum(cnode,"y",&zone.pos.y,sizeof(zone.pos.y)) &&
				parser->getNodePropNum(cnode,"width",&zone.width, sizeof(zone.width)) &&
				parser->getNodePropNum(cnode,"height",&zone.height,sizeof(zone.height)))
			{
				ret=true;
				point.push_back(zone);
				pointC++;
			}
		}
		else if (xmlStrcmp(cnode->name,(const xmlChar *)"dest")==0)
		{
			zPoint p;
			char tempName[MAX_NAMESIZE];
			if (parser->getNodePropNum(cnode,"x",&p.pos.pos.x,sizeof(p.pos.pos.x)) &&
				parser->getNodePropNum(cnode,"y",&p.pos.pos.y,sizeof(p.pos.pos.y)) &&
				parser->getNodePropStr(cnode,"name",tempName,MAX_NAMESIZE) &&
				parser->getNodePropNum(cnode,"width",&p.pos.width, sizeof(p.pos.width)) &&
				parser->getNodePropNum(cnode,"height",&p.pos.height,sizeof(p.pos.height)))
			{
				std::string fileName; 
				char temp[20];
				bzero(temp,sizeof(temp));
				DWORD country=0;

				/// Ϊ��֧�ֹ������ҵĸ���^_^
				parser->getNodePropNum(cnode,"country",&country,sizeof(country));
				if (country)
				{
					sprintf(temp,"%d",country);
				}
				else
				{
					sprintf(temp,"%d",countryid);
				}
				fileName = temp;
				fileName+= ".";
				fileName+= tempName;
				strncpy(p.name,fileName.c_str(),MAX_NAMESIZE);
				ret=true;
				dest.push_back(p);
				destC++;
			}
		}
		if (ret)
			cnode=parser->getNextNode(cnode,NULL);
		else
		{
			//return false;
			Zebra::logger->warn("xml��ת�������д�������");
			cnode=parser->getNextNode(cnode,NULL);
		}
	}
	return true;
}

/**
* \brief �����һ����ת��
* \return ��ת��
*/
const zPos WayPoint::getRandPoint()
{
	return point[randBetween(0,pointC-1)].GetRandPos();
}

/**
* \brief ���һ�������Ŀ�ĵ�
* \return Ŀ�ĵ�
*/
const Point WayPoint::getRandDest()
{
	Point m_Point;
	zPoint zPt = dest[randBetween(0,destC-1)];
	m_Point.pos = zPt.pos.GetRandPos();
	strncpy(m_Point.name,zPt.name, sizeof( m_Point.name ));
	return m_Point;
}

/**
* \brief ����һ����ת��
* \param wp ��ת��
* \return true 
*/
bool WayPointM::addWayPoint(const WayPoint &wp)
{
	wps.push_back(wp);
	return true;
}

/**
* \brief ��������ת��
* \return ���û����ת���򷵻�NULL,���򷵻���ת�㡣
*/
WayPoint *WayPointM::getRandWayPoint()
{
	if (!wps.empty())
		return &wps[randBetween(0,wps.size()-1)];
	else
		return NULL;
}

/**
* \brief ����Ŀ�ĵ����ƻ�ȡ��ת��
* \param filename Ŀ�ĵ�����
* \return ��ת�����
*/
WayPoint *WayPointM::getWayPoint(const char *filename)
{
	for(int i=0,n=wps.size();i<n;i++)
	{
		for(int j=0;j<wps[i].destC;j++)
			if (strncmp(wps[i].dest[j].name,filename,MAX_NAMESIZE)==0)
			{
				return &wps[i];
			}
	}
	return NULL;
}

/**
* \brief ���ָ��λ���ϵ���ת�����
* \param pos λ��
* \return ��ת�����
*/
WayPoint *WayPointM::getWayPoint(const zPos &pos)
{
	for(int i=0,n=wps.size();i<n;i++)
	{
		for(int j=0;j<wps[i].pointC;j++)
			if (wps[i].point[j].IsInZone(pos))
			{
				return &wps[i];
			}
	}
	return NULL;
}
