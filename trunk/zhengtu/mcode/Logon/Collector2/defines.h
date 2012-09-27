#ifndef _DEFINES_H_INCLUDE_
#define _DEFINES_H_INCLUDE_

#include <string>

#define IDD_MESSAGE_CONNECT             400         // 用户连接消息
#define IDD_MESSAGE_CONNECT_SUCESS      401         // 用户连接成功
#define IDD_MESSAGE_CONNECT_FAIL        402         // 用户连接失败
#define IDD_MESSAGE_CONNECT_EXIST       403         // 用户连接存在

/*********************************************************************************************/

#define IDD_MESSAGE_CENTER_LOGIN               300         // 用户登录消息
#define IDD_MESSAGE_CENTER_LOGIN_SUCESS        301         // 用户登录成功
#define IDD_MESSAGE_CENTER_LOGIN_FAIL          302         // 用户登录失败
#define IDD_MESSAGE_CENTER_LOGIN_BUSY          303         // 系统忙,用户登录过于频繁

/*********************************************************************************************/

#define IDD_MESSAGE_UPDATE_GAME_SERVER          7000                   // 更新游戏服务器信息
#define IDD_MESSAGE_UPDATE_GAME_SUCCESS         7001                   // 更新游戏服务器信息成功
#define IDD_MESSAGE_UPDATE_GAME_FAIL            7002                   // 更新游戏服务器信息失败

/*********************************************************************************************/

#define IDD_MESSAGE_REGISTER_GAME               6000                   // 注册游戏
#define IDD_MESSAGE_REGISTER_SUCCESS            6001                   // 注册游戏成功
#define IDD_MESSAGE_REGISTER_FAIL               6002                   // 注册游戏失败
#define IDD_MESSAGE_RE_REGISTER                 6003                   // 重复注册
#define IDD_MESSAGE_GET_GAMESERVER              6004                   // 得到游戏服务器列表

#endif
