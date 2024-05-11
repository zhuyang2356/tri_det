/*
 * ShareStruct.h
 *
 *  Created on: 2019年11月23日
 *      Author: passion
 */

#ifndef INC_SHARESTRUCT_H_
#define INC_SHARESTRUCT_H_

#include <string>
#include <map>
using namespace std;
#include "DriverBase.h"

//CPU状态结构体
struct CPU_STATUS {
	char name[20];				//CPU名称
	unsigned int user;			//用户态的CPU时间
	unsigned int nice;			//低优先级程序所占用CPU时间
	unsigned int system;		//系统态的CPU时间
	unsigned int idle;			//CPU空闲的时间
	unsigned int iowait;		//等待IO响应的时间
	unsigned int irq;			//处理硬件中断的时间
	unsigned int softirq;		//处理软件中断的时间
};

//内存状态结构体
struct MEM_STATUS{
	unsigned int total;		//总内存
	unsigned int free;		//空闲内存
};

//MQTT主题配置接口
// typedef struct{
// 	MQTTAsync 		subClient;
// 	MQTTAsync 		pubClient;
// 	string    		subTopic;
// 	string    		pubTopic;
// }MQTTCLIENTSTRUCT;


//MQTT主题类型
typedef enum{
	TOPIC_SUB_TYPE = 0,
	TOPIC_PUB_TYPE
} TOPIC_type;


//诊断算法类型
typedef enum{
	DETECT_SOLAR_M300_TYPE = 0,
	DETECT_SOLAR_M30_TYPE ,
	DETECT_SOLAR_M3_TYPE,
	DETECT_WIND_TYPE,
	DETECT_LINE_TYPE,
} detect_type;

//诊断算法配置
typedef struct{
	int type;
	string name;
	int count;
}algorithm_config;

//诊断算法处理类
typedef struct{
	DriverBase * pDriverBase;
	int status;
}algorithm_driver;
#endif /* INC_SHARESTRUCT_H_ */
