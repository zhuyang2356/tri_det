/*
 * SystemGuarder.cpp
 *
 *  Created on: 2020年04月14日
 *      Author: passion
 */

#include "../inc/SystemManager.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include <hiredis/hiredis.h>

// #include "../inc/DriverSolarM300.h"
using namespace rapidxml;
/******************************************************/
//定义动态库导出函数的指针
typedef DriverBase *(*MakeDriver)();
//算法诊断库类
map<int, vector<algorithm_driver> > m_algorithm_driver_;
vector<string> redis_params(3);
/******************************************************/
SystemManager::SystemManager() {
	// TODO Auto-generated constructor stub
	closeFlag_= false;
	logSaveTime_ = 30;
	usageLimit_ = 90;
	pTcpChannel_ = NULL;

}

SystemManager::~SystemManager() {
	// TODO Auto-generated destructor stub
}

int SystemManager::Init(string procName) {
	procName_ = procName;

	//获取系统配置
	if (!GetSystemConfig()) {
		return false;
	}
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)读取配置文件完成.\n")));

//	//初始化MQTT
//	if(!MqttManger_.Init(mqtt_address_,mqtt_usr_,mqtt_pwd_,m_mqttConfig_.subTopic,m_mqttConfig_.pubTopic))
//	{
//		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)初始化MQTT失败.\n")));
//		return false;
//	}

	//初始化MINIO
	if(!MinioManger_.init(minIO_address_, minIO_usr_, minIO_pwd_)){
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)初始化minIO失败.\n")));
		return false;
	}

	//加载算法诊断库
	if(!LoadAlgorithmLibs()){
		return false;
	}

//	//初始化回调函数
//	if(!StartRabbitMQ())
//	{
//		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)初始化回调函数失败.\n")));
//		return false;
//	}
	return 1;
}

int SystemManager::Open() {
	//MqttManger_.Open();
	this->activate();
	return 1;
}

int SystemManager::Close() {
	//MqttManger_.Close();
	closeFlag_ = true;
	this->wait();
	//卸载算法诊断库
	UnloadAlgorithmLibs();
	return 1;
}

 //调用MinIO下载
