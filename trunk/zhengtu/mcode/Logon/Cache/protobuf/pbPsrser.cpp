//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      �ȸ�Э�������
///  Create:      2012/10/21
///
/// @file         pbPsrser.cpp
/// @brief        
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

/*
 * @defgroup ģ���� �ȸ�Э�鶯̬������Ϣ��װ������
 * @{
 */
 /*
	//��һ��
	sCachepb->CreateProtoByName("test.proto");
	pb_msg_item *item = new pb_msg_item;
	item->key_name = "nickname";
	item->key_type = FieldDescriptorProto_Type_TYPE_STRING;
	item->key_label = FieldDescriptorProto_Label_LABEL_REQUIRED;
	vector<pb_msg_item *> items;
	items.push_back(item);
	sCachepb->AddTypeMeg("user",items);
	sCachepb->FinishAdd();
	Message *msg = sCachepb->CreateMeg("user");
	sCachepb->SetMegValue(msg,"nickname","hydtest");
	printf("%s\n",sCachepb->GetMegString(msg).c_str());
	delete msg;
	delete item;
	//�ڶ���
	sCachepb->CreateProtoByFile("./","addressbook.proto");
	Message *msg = sCachepb->CreateMeg("tutorial.Person");
	sCachepb->SetMegValue(msg,"name","hyd");
	sCachepb->SetMegValue(msg,"id",1111);
	sCachepb->SetMegValue(msg,"email","test@1.com");
	printf("%s\n",sCachepb->GetMegString(msg).c_str());
	delete msg;*/

/** @} */ // ģ���β

#include "Common.h"
#include "pbPsrser.h"
using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;

pbPsrser* pbPsrser::GetPBInterface()
{
	return new pbPsrser();
}

void pbPsrser::CreateProtoByFile(const char * path, const char * proto_file){
	release();
	sourceTree = new DiskSourceTree;
	/** @brief ���ҵ�ǰĿ¼ */
	sourceTree->MapPath("", path);
	importer = new Importer(sourceTree, NULL);
	/** @brief ��������foo.proto��file */
	importer->Import(proto_file);
	pool = importer->pool();
}
void pbPsrser::CreateProtoByName(const char *proto_name){
	release();
	file_proto = new FileDescriptorProto;
	/** @brief ����proto�� */
	file_proto->set_name(proto_name);
}

Message *pbPsrser::CreateMeg(const char * msg_name){
	const Message *message = NULL;
	Message *msg = NULL;
	if (factory == NULL)
	{
		factory = new DynamicMessageFactory(pool);
	}
	
	if (importer)
	{
		descriptor = pool->FindMessageTypeByName(msg_name);
	}
	else if (file_proto)
	{
		descriptor = file_descriptor->FindMessageTypeByName(msg_name);
	}
	message = factory->GetPrototype(descriptor);
	msg = message->New();

	return msg;
}

void pbPsrser::SetMegValue(Message *msg,const char *key,int value){
	const Reflection *reflection = msg->GetReflection();
	const FieldDescriptor *field = NULL;

	field = descriptor->FindFieldByName(key);
	reflection->SetInt32(msg, field, value);
}
void pbPsrser::SetMegValue(Message *msg,const char *key,string value){
	const Reflection *reflection = msg->GetReflection();
	const FieldDescriptor *field = NULL;

	field = descriptor->FindFieldByName(key);
	reflection->SetString(msg, field, value.c_str());
}
void pbPsrser::SetMegValue(Message *msg,const char *key,float value){
	const Reflection *reflection = msg->GetReflection();
	const FieldDescriptor *field = NULL;

	field = descriptor->FindFieldByName(key);
	reflection->SetFloat(msg, field, value);
}

void pbPsrser::AddTypeMeg(const char * msg_name, vector<pb_msg_item *> &keys){
	/** @brief ������Ϣ�� */
	DescriptorProto *message_proto = file_proto->add_message_type();
	message_proto->set_name(msg_name);

	FieldDescriptorProto *field_proto = NULL;

	int size = keys.size();
	for (int i=0;i<size;i++)
	{
		/** @brief ���������Ϣ�ĳ�Ա�����֣��洢���ͣ��������������� */
		pb_msg_item *item = keys[i];
		field_proto = message_proto->add_field();
		field_proto->set_name(item->key_name);  /**< ����  */
		field_proto->set_type(item->key_type);  /**< �ַ�  */
		field_proto->set_number(i+1);           /**< ��һ��  */
		field_proto->set_label(item->key_label);/**< ����  */
	}
}

string pbPsrser::GetMegString(Message *msg){
	return msg->DebugString();
}

void pbPsrser::FinishAdd()
{
	DescriptorPool *po  = new DescriptorPool;
	file_descriptor = po->BuildFile(*file_proto);
	pool = po;
}
