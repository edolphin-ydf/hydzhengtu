/**
 * \brief ��ͼ�������������
 */

#include <zebra/ScenesServer.h>

/**
 * \brief ���ó������
 * \param scenewh �����Ŀ��
 * \param screenMax ��������
 */
void zSceneEntryIndex::setSceneWH(const zPos sceneWH,const DWORD screenx,const DWORD screeny,const DWORD screenMax)
{
  this->sceneWH=sceneWH;
  this->screenx=screenx;
  this->screeny=screeny;
  for(int i = 0; i < zSceneEntry::SceneEntry_MAX; i++)
  {
    for(DWORD j=0; j < screenMax; j ++)
    {
      index[i][j];
    }
  }

  //Ԥ�Ƚ�����ͼ��������
  const int adjust[9][2] = { {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,0} };
  for(DWORD j=0; j < screenMax ; j ++)
  {
    int nScreenX = j % screenx;
    int nScreenY = j / screenx;
    //Zebra::logger->debug("%u,%u,%u",screenMax,nScreenX,nScreenY);
    //������Χ����
    {
      zPosIVector pv;
      for(int i = 0; i < 9; i++) {
        int x = nScreenX + adjust[i][0];
        int y = nScreenY + adjust[i][1];
        if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny) {
          pv.push_back(y * screenx + x);
        }
      }
      ninescreen.insert(NineScreen_map_value_type(j,pv));
    }
    //��������仯������������
    for(int dir = 0; dir < 8; dir++)
    {
      int start,end;
      zPosIVector pv;

      if (1 == dir % 2) {
        //б����
        start = 6;
        end = 10;
      }
      else {
        //������
        start = 7;
        end = 9;
      }
      for(int i = start; i <= end; i++) {
        int x = nScreenX + adjust[(i + dir) % 8][0];
        int y = nScreenY + adjust[(i + dir) % 8][1];
        if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny) {
          pv.push_back(y * screenx + x);
        }
      }
      direct_screen[dir].insert(NineScreen_map_value_type(j,pv));
    }
    //���㷴��仯������������
    for(int dir = 0; dir < 8; dir++)
    {
      int start,end;
      zPosIVector pv;

      if (1 == dir % 2) {
        //б����
        start = 2;
        end = 6;
      }
      else {
        //������
        start = 3;
        end = 5;
      }
      for(int i = start; i <= end; i++) {
        int x = nScreenX + adjust[(i + dir) % 8][0];
        int y = nScreenY + adjust[(i + dir) % 8][1];
        if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny) {
          pv.push_back(y * screenx + x);
        }
      }
      reversedirect_screen[dir].insert(NineScreen_map_value_type(j,pv));
    }
  }
}

/**
 * \brief ˢ�³������
 * ���û��������,�����ˢ��
 * ���뱣֤������ĺϷ���,����������治����֤����ĺϷ���
 * \param e Ҫˢ�µĳ������
 * \param newPos �����������
 * \return ˢ���Ƿ�ɹ�
 */
bool zSceneEntryIndex::refresh(zSceneEntry *e,const zPos & newPos)
{
  if (e==NULL) return false;
  zSceneEntry::SceneEntryType type = e->getType();
  if (e->inserted)
  {
    //�Ѿ������ͼ,ֻ������֮�������л�
    bool ret=false;
    zPosI orgscreen=e->getPosI();

    SceneEntry_SET &pimi = index[type][orgscreen];
    SceneEntry_SET::iterator it = pimi.find(e);
    if (it != pimi.end() && e->setPos(sceneWH,newPos))
    {
      ret=true;
      if (orgscreen!=e->getPosI())
      {
        //Zebra::logger->debug("%s ����(%ld->%ld)",e->name,orgscreen,e->getPosI());
        pimi.erase(it);
        index[type][e->getPosI()].insert(e);
        if (type == zSceneEntry::SceneEntry_Player)
        {
          freshEffectPosi(orgscreen,e->getPosI());
          freshGateScreenIndex((SceneUser*)e,e->getPosI());
        }
      }
    }

    return ret;
  }
  else
  {
    //�¼����ͼ
    if (e->setPos(sceneWH,newPos))
    {
      index[type][e->getPosI()].insert(e);
      //��ȫ�����������
      all[type].insert(e);
      //��npc���������
      if (zSceneEntry::SceneEntry_NPC == type )
      {
        SceneNpc *npc = (SceneNpc *)e;
        special_index[npc->id].insert(npc);
        //�ڹ���npc���������
        if (npc->isFunctionNpc())
        {
          functionNpc.insert(npc);
        }
      }
      if (type == zSceneEntry::SceneEntry_Player)
      {
        freshEffectPosi((zPosI)-1,e->getPosI());
        freshGateScreenIndex((SceneUser*)e,e->getPosI());
      }

      e->inserted=true;
      //Zebra::logger->debug("%s(%x) really inserted into scene entry index",e->name,e);
      //Zebra::logger->debug("%s �����ͼ(%d,%d,%d)",e->name,e->getPosI(),e->getPos().x,e->getPos().y);
    }
    else
      Zebra::logger->debug("������������ %s ʧ�� (%u,%u)",e->name,newPos.x,newPos.y);

    return e->inserted;
  }
}