// int SystemManager::downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey,std::string rawPreFixPath)
// {
//   return MinioManger_.downloadfile(BucketName, preFixPath, pathkey,rawPreFixPath);
// }
int SystemManager::downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey)
{
  return MinioManger_.downloadfile(BucketName, preFixPath, pathkey);
}
//调用MinIOn上传
bool SystemManager::uploadfile(std::string BucketName, std::string objectKey, std::string pathkey)
{
  return MinioManger_.uploadfile(BucketName, objectKey, pathkey);
}
  
  
int SystemManager::svc() {
	SGT_LogRecord::instance()->Start(procName_);		//创建日志接口
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)诊断任务管理线程开启.\n")));

	while(!closeFlag_)
	{
		//休眠100毫秒
		ACE_OS::sleep(ACE_Time_Value(0,100000));

		// access to the event loop
		auto *loop = EV_DEFAULT;

		// handler for libev
		RabbitMQClientHandler handler(loop);

		// make a connection
		AMQP::Address address(rabbitmq_address_);

		AMQP::TcpConnection connection(&handler, address);

		//*********************创建一个测试回调队列***************************
		if (!rabbitmq_queuename_cb_test_.empty()){
		const std::string queuename_callback = rabbitmq_queuename_cb_test_;
		// we need a channel too
		AMQP::TcpChannel channel_cb(&connection);
		pTcpChannel_cb_ = &channel_cb;
		channel_cb.consume(queuename_callback).onSuccess(
		[](const std::string &tag) {
			// the consumer is ready
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)started consuming with tag : %s\n"), tag.c_str()));
		}).onCancelled(
		[](const std::string &tag) {
			// the consumer was cancelled by the server
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)consumer :%s was cancelled.\n"),tag.c_str()));
		}).onReceived(
		[&channel_cb, queuename_callback](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
		}); 
		}else{ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D).第二个监听回调序列没初始化起来.\n")));}

		//*********************创建回调队列***************************
		const std::string queuename_pub = rabbitmq_queuename_pub_;
		// we need a channel too
		AMQP::TcpChannel channel_pub(&connection);

		pTcpChannel_ = &channel_pub;
		//----------------------------------------------------------------------------------------- 
		//*********************************************************
		// we need a channel too
		AMQP::TcpChannel channel_sub(&connection);
		//消费端函数，监听东西
		const std::string queuename = rabbitmq_queuename_sub_;
		// start a consumer
		channel_sub.consume(queuename).onSuccess(
		[](const std::string &tag) {
			// the consumer is ready
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)started consuming with tag : %s\n"), tag.c_str()));
		}).onCancelled(
		[](const std::string &tag) {
			// the consumer was cancelled by the server
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)consumer :%s was cancelled.\n"),tag.c_str()));
		}).onReceived(
		[&channel_sub, queuename](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
			string strRecvJSON(message.body(), message.bodySize());
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)message :%s .\n"),strRecvJSON.c_str()));
			//todo 添加,redis 参数
			//这个consume函数里的东西，都只能用自己作用域里的东西
			try{
				//解析json消息数据
				json j = json::parse(strRecvJSON);
				ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)信息修改！\n")));

				//从redis里取taskID
				//todo 改成可配置的,redis 参数
				string taskId,cmpl_taskId;

				// redisContext *c1=redisConnect("192.16.13.80",6379);
				redisContext *c1=redisConnect(redis_params[0].c_str(),atoi(redis_params[1].c_str()));

				if(c1->err){
						redisFree(c1);
						cout<<"connect to redis fail"<<endl;
						ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)redis没有连接上.\n")),false);
				}
				redisReply *redis_auth=(redisReply*)redisCommand(c1,"AUTH %s",redis_params[2].c_str());
				// redisReply *redis_auth=(redisReply*)redisCommand(c1,"AUTH %s",redis_pwd_);
				if (redis_auth->type == REDIS_REPLY_ERROR) {
					// ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)Redis初始认证失败！！\n")));
					ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)Redis初始认证失败！！.\n")),false);
				}
				else
					{
						ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)Redis初始认证成功！！\n")));
					}
				redisReply *redis_reply=(redisReply*)redisCommand(c1,"get %s",queuename.c_str());
				if(redis_reply->str){
					cout<<"key里有值"<<endl;
					cmpl_taskId=redis_reply->str;
					}
				else{
					cmpl_taskId="";
					}
				redisFree(c1);
				freeReplyObject(redis_auth);
				freeReplyObject(redis_reply);
				//获取诊断素材
				if (j.find("BucketName") == j.end()) {
					ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)接收JSON数据中没有找到【BucketName】, 不与处理！\n")));
				}else{
					//判断诊断类型
					if(j.find("TaskType") == j.end()){
						ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)接收JSON数据中没有找到【TaskType】, 不与处理！\n")));
					}else{
						ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)222OK！\n")));
						int taskType = j["TaskType"].get<int>();
						int isDetect = 0;
						taskId=j["taskId"].get<string>();
						//已经完成任务ID不存在，或者已完成任务ID和现有ID不一样，
						if (cmpl_taskId.empty() || taskId!=cmpl_taskId){
						// string taskId=j["TaskId"].get<string>();
						ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)进来信息的ID【%d】,此时线程队列状态【%d】, 完成！\n"),deliveryTag,isDetect));
						//判断目前算法队列可以空闲
						map<int, vector<algorithm_driver> >::iterator iter = m_algorithm_driver_.find(taskType);
						for (unsigned int i = 0; i < iter->second.size(); ++i)
						{
							if(iter->second[i].pDriverBase && !iter->second[i].status)
							{
								//加入相应诊断任务队列
								ACE_Message_Block * taskMsg = new ACE_Message_Block(message.bodySize());;
								taskMsg->copy(message.body(), message.bodySize());
								ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)在sysmanager里面开始进行算法流程.\n")));
//								重置算法诊断动态库提示是在DriverSolarM300的方法里完成的。
								iter->second[i].status = 1;
								isDetect=iter->second[i].pDriverBase->PutTaskMessage(taskType, i, taskMsg);
								ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)诊断任务23456788, 完成！\n")));
								// isDetect = 1;
								break;
							}
						}
						if(isDetect){
							ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)诊断任务【%d】, 完成！\n"),taskType));
							// ack the message
							channel_sub.ack(deliveryTag);
							// redisContext *c2=redisConnect("192.16.13.80",6379);
							redisContext *c2=redisConnect(redis_params[0].c_str(),atoi(redis_params[1].c_str()));
								if(c2->err){
									redisFree(c2);
									cout<<"connect to redis fail"<<endl;
									ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)redis没有连接上.\n")),false);
								}
							redisReply *redis_auth_2=(redisReply*)redisCommand(c2,"AUTH %s",redis_params[2].c_str());
							// redisReply *redis_auth_2=(redisReply*)redisCommand(c2,"AUTH %s",redis_pwd_);

								if (redis_auth_2->type == REDIS_REPLY_ERROR) {
									// ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)Redis初始认证失败！！\n")));
									ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)Redis第二次认证失败！！.\n")),false);
								}
								else
									{
										ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)Redis第二次认证成功！！\n")));
									}
							redisReply *redis_set_reply=(redisReply*)redisCommand(c2,"SET %s %b",queuename.c_str(),taskId.data(),taskId.length());
							redisFree(c2);
							freeReplyObject(redis_auth_2);
							freeReplyObject(redis_set_reply);
							// cmpl_taskId=taskId;
						}else{
							ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)诊断类型【%d】线程已满, 本此诊断任务此服务器放弃诊断！\n"),taskType));
							// 如果本次任务出错，那么也要把这次消费给消费掉
							channel_sub.ack(deliveryTag);
							// //返回给queueName一个结果
							// json error_back={{"taskId",taskId},{"taskStatus","failed"}};
							// string strBack=error_back.dump();
							// ACE_Message_Block * taskMsg = new ACE_Message_Block(strBack.size());;
							// taskMsg->copy(strBack.c_str(), strBack.size());
							// // PutDetectMessage(1,1,taskMsg);
							// pTcpChannel_->publish("", rabbitmq_queuename_pub_, taskMsg->rd_ptr(),taskMsg->size());
							// taskMsg->release();
						}
					}
					else if(!cmpl_taskId.empty() && taskId==cmpl_taskId){
						ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)诊断任务【%d】, 完成,重复消费，消息退回！\n"),deliveryTag));
						channel_sub.ack(deliveryTag);
					}
				}
			}
		}catch(...){
				ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)LOLmessage解析抛出异常！\n")));
				// ack the message
				channel_sub.ack(deliveryTag);
			}
		});

		// run the loop
		ev_run(loop, 0);
	}

	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)LOL诊断任务管理线程退出.\n")));
	return 0;
}

