/*
 * CheckSysStatus.cpp
 *
 *  Created on: 2019年11月23日
 *      Author: passion
 */

#include "../inc/CheckSysStatus.h"
#include "../inc/SystemManager.h"

CheckSysStatus::CheckSysStatus() {
	// TODO Auto-generated constructor stub
	memset(&cpu_stat_last,0,sizeof(CPU_STATUS));
	memset(&cpu_stat_now,0,sizeof(CPU_STATUS));
	memset(&mem_stat,0,sizeof(MEM_STATUS));

	sysManager_ = 0;
}

CheckSysStatus::~CheckSysStatus() {
	// TODO Auto-generated destructor stub
}

bool CheckSysStatus::Init(SystemManager *p) {
	sysManager_ = p;

	//获取环境变量DAS_HOME
	const char *project_home = ACE_OS::getenv("TRI_DETECT_HOME");

	if(!project_home){
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)环境变量IDP_HOME获取失败.\n")),false);
	}

	//构造系统日志路径
	logDir_.clear();
	logDir_.append(project_home);
	logDir_.append(ACE_TEXT("/log"));

	//构造扫描日志脚本路径
	scanShell_.clear();
	scanShell_.append(project_home);
	scanShell_.append(ACE_TEXT("/config/scan_dir.sh"));	//扫描日志脚本路径

	return true;
}

void CheckSysStatus::check_sys_space() {
	//清理日志，保留设置的天数
	clean_sys_log(sysManager_->logSaveTime_);

	//获取硬盘使用率
	if (get_disk_usage("/home") > sysManager_->usageLimit_) {
		//若home目录下使用空间超过阀值，则删除今天以前的所有日志
		clean_sys_log(1);
	}

	return;
}

void CheckSysStatus::clean_sys_log(int logtime) {
	//构造执行命令
	char cleanCmd[256] = {0};
	ACE_OS::sprintf(cleanCmd,"%s %s %d",scanShell_.c_str(),logDir_.c_str(),logtime);

	//执行命令
	ACE_OS::system(cleanCmd);
	return;
}


float CheckSysStatus::get_disk_usage(string path) {
	float ret = 0;
	struct statfs diskInfo;
	statfs(path.c_str(), &diskInfo);

	unsigned long long blocksize     = diskInfo.f_bsize;    //每个block里包含的字节数
	unsigned long long totalsize     = diskInfo.f_blocks * blocksize;   //总的字节数，f_blocks为block的数目
	unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //可用空间大小

	unsigned long long usedDisk = totalsize - availableDisk;	//已用

	float percent =  ((float)usedDisk / (float)totalsize) * 100;

	if (percent>=0 &&percent<=100){
		ret = percent;
	}

	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)目录{%s}的磁盘空间工况=>\n"
			"(%D)总共=%qMB=%qGB\n"
			"(%D)可用=%qMB=%qGB\n"
			"(%D)已用=%qMB=%qGB\n"
			"(%D)已用比例=%.3f%%\n"),
			path.c_str(),
			totalsize>>20,totalsize>>30,
			availableDisk>>20,availableDisk>>30,
			usedDisk>>20,usedDisk>>30,
			percent));
	return ret;
}

void CheckSysStatus::check_cpu_mem() {
	//获取CPU状态
	get_cpu_status(cpu_stat_now);

	//获取内存状态
	get_mem_status(mem_stat);

	//计算CPU和内存使用率
	cal_cpu_mem(cpu_stat_last,cpu_stat_now,mem_stat);

	return;
}

void CheckSysStatus::get_cpu_status(CPU_STATUS &cpust) {
	char buff[256];

	//打开相关文件
	FILE *fd = fopen ("/proc/stat", "r");
	if (fd == NULL) {
		ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)/proc/stat文件打开失败，CPU状态获取失败.\n")));
		return;
	}

	//获取第一行，总CPU使用状况
	fgets (buff, sizeof(buff), fd);
	sscanf (buff, "%s %u %u %u %u %u %u %u"
			,cpust.name,&cpust.user,&cpust.nice,&cpust.system,&cpust.idle,&cpust.iowait,&cpust.irq,&cpust.softirq);

	fclose(fd);
	return;
}

void CheckSysStatus::get_mem_status(MEM_STATUS &mem) {
	char buff[256];
	char name[32];

	//打开相关文件
	FILE *fd = fopen ("/proc/meminfo", "r");
	if (fd == NULL) {
		ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)/proc/meminfo文件打开失败，内存状态获取失败.\n")));
		return;
	}

	//获取第一行，总内存
	fgets (buff, sizeof(buff), fd);
	sscanf (buff, "%s %u %s", name, &mem.total, name);

	//获取第二行，空闲内存
	fgets (buff, sizeof(buff), fd);
	sscanf (buff, "%s %u %s", name, &mem.free, name);

	//关闭文件fd
	fclose(fd);
	return;
}

void CheckSysStatus::cal_cpu_mem (CPU_STATUS &last,CPU_STATUS &now,MEM_STATUS &mem) {
	///////////////////////////计算CPU使用率/////////////////////////////
	float cpu_use = 0;

	//上次总CPU使用时间
	unsigned int last_total = last.user + last.nice + last.system + last.idle + last.iowait + last.irq + last.softirq;
	//本次总CPU使用时间
	unsigned int now_total = now.user + now.nice + now.system + now.idle + now.iowait + now.irq + now.softirq;

	//计算总消耗时间
	int sum = now_total - last_total;
	//计算总空闲时间
	int idle = now.idle - last.idle;

	//计算使用率
	if(sum >= 0) {
		cpu_use = (1 - (float)idle / sum) * 100;
	}

	//将本次状态保存至上次
	last = now;

	///////////////////////////计算内存使用率/////////////////////////////
	//计算使用率
	float mem_use = 0;
	if (mem.total >= 0) {
		mem_use = (1 - (float)mem.free / mem.total) * 100;
	}

	///////////////////////////输出信息/////////////////////////////
	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)系统CPU和内存使用工况=>\n"
			"(%D)总CPU时间=%d|空闲CPU时间=%d|CPU使用率=%.2f%%\n"
			"(%D)总内存=%dkB|空闲内存=%dkB|内存使用率=%.2f%%\n")
			,sum,idle,cpu_use,mem.total,mem.free,mem_use));
	return;
}
