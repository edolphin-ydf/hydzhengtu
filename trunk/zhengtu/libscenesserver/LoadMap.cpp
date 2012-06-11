#include <zebra/ScenesServer.h>

/**
 * \brief ��ȡ��ͼ�ļ�
 * \param pszFileName: ��ͼ�ļ���
 * \param aTiles: ��ͼ��
 * \param width: ��ͼ���
 * \param height: ��ͼ�߶�
 * \return ����ɹ�����true���򷵻�false
 */
bool LoadMap(const char* pszFileName,zTiles &aTiles,DWORD & width,DWORD & height)
{
  // aTiles : ��ά����,��������

  static int i = 0;
  fprintf(stderr,"���ص�ͼ%d �ļ���: %s\n",++i, pszFileName);
  stMapFileHeader hdr;
  FILE* fp = fopen(pszFileName,"rb");
  if (!fp) return false;
  fread(&hdr,sizeof(hdr),1,fp);
  //if (hdr.magic != MAP_MAGIC || hdr.ver != MAP_VERSION)  
  //{
  //  Zebra::logger->debug("���ص�ͼ %s �汾����ȷ",pszFileName);
  //  fclose(fp);
  //  return false;
  //}

  width = hdr.width;
  height = hdr.height;

  aTiles.resize(width * height);
  fread(&aTiles[0],aTiles.size() * sizeof(stSrvMapTile),1,fp);

  fclose(fp);
  return true;
}