bool SystemManager::GetSystemConfig() {
	//获取环境变量DAS_HOME
	// const char *project_home = ACE_OS::getenv("DISTRI_HOME_2");
	// const char *project_home = "/home/sne/data_7t/tri_det/cp10";
	const char *project_home = ACE_OS::getenv("TRI_DETECT_HOME");

	if(!project_home){
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)环境变量IDP_HOME获取失败.\n")),false);
	}

	//构造XML配置文件路径
	string configFile;
	configFile.append(project_home);
	configFile.append(ACE_TEXT("/config/SystemConfig.xml"));

	//打开XML文件
	file<> fdoc(configFile.c_str());
	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	/****************************
	 获取其他配置项
	 ****************************/
	//获取ElseConfig节点
	xml_node<char> *node = doc.first_node("ElseConfig");
	if (!node) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取其他系统配置失败，配置文件缺少ElseConfig节点.\n")),false);
	}

	//获取LogSaveTime子节点
	xml_node<char> *subnode = node->first_node("LogSaveTime");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取其他系统配置失败，配置文件缺少LogSaveTime子节点.\n")),false);
	} else {
		logSaveTime_ = ACE_OS::atoi(subnode->first_attribute("val")->value());
	}

	//获取DiskUsageLimit子节点
	subnode = node->first_node("DiskUsageLimit");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取其他系统配置失败，配置文件缺少DiskUsageLimit子节点.\n")),false);
	} else {
		usageLimit_ = ACE_OS::atof(subnode->first_attribute("val")->value());
	}

