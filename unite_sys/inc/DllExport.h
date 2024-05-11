/*
 * DllExport.h
 *
 *  Created on: Feb 24, 2023
 *      Author: sne
 */

#ifndef DLLEXPORT_H_
#define DLLEXPORT_H_

#include "DriverSolarM300.h"

#ifdef DRIVER_SOLAR_M300_DLL
#define DRIVER_SOLAR_M300_DLL ACE_Proper_Export_Flag
#else
#define DRIVER_SOLAR_M300_DLL ACE_Proper_Import_Flag
#endif

extern "C" DRIVER_SOLAR_M300_DLL DriverBase *MakeDriver();

#endif /* DLLEXPORT_H_ */