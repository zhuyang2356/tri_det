/*
 * ManagerBase.h
 *
 *  Created on: 2023-2-20
 *      Author: passion
 *  Description:	算法管理功能基类
 */

#ifndef MANAGERBASE_H_
#define MANAGERBASE_H_

#include <string>
using namespace std;

//默认数据缓存大小
#define DEFAULT_BLOCK_SIZE 1024
 
class ACE_Message_Block;


class ManagerBase {
public:
	ManagerBase(){};
	virtual ~ManagerBase(){};

public:
	//初始化
	virtual int Init(string procName) = 0;
	//打开
	virtual int Open() = 0;
	//关闭
	virtual int Close() = 0;
	//重置诊断任务
	virtual int Retask(int tasktype,  int taskNo) = 0;
	//回调诊断结果数据
	virtual int PutDetectMessage( int tasktype,  int taskNo, ACE_Message_Block *mbk) = 0;
  //调用MinIO下载
  	// virtual int downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey,std::string rawPreFixPath) = 0;
	virtual int downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey) = 0;

  //调用MinIOn上传
  	virtual bool uploadfile(std::string BucketName, std::string objectKey, std::string pathkey) = 0;
};

#endif /* CHANNELBASE_H_ */
