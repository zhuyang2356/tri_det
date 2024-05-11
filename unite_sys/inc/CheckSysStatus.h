/*
 * CheckSysStatus.h
 *
 *  Created on: 2019年11月23日
 *      Author: passion
 */

#ifndef INC_CHECKSYSSTATUS_H_
#define INC_CHECKSYSSTATUS_H_

#include "sys/vfs.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ShareStruct.h"
//printf("运行到这里");
class SystemManager;

class CheckSysStatus {
public:
	CheckSysStatus();
	virtual ~CheckSysStatus();

public:
	//初始化
	bool Init(SystemManager *p);
	//检查系统空间
	void check_sys_space();
	//清理系统日志
	void clean_sys_log(int logtime);
	//获取硬盘使用率
	float get_disk_usage(string path);
	//检查CPU和内存库状态
	void check_cpu_mem();
	//获取CPU状态
	void get_cpu_status(CPU_STATUS &cpust);
	//获取内存状态
	void get_mem_status(MEM_STATUS &mem);
	//计算CPU和内存使用率
	void cal_cpu_mem(CPU_STATUS &last,CPU_STATUS &now,MEM_STATUS &mem);

private:
	CPU_STATUS cpu_stat_last;	//上次CPU状态
	CPU_STATUS cpu_stat_now;	//本次CPU状态
	MEM_STATUS mem_stat;		//内存状态

	string logDir_;				//系统日志路径
	string scanShell_;			//扫描脚本路径
	SystemManager *sysManager_;	//守护对象指针
};

#endif /* INC_CHECKSYSSTATUS_H_ */
