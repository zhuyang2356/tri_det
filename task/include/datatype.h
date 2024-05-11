/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-24 16:41:07
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-25 16:28:01
 * @FilePath: /SolarDetect_wzj/lib/include/datatype.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * 
 * 
 */
#pragma once

#include<iostream>
#include<opencv2/opencv.hpp>
#include<vector>
#include<string>
#include<math.h>

using namespace std;
using namespace cv;


struct inputdata
{
    string m_timagename;
    string m_zimagename;
    string dirname;
    
};

struct DJIdata
{
    char Cameratype[8];   //摄像机型号
    double ImgJD;
    double ImgWD;
    double Flyheight;
    double Focallength;
    double GimbalPitch;
    double GimbalYaw;
};