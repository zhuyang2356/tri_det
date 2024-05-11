/*
 * Main.cpp
 *
 *  Created on: 2019年11月23日
 *      Author: passion
 */

#include "../inc/SignalHandler.h"
#include "../inc/CheckSysStatus.h"
#include "../inc/SystemManager.h"

//检查进程运行个数
int CheckProcCount(string progName) {

	//初始化运行个数为0
	int nCount = 0;
	char tmpbuf[128] = {0};
	char strcmd[128] = {0};

	//利用wc -l 精确查找XXX进程启动个数
	//再利用ps aux全路径，grep -v 反向过滤，屏蔽查找命令本身
	sprintf(strcmd,"ps |grep %s|grep -v '/%s/'|grep -v 'grep'|wc -l",progName.c_str(),progName.c_str());
	//执行命令并返回执行结果I/O流
	FILE *ptr = popen(strcmd,"r");
	if (ptr != NULL) {
		//读取执行结果
		if (fgets(tmpbuf,128,ptr) != NULL) {
			//获取运行个数
			nCount = atoi(tmpbuf);
		}

		//关闭流
		pclose(ptr);
	}
	return nCount;
}

int ACE_TMAIN(int argc,ACE_TCHAR * argv[]) {
	//获取当前进程名
	string procName = argv[0];
	procName = procName.substr(procName.find_last_of("/")+1);
	ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)进程%s已经运行.\n"),procName.c_str()));
	SGT_LogRecord::instance()->Start(procName);		//创建日志接口

	//检查当前进程运行个数
	if (CheckProcCount(procName) > 1) {
		ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)进程%s已经运行.\n"),procName.c_str()));
		return 0;
	}

	ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)进程%s启动.\n"),procName.c_str()));

	//系统守护对象
	SystemManager sysManager;

	//系统状态检查对象
	CheckSysStatus system_check;

	//初始化，在这里加载动态库 libDriverSolarM300.so
	if (!sysManager.Init(procName) || !system_check.Init(&sysManager)) {
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%D)初始化失败，进程%s退出.\n"),procName.c_str()),false);
	}
	//开始工作
	sysManager.Open();
	//信号处理对象
	bool bExit = false;
	SignalHandler sigHandler(&bExit);
	ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)进程%s已经运行.\n"),"s123"));
//	在这里启动组件以后，就开始并行运行诊断算法，收到publisher给的信息就开始跑
	//注册关闭信号
	ACE_Sig_Handler sig_handler;
	sig_handler.register_handler(SIGTERM,&sigHandler);	//注册终止信号
//
	//检查时间
	time_t checkTime = ACE_OS::time(0);
	while (!bExit) {

		//获取当前时间
		time_t currTime = ACE_OS::time(0);

		//判断是否到检查周期
		if (currTime -checkTime >= 10 &&  currTime > checkTime) {
			ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)丛这里开始注册算法6666666666666.\n")));
			//检查系统空间
			system_check.check_sys_space();
			//检查CPU和内存
			system_check.check_cpu_mem();

			//保存检查时间
			checkTime = currTime;
		}
		ACE_OS::sleep(600);
	}

	//停止工作
	sysManager.Close();
	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)进程%s退出.\n"),procName.c_str()));
 	return 0;
}
