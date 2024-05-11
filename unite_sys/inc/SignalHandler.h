/*
 * SignalHandler.h
 *
 *  Created on: 2023年2月20日
 *      Author: passion
 */

#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_

#include "ace/Event_Handler.h"
#include "ace/Signal.h"
#include "ace/Sig_Handler.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

class SignalHandler : public ACE_Event_Handler {
public:
	SignalHandler(bool *exit,bool *gps=NULL) {
		pExit_ = exit;
		pGps_ = gps;
	}

	virtual ~SignalHandler(){
	}

public:
	int handle_signal(int signum,siginfo_t *info,ucontext_t *context) {
		//分析信号类型
		switch (signum) {
		case SIGTERM:		//关闭信号
			{
				if (pExit_) {
					*pExit_ = true;
					ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)接收到关闭信号，即将退出！\n")));
				}
			}
			break; 
		default:
			break;
		}

		return 1;
	}

public:
	bool *pExit_;		//退出信号标识
	bool *pGps_;		//GPS信号标识
};

#endif /* SIGNALHANDLER_H_ */