/**
 * \brief �Ƴ��������
 * \param e Ҫ�Ƴ��ĳ������
 * \return  true�Ƴ��ɹ�,false ʧ��
 */
bool zSceneEntryIndex::removeSceneEntry(zSceneEntry *e)
{
  if (e==NULL || !e->inserted) return false;

  zSceneEntry::SceneEntryType type = e->getType();
  SceneEntry_SET &pimi = index[type][e->getPosI()];
  SceneEntry_SET::iterator it = pimi.find(e);
  if (it != pimi.end())
  {
    //����������ɾ��
    pimi.erase(it);
    //��ȫ��������ɾ��
    all[type].erase(e);
    //��npc������ɾ��
    if (zSceneEntry::SceneEntry_NPC == type )
    {
      SceneNpc *npc = (SceneNpc *)e;
      special_index[npc->id].erase(npc);
      //�ڹ���npc������ɾ��
      if (npc->isFunctionNpc())
      {
        functionNpc.erase(npc);
      }
    }
    if (type == zSceneEntry::SceneEntry_Player)
    {
      freshEffectPosi(e->getPosI(),(zPosI)-1);
      freshGateScreenIndex((SceneUser*)e,(DWORD)-1); //-1��ʾ�ӵ�ǰ��������ɾ��
    }
    e->inserted=false;
    //Zebra::logger->debug("%s(%x) really removed from scene entry index",e->name,e);
    return true;
  }

  return false;
}

/**
 * \brief ����һ�������,���������
 * ��������,�������е��������
 * \param screen ĳһ���������
 * \param callback Ҫ��������в����Ļص���
 */
void zSceneEntryIndex::execAllOfScreen(const zPosI screen,zSceneEntryCallBack &callback)
{
  for(int i = 0; i < zSceneEntry::SceneEntry_MAX; i++)
  {
    SceneEntry_SET &pimi = index[i][screen];
    for(SceneEntry_SET::iterator it=pimi.begin();it!=pimi.end();)
    {
      //Ԥ�ȱ��������,��ֹ�ص���ʹ������ʧЧ
      SceneEntry_SET::iterator tmp = it;
      it++;
      zSceneEntry *eee = *tmp;
      if (!callback.exec(eee)) return;
    }
  }
}

/**
 * \brief ����һ�������,���������
 * �����ض����͵��������
 * \param type �������
 * \param screen ĳһ���������
 * \param callback Ҫ��������в����Ļص���
 */
void zSceneEntryIndex::execAllOfScreen(const zSceneEntry::SceneEntryType type,const zPosI screen,zSceneEntryCallBack &callback)
{
  SceneEntry_SET &pimi = index[type][screen];
  for(SceneEntry_SET::iterator it=pimi.begin();it!=pimi.end();)
  {
    //Ԥ�ȱ��������,��ֹ�ص���ʹ������ʧЧ
    SceneEntry_SET::iterator tmp = it;
    it++;
    zSceneEntry *eee = *tmp;
    if (!callback.exec(eee)) return;
  }
}

/**
 * \brief ����һ�������,���������
 * ��������,�������е��������
 * \param callback Ҫ��������в����Ļص���
 */
