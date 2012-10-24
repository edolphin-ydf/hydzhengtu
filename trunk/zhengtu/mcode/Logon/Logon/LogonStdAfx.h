#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <string>

#include "Common.h"
#include <Network/Network.h>

#include "shared/Log.h"
#include "shared/Util.h"
#include "shared/ByteBuffer.h"
#include "shared/Config/ConfigEnv.h"
#include <zlib.h>

//#include "shared/Database/DatabaseEnv.h"

#include "shared/Auth/BigNumber.h"
#include "shared/Auth/Sha1.h"
#include "shared/Auth/PacketCrypt.h"

#include "IntranetManager.h"
#include "ServerOpcodes.h"
#include "Main.h"
#include "PeriodicFunctionCall_Thread.h"
#include "InfoCore.h"
#include "AuthSocket.h"
#include "AuthStructs.h"
#include "LogonCommServer.h"
#include "LogonConsole.h"
#include "WorldPacket.h"
#include "global.h"
#define CONFDIR "configs"

