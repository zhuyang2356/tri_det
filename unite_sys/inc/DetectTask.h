/*
 * DetectTask.h
 *
 *  Created on: Mar 23, 2023
 *      Author: sne
 */

#ifndef SRC_DETECTTASK_H_
#define SRC_DETECTTASK_H_

#include "LogRecord.h"

class DetectTask : public ACE_Task_Base{
public:
	DetectTask();
	virtual ~DetectTask();
};

#endif /* SRC_DETECTTASK_H_ */