/****************************
	 获取redis配置信息
	 ****************************/

	node = doc.first_node("REDIS");
	if (!node) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取REDIS配置信息失败，配置文件缺少MINIO节点.\n")),false);
		return false;
	}

	//获取ADDRESS子节点
	subnode = node->first_node("ADDRESS");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少USR子节点.\n")),false);
		return false;
	} else {
		redis_addr_ = subnode->first_attribute("val")->value();
		redis_params[0]=redis_addr_;
	}

	//获取PORT子节点
	subnode = node->first_node("PORT");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少PWD子节点.\n")),false);
	} else {
		redis_port_ = subnode->first_attribute("val")->value();
		redis_params[1]=redis_port_;
	}

	//获PWD子节点
	subnode = node->first_node("PWD");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少ADDRESS子节点.\n")),false);
	} else {
		redis_pwd_ = subnode->first_attribute("val")->value();
		redis_params[2]=redis_pwd_;
	}

	/****************************
	 获取minIO配置信息
	 ****************************/

	node = doc.first_node("MINIO");
	if (!node) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少MINIO节点.\n")),false);
		return false;
	}

	//获取USR子节点
	subnode = node->first_node("USR");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少USR子节点.\n")),false);
		return false;
	} else {
		minIO_usr_ = subnode->first_attribute("val")->value();
	}

	//获取PWD子节点
	subnode = node->first_node("PWD");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少PWD子节点.\n")),false);
	} else {
		minIO_pwd_ = subnode->first_attribute("val")->value();
	}

	//获ADDRESS子节点
	subnode = node->first_node("ADDRESS");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少ADDRESS子节点.\n")),false);
	} else {
		minIO_address_ = subnode->first_attribute("val")->value();
	}

	//获PATH子节点
	subnode = node->first_node("PATH");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取minIO配置信息失败，配置文件缺少PATH子节点.\n")),false);
	} else {
		minIO_path_ = subnode->first_attribute("val")->value();
	}

	/****************************
	 获取算法库配置信息
	 ****************************/

	node = doc.first_node("DriverAlgorithm");
	if (!node) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取DriverAlgorithm配置信息失败，配置文件缺少DriverAlgorithm节点.\n")),false);
		return false;
	}

	//获取row节点
	xml_node<char> *node1 = node->first_node();
	for ( ; node1 != 0; node1 = node1->next_sibling()) {
		algorithm_config tmp_config;
		//获取每个字段的描述名和描述值
		xml_attribute<char> *col = node1->first_attribute();
		for ( ; col != 0; col = col->next_attribute()) {
			if(!strcmp(col->name(),"name")){
				tmp_config.name.append(col->value());
			}else if(!strcmp(col->name(),"type")){
				tmp_config.type = atoi(col->value());
			}else if(!strcmp(col->name(),"count")){
				tmp_config.count = atoi(col->value());
			}
		}
		m_algorithm_config_[tmp_config.type] = tmp_config;
	}

	/****************************
	 获取RabbitMQ连接信息
	 ****************************/

	node = doc.first_node("RabbitMQ");
	if (!node) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取RabbitMQ配置信息失败，配置文件缺少RabbitMQ节点.\n")),false);
		return false;
	}

	//获ADDRESS子节点
	subnode = node->first_node("ADDRESS");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取mRabbitMQ配置信息失败，配置文件缺少ADDRESS子节点.\n")),false);
	} else {
		rabbitmq_address_ = subnode->first_attribute("val")->value();
	}

	//获PATH子节点
	subnode = node->first_node("QUEUENAME_SUB");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取RabbitMQ配置信息失败，配置文件缺少QUEUENAME_SUB子节点.\n")),false);
	} else {
		rabbitmq_queuename_sub_ = subnode->first_attribute("val")->value();
	}

	//获PATH子节点
	subnode = node->first_node("QUEUENAME_PUB");
	if (!subnode) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取RabbitMQ配置信息失败，配置文件缺少QUEUENAME_PUB子节点.\n")),false);
	} else {
		rabbitmq_queuename_pub_ = subnode->first_attribute("val")->value();
	}
	//-------------------------------------获测试的时候为了看返回结果，自己用一个queuePATH子节点--------------------------------
	subnode = node->first_node("QUEUENAME_CALLBACK_TEST");
	if (!subnode) {
		// ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取RabbitMQ配置信息失败，配置文件缺少QUEUENAME_CALL_BACK子节点.\n")),false);
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)QUENAME_CALLBACK节点没有配置在文件里\n")));
	} else {
		rabbitmq_queuename_cb_test_ = subnode->first_attribute("val")->value();
	}
	// ----------------------------------------------------------------------------------------------------------------------
