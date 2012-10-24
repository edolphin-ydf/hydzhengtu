//////////////////////////////////////////////////
/// @file : pbPsrser.h
/// @brief : 谷歌协议解析类
/// @date:  2012/10/21
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __pbPsrser_H__
#define __pbPsrser_H__

/** @brief protobuf */
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/compiler/importer.h"



struct pb_msg_item{
	char *key_name;
	::google::protobuf::FieldDescriptorProto_Type key_type;
	::google::protobuf::FieldDescriptorProto_Label key_label;
};

////////////////////////////////////////////////////////////////
/// @class pbPsrser
/// @brief 包含了两种动态生成消息的方法
///
/// @note  1，运行时构建消息；2，通过proto构建
class pbPsrser
{
private:
	google::protobuf::compiler::Importer *importer;
	google::protobuf::FileDescriptorProto *file_proto;
	google::protobuf::DynamicMessageFactory *factory;
	google::protobuf::compiler::DiskSourceTree *sourceTree;
	const google::protobuf::DescriptorPool *pool;
	const google::protobuf::FileDescriptor *file_descriptor;
	const google::protobuf::Descriptor *descriptor;
public:
	pbPsrser()
	{
		importer = NULL;
		file_proto = NULL;
		factory = NULL;
		pool = NULL;
		file_descriptor = NULL;
		descriptor = NULL;
	}
	~pbPsrser()
	{
		release();
		google::protobuf::ShutdownProtobufLibrary();
	}

	void release(){
		if (importer)
		{
			if (sourceTree)
			{
				delete sourceTree;
				sourceTree = NULL;
			}
			delete importer;
			importer = NULL;
		}
		if (file_proto)
		{
			if (pool)
			{
				delete pool;
				pool = NULL;
			}
			delete file_proto;
			file_proto = NULL;
		}
		if (factory)
		{
			delete factory;
			factory = NULL;
		}
	}
	/** @brief 通过proto文件创建 */
	void CreateProtoByFile(const char * path, const char * proto_file);
	/** @brief 直接在内存中构建 */
	void CreateProtoByName(const char *proto_name);
	/** @brief 添加一个类型的消息 */
	void AddTypeMeg(const char * msg_name, vector<pb_msg_item *> &keys);
	/** @brief 添加消息完成 */
	void FinishAdd();
	/** @brief 创建建消息,从proto文件读取时，如果有包名，
	例如package tutorial，需要加上tutorial.Person */
	google::protobuf::Message *CreateMeg(const char * msg_name);

	/** @brief 为消息的对象赋值 */
	void SetMegValue(google::protobuf::Message *msg,const char *key,int value);
	void SetMegValue(google::protobuf::Message *msg,const char *key,string value);
	void SetMegValue(google::protobuf::Message *msg,const char *key,float value);

	string GetMegString(google::protobuf::Message *msg);
public:
	static pbPsrser* GetPBInterface();
};


#endif