void zSceneEntryIndex::execAllOfScene(zSceneEntryCallBack &callback)
{
  for(int i = 0; i < zSceneEntry::SceneEntry_MAX; i++)
  {
    for(SceneEntry_SET::iterator it = all[i].begin(); it != all[i].end();)
    {
      //Ԥ�ȱ��������,��ֹ�ص���ʹ������ʧЧ
      SceneEntry_SET::iterator tmp = it;
      it++;
      zSceneEntry *eee = *tmp;
      if (!callback.exec(eee)) return;
    }
  }
}

/**
 * \brief ����һ�������,���������
 * �����ض����͵��������
 * \param type �������
 * \param callback Ҫ��������в����Ļص���
 */
void zSceneEntryIndex::execAllOfScene(const zSceneEntry::SceneEntryType type,zSceneEntryCallBack &callback)
{
  for(SceneEntry_SET::iterator it = all[type].begin(); it != all[type].end();)
  {
    //Ԥ�ȱ��������,��ֹ�ص���ʹ������ʧЧ
    SceneEntry_SET::iterator tmp = it;
    it++;
    zSceneEntry *eee = *tmp;
    if (!callback.exec(eee)) return;
  }
}

/**
 * \brief ������ͼ�������ض����͵�npc,������ûص�����
 * ע���˻ص�ֻ�ṩ����ִ��,���ṩɾ������
 * \param id npc����
 * \param callback �ص�����
 */
void zSceneEntryIndex::execAllOfScene_npc(const DWORD id,zSceneEntryCallBack &callback)
{
  SpecialNpc_Index::iterator my_it = special_index.find(id);
  if (special_index.end() != my_it)
  {
    for(Npc_Index::iterator it = my_it->second.begin(); it != my_it->second.end(); ++it)
    {
      if (!callback.exec(*it)) return;
    }
  }
}

/**
 * \brief ������ͼ�����й���npc,������ûص�����
 * ע���˻ص�ֻ�ṩ����ִ��,���ṩɾ������
 * \param callback �ص�����
 */
void zSceneEntryIndex::execAllOfScene_functionNpc(zSceneEntryCallBack &callback)
{
  for(Npc_Index::iterator it = functionNpc.begin(); it != functionNpc.end(); ++it)
  {
    if (!callback.exec(*it)) return;
  }
}

/**
 * \brief ��������,��ȡվ��������������ĵ�ͼ���
 * \param type �������
 * \param pos �����
 * \param bState �Ƿ��ж����״̬
 * \param byState ���״̬
 * \return ��ͼ���
 */
zSceneEntry *zSceneEntryIndex::getSceneEntryByPos(zSceneEntry::SceneEntryType type,const zPos &pos,const bool bState,const zSceneEntry::SceneEntryState byState)
{
  class GetSceneEntryByPos : public zSceneEntryCallBack
  {
    public:
      const zPos &pos;
      zSceneEntry *sceneEntry;
      bool _bState;
      zSceneEntry::SceneEntryState _byState;

      GetSceneEntryByPos(const zPos &pos,const bool bState,const zSceneEntry::SceneEntryState byState)
        : pos(pos),sceneEntry(NULL),_bState(bState),_byState(byState) {}
      bool exec(zSceneEntry *e)
      {
        if (e->getPos() == pos)
        {
          // ��������״̬������״̬��Entry������������,���Լ�������ɴ�״̬�ļ���
          if (_bState && e->getState() != _byState)
            return true;
          //ʹ����λ��ȡͼ���������npc�޷�ȡ��
          if (e->getType() == zSceneEntry::SceneEntry_NPC
              && ((SceneNpc *)e)->getPetType() == Cmd::PET_TYPE_TOTEM)
          {
            return true;
          }
          sceneEntry = e;
          return false;
        }
        else
          return true;
      }
  } ret(pos,bState,byState);
  zPosI screen;
  zSceneEntry::zPos2zPosI(sceneWH,pos,screen);
  execAllOfScreen(type,screen,ret);
  return ret.sceneEntry;
}


/**
 * \brief ά��Ӱ��npc����
 * \param oldscreen �û�����ǰ���(-1��ʾֻ����)
 * \param newscreen �û���������(-1��ʾֻɾ��)
 */
