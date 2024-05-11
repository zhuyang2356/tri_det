/*
 * DriverBase.h
 *
 *  Created on: 2023-2-20
 *      Author: passion
 *  Description:	算法诊断驱动功能基类
 */
#ifndef DRIVERBASE_H
#define DRIVERBASE_H

#include <string>
#include <vector>

using namespace std;


class ACE_Message_Block;
class ManagerBase;

class DriverBase {
public:
	DriverBase(){};
	virtual ~DriverBase(){};

public:
	//初始化
	virtual int Init(string procName, ManagerBase *pManagerBase) = 0;
	// virtual int Init() = 0;

	//打开
	virtual int Open() = 0;
	//关闭
	virtual int Close() = 0;
	//传递诊断任务数据
	virtual int PutTaskMessage(int tasktype, int taskNo, ACE_Message_Block* recvMsg) = 0;
	// virtual string pure_algori(vector<string> tFileList,int i,string m_sPicturePath,string faultimg_pos);
};
#endif
