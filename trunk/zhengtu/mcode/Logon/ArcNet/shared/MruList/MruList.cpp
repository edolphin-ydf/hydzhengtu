// MruList.cpp : �������̨Ӧ�ó������ڵ㡣
//
//����
//#include "stdafx.h"
//#include <time.h>
//#include "MapTemplate.h"
//
//struct _Data
//{
//	int a;
//};
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	/*
//	typedef map<int, _Data*> MapData;
//	MapData mapData;
//	clock_t ctBegin;
//	clock_t ctEnd;
//
//	ctBegin = clock();
//
//	for(int i = 0; i < 1000000; i++)
//	{
//		_Data* p1 = new _Data();
//		mapData.insert(MapData::value_type(i, p1));
//	}
//
//	ctEnd = clock();
//
//	printf("[Main]time is %f.\n", (double)(ctEnd - ctBegin)/CLOCKS_PER_SEC);
//	*/
//
//	//CMapTemplate<int, _Data> MapData(3);
//	McMruContainter<int, _Data> MapData(3);
//	clock_t ctBegin;
//	clock_t ctEnd;
//
//	ctBegin = clock();//���ص��Ǻ��룬���߱�ʾӲ���δ�Ĵ�����1000����1��
//
//	for(int i = 0; i < 100000; i++)
//	{
//		_Data* p1 = new _Data();
//		MapData.AddMapData(i, p1);
//	}
//
//	ctEnd = clock();
//
//	printf("[Main]time is %f.\n", (double)(ctEnd - ctBegin)/CLOCKS_PER_SEC);
//
//
//	_Data* pt = MapData.SearchMapData(1);
//	if(pt == NULL)
//	{
//		printf("[Main]pt not find.\n");
//	}
//
//	int nSize = MapData.GetSize();
//	printf("[Main]Size = %d.\n", nSize);
//	MapData.Clear();
//
//	getchar();
//
//	return 0;
//}