//	/****************************
//	 获取MQTT连接信息
//	 ****************************/
//
//	//获取MQTT节点
//	node = doc.first_node("MQTT");
//	if (!node) {
//		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取MQTT连接信息失败，配置文件缺少MQTT节点.\n")),false);
//	}
//
//	//获取USR子节点
//	subnode = node->first_node("USR");
//	if (!subnode) {
//		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取MQTT连接信息失败，配置文件缺少USR子节点.\n")),false);
//	} else {
//		mqtt_usr_ = subnode->first_attribute("val")->value();
//	}
//
//	//获取PWD子节点
//	subnode = node->first_node("PWD");
//	if (!subnode) {
//		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取MQTT连接信息失败，配置文件缺少PWD子节点.\n")),false);
//	} else {
//		mqtt_pwd_ = subnode->first_attribute("val")->value();
//	}
//
//	//获ADDRESS子节点
//	subnode = node->first_node("ADDRESS");
//	if (!subnode) {
//		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取MQTT连接信息失败，配置文件缺少ADDRESS子节点.\n")),false);
//	} else {
//		mqtt_address_ = subnode->first_attribute("val")->value();
//	}
//	/****************************
//	 获取MQTT任务主题
//	 ****************************/
//	node = doc.first_node("MQTTTOPIC");
//	if (!node) {
//		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)获取MQTTTOPIC任务主题信息失败，配置文件缺少MQTTTOPIC节点.\n")),false);
//	}
//
//	//获取row节点
//	xml_node<char> *node1 = node->first_node();
//	for ( ; node1 != 0; node1 = node1->next_sibling()) {
//		string temTopic;
//		int type = -1;
//		//获取每个字段的描述名和描述值
//		xml_attribute<char> *col = node1->first_attribute();
//		for ( ; col != 0; col = col->next_attribute()) {
//			if(!strcmp(col->name(),"topic")){
//				temTopic.append(col->value());
//			}else if(!strcmp(col->name(),"type")){
//				type = atoi(col->value());
//			}
//		}
//
//		if(type == TOPIC_SUB_TYPE){
//			m_mqttConfig_.subTopic = temTopic;
//		}else if(type == TOPIC_PUB_TYPE){
//			m_mqttConfig_.pubTopic = temTopic;
//		}
//	}
	return true;
}

void SystemManager::Mkdir(string path, string bucketName){
	//判断文件夹是否存在
	string dirname =  path + bucketName + "/source";
	ACE_DIR* pDir = opendir(dirname.c_str());
	if(pDir == 0){
		//不存在，创建文件夹
		ACE_OS::mkdir(dirname.c_str());
	}else{
		//已存在，关闭文件夹
		closedir(pDir);
	}
	dirname =  path + bucketName + "/result";
	pDir = opendir(dirname.c_str());
	if(pDir == 0){
		//不存在，创建文件夹
		ACE_OS::mkdir(dirname.c_str());
	}else{
		//已存在，关闭文件夹
		closedir(pDir);
	}

	dirname =  path + bucketName + "/infrared_mask";
	pDir = opendir(dirname.c_str());
	if(pDir == 0){
		//不存在，创建文件夹
		ACE_OS::mkdir(dirname.c_str());
	}else{
		//已存在，关闭文件夹
		closedir(pDir);
	}

	dirname =  path + bucketName + "/visible_mask";
	pDir = opendir(dirname.c_str());
	if(pDir == 0){
		//不存在，创建文件夹
		ACE_OS::mkdir(dirname.c_str());
	}else{
		//已存在，关闭文件夹
		closedir(pDir);
	}
}

