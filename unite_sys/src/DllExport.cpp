	/*
	* DllExport.cpp
	*
	*  Created on: Feb 24, 2023
	*      Author: sne
	*/
#include "../inc/DllExport.h"

DriverBase *MakeDriver() {
	//创建网络客户端通道类	
	DriverSolarM300 *p = new DriverSolarM300();
	//转换为基类指针并返回
	return dynamic_cast<DriverBase *>(p);
}
