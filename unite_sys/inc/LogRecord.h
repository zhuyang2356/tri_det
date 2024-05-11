/*
 * LogRecord.h
 *
 *  Created on: 2023年2月20日
 *      Author: passion
 */

#ifndef LOGRECORD_H_
#define LOGRECORD_H_

#include "ace/Task.h"
#include "ace/Date_Time.h"
#include "ace/streams.h"
#include "ace/Log_Msg.h"
#include "ace/Log_Msg_Callback.h"
#include "ace/Log_Record.h"
#include "ace/OS.h"
#include "ace/Singleton.h"
#include <string>
using namespace std;

class LogRecord : public ACE_Log_Msg_Callback,public ACE_Task<ACE_MT_SYNCH> {
public:
	LogRecord() {
		//初始化相关变量
		output_ = NULL;
		ACE_OS::memset(lastHour_,0,16);
		ACE_OS::memset(curHour_,0,16);
		//获取环境变量 IDP_HOME
		// project_home_ = ACE_OS::getenv("DISTRI_HOME_1");
		// project_home_ = "/home/sne/data_7t/tri_det/cp10";
		project_home_ = ACE_OS::getenv("TRI_DETECT_HOME");
	}

	~LogRecord() {
		//退出工作线程
		Stop();
	}

	//开始工作
	void Start(string procName) {
		ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)进程2%s已经运行.\n"),procName.c_str()));
		//保存进程名
		procName_ = procName;
		//设置日志为回调
		ACE_LOG_MSG->set_flags(ACE_Log_Msg::MSG_CALLBACK);
		ACE_LOG_MSG->msg_callback(this);
		//激活工作线程
		this->activate();

	}

	//日志回调函数
	void log(ACE_Log_Record &log_record) {
		//提取日志信息
		ACE_Message_Block * msg = 0;
		ACE_NEW(msg,ACE_Message_Block(log_record.msg_data_len()));
		msg->copy(log_record.msg_data(),log_record.msg_data_len());
		//将日志放入队列
		if(msg != 0) {
			this->putq(msg);
		}

		return;
	}

	//保存日志
	void WriteLog(ACE_Message_Block *msg) {
		try{
			//获取当前时间字符串
			ACE_Date_Time curTime;
			ACE_OS::sprintf(curHour_,"%04d%02d%02d%02d",
					(int)curTime.year(),
					(int)curTime.month(),
					(int)curTime.day(),
					(int)curTime.hour());

			//判断小时是否发生变化
			int ret = ACE_OS::strcmp(curHour_,lastHour_);
			if (ret > 0) {
				//当前时间大于上次时间，释放过时的流
				if (output_ != NULL) {
					delete output_;
					output_ = NULL;
				}

				//保存当前时间字符串
				ACE_OS::strcpy(lastHour_,curHour_);
			}

			//判断输出流是否为0
			if (output_ == NULL) {
				//判断LOG文件夹是否存在
				string dirname = project_home_ + "/log";
				ACE_DIR* pDir = ACE_OS::opendir(dirname.c_str());
				if(pDir == 0){
					//不存在，创建文件夹
					if(0 != ACE_OS::mkdir(dirname.c_str())){
						return;
					}
				}else{
					//已存在，关闭文件夹
					ACE_OS::closedir(pDir);
				}

				//判断对应进程文件夹是否存在
				dirname += "/" + procName_;
				pDir = ACE_OS::opendir(dirname.c_str());
				if(pDir == 0){
					//不存在，创建文件夹
					if(0 != ACE_OS::mkdir(dirname.c_str())){
						return;
					}
				}else{
					//已存在，关闭文件夹
					ACE_OS::closedir(pDir);
				}

				//构造当前日志文件夹、文件
				char curFolder[64];
				char curFile[64];
				ACE_OS::memset(curFolder,0,64);
				ACE_OS::memset(curFile,0,64);
				ACE_OS::sprintf(curFolder,"/%04d%02d%02d",
						(int)curTime.year(),
						(int)curTime.month(),
						(int)curTime.day());
				ACE_OS::sprintf(curFile,"/hour%02d",
						(int)curTime.hour());

				//判断当前日志文件夹是否存在
				dirname += string(curFolder);
				pDir = ACE_OS::opendir(dirname.c_str());
				if(pDir == 0) {
					//不存在，创建文件夹
					if(0 != ACE_OS::mkdir(dirname.c_str())){
						return;
					}
				}else{
					//已存在，关闭文件夹
					ACE_OS::closedir(pDir);
				}

				//构造文件输出流
				dirname += string(curFile);
				output_ = new std::ofstream(dirname.c_str(),std::ofstream::app|std::ofstream::binary|std::iostream::out);
			}

			//写入日志
			*output_ << msg->rd_ptr();
			output_->flush();
		} catch(...) {
			//释放当前流，重新打开
			if (output_ != NULL) {
				delete output_;
				output_ = NULL;
			}
			ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)**********write log error<%s>**********\n"),procName_.c_str()));
		}

		return;
	}

	//工作线程
	int svc() {
		//ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)**********log svc start<%s>**********\n"),procName_.c_str()));
		ACE_Message_Block * msg = 0;

		while(1) {
			//从队列中获取消息
			this->getq(msg);

			//判断消息类型
			if(msg->msg_type() == ACE_Message_Block::MB_STOP){
				//退出消息，直接退出线程
				msg->release();
				msg_queue()->close();
				break;
			} else if (msg->msg_type() == ACE_Message_Block::MB_DATA) {
				//日志消息，写入文件
				WriteLog(msg);
				msg->release();
			}
		}

		//ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)**********log svc stop<%s>**********\n"),procName_.c_str()));
		return 0;
	}

	//停止工作
	void Stop() {
		//构造退出消息
		ACE_Message_Block * stop_msg = 0;
		ACE_NEW(stop_msg,ACE_Message_Block(0,ACE_Message_Block::MB_STOP));
		//压入队列
		this->putq(stop_msg);
		//等待线程退出
		this->wait();
	}

private:
	string procName_;		//进程名
	string project_home_;	//环境变量
	char lastHour_[16];			//上次时间
	char curHour_[16];			//当前时间
	ACE_OSTREAM_TYPE *output_;	//输出流
};

//定义单件模式
typedef ACE_Singleton<LogRecord,ACE_Recursive_Thread_Mutex> SGT_LogRecord;

#endif /* LOGRECORD_H_ */
