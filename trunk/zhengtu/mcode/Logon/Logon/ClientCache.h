//////////////////////////////////////////////////
/// @file : ClientCache.h
/// @brief : �����������ӻ��������
/// @date:  2012/10/22
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __ClientCache_H__
#define __ClientCache_H__

#include "../shared/Network/MNetSocket.h"
////////////////////////////////////////////////////////////////
/// @class ClientCache
/// @brief �����������ӻ��������
///
/// @note �̳���socket
class ClientCache : public MCodeNetSocket
{
public:
	ClientCache(SOCKET fd);
	~ClientCache();

	void _HandlePacket();                            /**< ����ӿ�  */

	
public:
	
};
#endif