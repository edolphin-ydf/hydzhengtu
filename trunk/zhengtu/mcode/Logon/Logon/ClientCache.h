//////////////////////////////////////////////////
/// @file : ClientCache.h
/// @brief : 用于内网连接缓存服务器
/// @date:  2012/10/22
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __ClientCache_H__
#define __ClientCache_H__

#include "../shared/Network/MNetSocket.h"
////////////////////////////////////////////////////////////////
/// @class ClientCache
/// @brief 用于内网连接缓存服务器
///
/// @note 继承于socket
class ClientCache : public MCodeNetSocket
{
public:
	ClientCache(SOCKET fd);
	~ClientCache();

	void _HandlePacket();                            /**< 纯虚接口  */

	
public:
	
};
#endif