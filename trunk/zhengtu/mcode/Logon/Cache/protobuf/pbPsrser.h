//////////////////////////////////////////////////
/// @file : pbPsrser.h
/// @brief : �ȸ�Э�������
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
/// @brief ���������ֶ�̬������Ϣ�ķ���
///
/// @note  1������ʱ������Ϣ��2��ͨ��proto����
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
	/** @brief ͨ��proto�ļ����� */
	void CreateProtoByFile(const char * path, const char * proto_file);
	/** @brief ֱ�����ڴ��й��� */
	void CreateProtoByName(const char *proto_name);
	/** @brief ���һ�����͵���Ϣ */
	void AddTypeMeg(const char * msg_name, vector<pb_msg_item *> &keys);
	/** @brief �����Ϣ��� */
	void FinishAdd();
	/** @brief ��������Ϣ,��proto�ļ���ȡʱ������а�����
	����package tutorial����Ҫ����tutorial.Person */
	google::protobuf::Message *CreateMeg(const char * msg_name);

	/** @brief Ϊ��Ϣ�Ķ���ֵ */
	void SetMegValue(google::protobuf::Message *msg,const char *key,int value);
	void SetMegValue(google::protobuf::Message *msg,const char *key,string value);
	void SetMegValue(google::protobuf::Message *msg,const char *key,float value);

	string GetMegString(google::protobuf::Message *msg);
public:
	static pbPsrser* GetPBInterface();
};


#endif