bool SystemManager::LoadAlgorithmLibs() {
	//获取环境变量DAS_HOME
	// const char *project_home = ACE_OS::getenv("DISTRI_HOME_2");
	// const char *project_home = "/home/sne/data_7t/tri_det/cp10";
	const char *project_home = ACE_OS::getenv("TRI_DETECT_HOME");

	if(!project_home){
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)环境变量IDP_HOME获取失败.\n")),false);
	}
	string libPath;
	libPath.append(project_home);
	libPath.append("/lib/");
	string libFilePath;


	map<int, algorithm_config>::iterator iter = m_algorithm_config_.begin();
	for(; iter != m_algorithm_config_.end(); ++iter)
	{
		vector<algorithm_driver> v_driver;
		 for(int i=0; i < iter->second.count; i++)
		 {
			 libFilePath = libPath + iter->second.name;
			 ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)%d-%s\n"),i,libFilePath.c_str()));
			 ACE_DLL *pDriverDll_ = new ACE_DLL();
			ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)----------.\n")));
			// libFilePath="/home/sne/data_7t/tri_det/cp7/lib/libDriverSolarM30.so";
			 if (0 == pDriverDll_->open(libFilePath.c_str())) {
				ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)%s-%s\n"),"--------",libFilePath.c_str()));
				void *void_ptr = pDriverDll_->symbol(ACE_TEXT("MakeDriver"));
				if (0 == void_ptr) {
					ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)动态库%s获取MakeDriver函数指针失败！\n"),iter->second.name.c_str()),false);
				}
				ptrdiff_t tmp = reinterpret_cast<ptrdiff_t>(void_ptr);
				MakeDriver pMakeDriver= reinterpret_cast<MakeDriver>(tmp);
				algorithm_driver tmp_driver;
				tmp_driver.pDriverBase = pMakeDriver();
				tmp_driver.pDriverBase->Init(procName_, this);
				tmp_driver.status = 0;
				v_driver.insert(v_driver.begin(), tmp_driver);
				ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)在【%s】\n 路径下的动态库【%s】-【%d】加载成功.\n"),libFilePath.c_str(),iter->second.name.c_str(),i));
			} else {
				if(pDriverDll_->error_){
					ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)动态库%s加载失败(%s)！\n"),iter->second.name.c_str(),pDriverDll_->error()),false);
				}
			}
		 }
		 m_algorithm_driver_[iter->second.type] = v_driver;
	}
	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)算法诊断动态库全部加载成功.\n")));
	return true;
}

bool SystemManager::UnloadAlgorithmLibs() {
	//释放驱动
	map<int, vector<algorithm_driver> >::iterator iter = m_algorithm_driver_.begin();
	for(; iter != m_algorithm_driver_.end(); ++iter)
	{
		for (unsigned int i = 0; i < iter->second.size(); ++i)
		{
			if(iter->second[i].pDriverBase)
			{
				iter->second[i].pDriverBase->Close();
				delete iter->second[i].pDriverBase;
			}
		}
	}
	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)释放算法诊断动态库完成.\n")));
	return true;
}
int SystemManager::Retask(int tasktype, int taskNo){
	map<int, vector<algorithm_driver> >::iterator iter  = m_algorithm_driver_.find(tasktype);
	if(iter != m_algorithm_driver_.end()){
		iter->second.at(taskNo).status = 0;
		ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)重置算法诊断动态库【%d-%d】完成.\n"),tasktype,taskNo));
	}
	return true;
}

//回调诊断结果数据
int SystemManager::PutDetectMessage( int tasktype,  int taskNo, ACE_Message_Block *mbk){
	pTcpChannel_->publish("", rabbitmq_queuename_pub_, mbk->rd_ptr(),mbk->size());
	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)\n 回调，发送给:%s \n"),rabbitmq_queuename_pub_.c_str()));
	// if (!pTcpChannel_cb_){
	// pTcpChannel_cb_->publish("", rabbitmq_queuename_cb_test_, mbk->rd_ptr(),mbk->size());
	// ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)\n 回调，发送给:%s \n 和：%s"),rabbitmq_queuename_pub_.c_str(),rabbitmq_queuename_cb_test_.c_str()));
	// }else{ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)\n 回调，发送给:%s \n"),rabbitmq_queuename_pub_.c_str()));}
	mbk->release();
	// sleep(10);
	return true;
}
// int SystemManager::PutDetectMessage(ACE_Message_Block *mbk){
// 	pTcpChannel_->publish("", rabbitmq_queuename_pub_, mbk->rd_ptr(),mbk->size());
// 	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)\n 回调，发送给:%s \n"),rabbitmq_queuename_pub_.c_str()));

// 	mbk->release();
// 	sleep(10);
// 	return true;
// }
bool SystemManager::StartRabbitMQ(){
	// access to the event loop
	auto *loop = EV_DEFAULT;

	const std::string queuename = rabbitmq_queuename_pub_;

	// handler for libev
	RabbitMQClientHandler handler(loop);

	// make a connection
	AMQP::Address address("amqp://sne2023:sne2023@198.120.0.143/");

	AMQP::TcpConnection connection(&handler, address);

	// we need a channel too
	AMQP::TcpChannel channel(&connection);

	// create a temporary queue
	channel.declareQueue(queuename).onSuccess(
			[&connection, &channel, loop](const std::string &queuename,
					uint32_t messagecount, uint32_t consumercount) {

				// report the name of the temporary queue
				std::cout << "declared queue " << queuename << std::endl;
				ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)declared queue 【%s】完成.\n"),queuename.c_str()));
	});
	pTcpChannel_ = &channel;


	return true;
}


