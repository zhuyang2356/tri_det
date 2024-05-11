/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-23 12:46:34
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-27 15:22:45
 * @FilePath: /SolarDetect_wzj/lib/include/algorithm.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <iostream>
#include <vector>

#include <string>
#include <opencv2/opencv.hpp>

#include <thread>

#include<spdlog/spdlog.h>
#include<spdlog/sinks/rotating_file_sink.h>

#include<datatype.h>
#include<infra_detect.h>    //红外算法库
#include<vis_detect.h>      //可见算法库
#include<resultPackage.h>   //json融合库
#include <solar_utils.h>


class SneDetectAI
{
private:
    /* data */
    bool sne_debug=true;
    // inputdata solarinputdata;
    string m_dirpath;
    string m_z_imgpath;
    string m_t_imgpath;
    string m_z_imgname;
    string m_t_imgname;
    string outputpath ;

    bool superres =false;
    // int real_shijiao_x,real_shijiao_y,real_shijiao_w,real_shijiao_h;
    float k1,k2,p1,p2;
   
    DJIdata djitimgdata,djizimgdata;

    //********红外相机参数*********//
    //注释人：cousin
    //使  用：
    //************************//
    double m_dTx,m_dTy,m_dTheight,m_Tfocus;
    double m_TGimbalPitchDegree;
    double m_TGimbalYawDegree;
    double m_tBlc;            
    
    //********可见相机参数*********//
    //注释人：cousin
    //使  用：
    //************************//
    double m_dZx,m_dZy,m_dZheight,m_Zfocus; 
    double m_ZGimbalPitchDegree;
    double m_ZGimbalYawDegree;
    double m_zBlc;  


    Mat inputT,Timg,Zimg;

 


public:
    SneDetectAI();
    ~SneDetectAI();
    
    resPack resPackProc;
    DetectInfraLogic detectInfraLogic;
    DetectVisLogic detectVislogic;

    vector<infraFaultJson> infrapackages;
    vector<visbleFaultJson> visblepackages;

    void run(inputdata solarinputdata);
    void initalgorithm();
    // void getimginfo();
    void infradetectprocess();
    // void visbledetectprocss(int threadID);
    void visbledetectprocss();


    void InfraVisbelFusion();
    void GetFaultPointGpsInfo(  double pointx,double pointy, double & jd,double &wd , int img_width,
                                float focus,double centerE,double centerN,float chipsize_w  );   //根据故障和不同分辨率计算故障点组件的GPS
    
    void SolarJsonFusionFGX(string &RESjson,string faultimg_pos);
    void SolarJsonFusionIVCV(string &RESjson);
    void SolarIvCvReProcess(bool DEBUG_DRAW);
    void gpsInfoAssemble(vector<infraFaultJson> &infrapackages,vector<visbleFaultJson> &visblepackages);

    // void SolarJsonCobine();


};


