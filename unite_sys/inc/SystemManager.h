/*
 * SystemGuarder.h
 *
 *  Created on: 2019年11月23日
 *      Author: passion
 */

#ifndef INC_SYSTEMMANAGER_H_
#define INC_SYSTEMMANAGER_H_

#include "ace/Task.h"
#include "ace/Process.h"
#include "LogRecord.h"
#include "ShareStruct.h"
#include "json/json.hpp"
#include "ManagerBase.h"
#include "RabbitMQClient.h"
#include "DriverSolarM300.h"
#include "SneMinIOManager.h"

#include "ev.h"

using json = nlohmann::json;


class SystemManager: public ACE_Task_Base, public ManagerBase {
public:
	SystemManager();
	virtual ~SystemManager();
public:
	//初始化
	int Init(string procName );
	//打开线程
	int Open();
	//系统守护线程
	int svc();
	//关闭线程
	int Close() ;
	//重置诊断任务
	int Retask(int tasktype, int taskNo);
	//回调诊断结果数据
	int PutDetectMessage( int tasktype,  int taskNo, ACE_Message_Block *mbk);
	// int PutDetectMessage(ACE_Message_Block *mbk);
   //调用MinIO下载
//   int downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey,std::string rawPreFixPath);
    int downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey);

  //调用MinIOn上传
  bool uploadfile(std::string BucketName, std::string objectKey, std::string pathkey);
private:
	//读取系统配置
	bool GetSystemConfig();
	//根据路径创建文件夹
	void Mkdir(string path, string bucketName);
	//加载算法诊断库
	bool LoadAlgorithmLibs();
	//卸载算法诊断库
	bool UnloadAlgorithmLibs();
	//启动RabbitMQ消息队列
	bool StartRabbitMQ();

public:
	string procName_;			//进程名
	bool closeFlag_;			//关闭标识
	int logSaveTime_;			//日志保存时间
	float usageLimit_;			//硬盘使用率上限
	//标识符，每次任务的唯一标识，防止未消费信息从队列里重新进入算法
	// string compl_taskId,taskId;
	string redis_addr_;
	string redis_port_;
	string redis_pwd_;
//	CMqttManager 	MqttManger_;	//mqtt消息处理类
	CMinIOManager 	MinioManger_;	//minio分布式文件处理类

private:
	//MQTT连接参数
//	MQTTCLIENTSTRUCT m_mqttConfig_;
//	string mqtt_usr_;
//	string mqtt_pwd_;
//	string mqtt_address_;
	//minIO连接参数
	string minIO_usr_;
	string minIO_pwd_;
	string minIO_address_;
	string minIO_path_;
	//算法诊断库配置
	map<int, algorithm_config> m_algorithm_config_;
	//算法库指针
	vector<ACE_DLL *> v_algorithm_driver_dll_;

	//RabbitMQ连接信息
	string rabbitmq_address_;
	string rabbitmq_queuename_sub_;
	string rabbitmq_queuename_pub_;
	string rabbitmq_queuename_cb_test_;
	//
	AMQP::TcpChannel* pTcpChannel_;
	AMQP::TcpChannel* pTcpChannel_cb_;

};

#endif /* INC_SYSTEMMANAGER_H_ */