void zSceneEntryIndex::freshEffectPosi(const zPosI oldposi,const zPosI newposi)
{
  if (oldposi != (zPosI)-1)
  {
    const zPosIVector &pv = getNineScreen(oldposi);
    for(zPosIVector::const_iterator it = pv.begin(); it != pv.end(); it++)
    {
      PosiEffectMap_iter iter = posiEffect[(*it)%MAX_NPC_GROUP].find(*it);
      if (iter != posiEffect[(*it)%MAX_NPC_GROUP].end() && iter->second > 0)
      {
        iter->second--;
        if (iter->second == 0)
        {
          posiEffect[(*it)%MAX_NPC_GROUP].erase(iter);
          //Zebra::logger->debug("ɾ����Ч������%d",iter->first);
        }
      }
    }
  }
  if (newposi != (zPosI)-1)
  {
    const zPosIVector &pv = getNineScreen(newposi);
    for(zPosIVector::const_iterator it = pv.begin(); it != pv.end(); it++)
    {
      posiEffect[(*it)%MAX_NPC_GROUP][*it]++;
      //Zebra::logger->debug("�����Ч������%d,��Ч����%d",*it,posiEffect[(*it)%MAX_NPC_GROUP][*it]);
    }
  }
}

void zSceneEntryIndex::execAllOfEffectNpcScreen(const DWORD group,zSceneEntryCallBack &callback)
{
  PosiEffectMap_iter iter = posiEffect[group%MAX_NPC_GROUP].begin();
  for( ;iter != posiEffect[group%MAX_NPC_GROUP].end() ; iter++)
  {
    /*
    //Ԥ�ȱ��������,��ֹ�ص���ʹ������ʧЧ
    PosiEffectMap_iter iter_tmp = iter; 
    iter++;
    SceneEntry_SET &pimi = index[zSceneEntry::SceneEntry_NPC][iter_tmp->first];
    // */
    // ������������ƶ�,ֻҪ������9��,�Ͳ�������������Ч,���ǿ��ܻ�����iter++����Ч,
    // ���Բ�����++�ٴ���
    SceneEntry_SET &pimi = index[zSceneEntry::SceneEntry_NPC][iter->first];
    for(SceneEntry_SET::iterator it=pimi.begin();it!=pimi.end();)
    {
      //Ԥ�ȱ��������,��ֹ�ص���ʹ������ʧЧ
      SceneEntry_SET::iterator tmp = it;
      it++;
      zSceneEntry *eee = *tmp;
      if (!callback.exec(eee)) return;
    }
  }
}

/**
 * \brief �õ�ĳһλ��һ����Χ�ڵ����б�
 * \param pos ָ������
 * \param range ָ����Χ
 * \return ���б�
 */
const zPosIVector &zSceneEntryIndex::getScreenByRange(const zPos &pos,const int range)
{
  static zPosIVector pv;
  pv.clear();
  int scnX = pos.x / SCREEN_WIDTH;//�����
  int scnY = pos.y / SCREEN_HEIGHT;
  int offX = pos.x % SCREEN_WIDTH;//��һ���е�λ��
  int offY = pos.y % SCREEN_HEIGHT;

  int x=0,y=0;

  //����
  x = scnX;
  y = scnY;
  if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
    pv.push_back(y * screenx + x);

  if (offX<range)
  {
    //��
    x = scnX - 1;
    y = scnY;
    if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
      pv.push_back(y * screenx + x);

    if (offY<range)
    {
      //��
      x = scnX;
      y = scnY - 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);

      //����
      x = scnX - 1;
      y = scnY - 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);
    }

    if (offY+range>SCREEN_HEIGHT)
    {
      //��
      x = scnX;
      y = scnY + 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);

      //����
      x = scnX - 1;
      y = scnY + 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);
    }
  }

  if (offX+range>SCREEN_WIDTH)
  {
    //��
    x = scnX + 1;
    y = scnY;
    if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
      pv.push_back(y * screenx + x);

    if (offY<range)
    {
      //��
      x = scnX;
      y = scnY - 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);

      //����
      x = scnX + 1;
      y = scnY - 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);
    }

    if (offY+range>SCREEN_HEIGHT)
    {
      //��
      x = scnX;
      y = scnY + 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);

      //����
      x = scnX + 1;
      y = scnY + 1;
      if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny)
        pv.push_back(y * screenx + x);
    }
  }
  return pv;
}

