#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H


//*****************Poco库*************************//
#include "Poco/Thread.h"
#include "Poco/Runnable.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPCredentials.h>
#include <Poco/StreamCopier.h>
#include <Poco/NullStream.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Net/HTMLForm.h>
//*****************Poco库*************************//


#include <iostream>
#include <unistd.h>
#include <json/json.h>
#include <vector>

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <dirent.h>
#include <glog/logging.h>

#include<modelinferce/segmentation/u2net/u2net.h> 

#include <sstream>
#include <exif/exif.h>


#include "string"
#include<modelinferce/detection/yolox/yoloxDetect.h>

// 
#include "ctime"
#include "thread"
#include<spdlog/spdlog.h>

#include<spdlog/sinks/rotating_file_sink.h>

#include<datatype.h>
// #include<infra_detect.h>  //cousin 20230510
// #include<vis_detect.h> //cousin 20230510
// #include<resultPackage.h>
#include<algorithm.h>

using namespace std;
using namespace cv;
using Poco::Thread;
using namespace google;

extern bool g_flag;
extern string g_sRecev;
extern vector<string> gvec_sRecev;  //声明  接受后端
extern string json_seted_para;//发送配置参数
extern bool g_stop;
extern bool g_setParam;
//extern bool g_sendParam;//发送配置参数



struct PlotInfo{
    string longitude;
    string latitude;
};


class DetectThread:public Poco::Runnable
{
    public:
        DetectThread();
        ~DetectThread();
        virtual void run();    //虚拟运行  多态有关  虚函数
        void Start();          //可以去掉 
        void Stop();           //可以去掉
        
        //**************************  接受数据 根据接受的JSON 判断诊断类型 ***************************//
        void HandRecvData();    //接收后端数据
        void ImagesDetect(bool status,Json::Value root);    //确认任务诊断类型  是单文件诊断还是文件夹诊断
        void GetSquareInfo(Json::Value root);               //航线信息  //添加故障组件归属方阵所需要的功能
        void DirDector();                                                                  //文件夹检测
        void SingleFileDetect(string t_filename,string z_filename,string& result);          
        //******************************************************************************************//


        // ********************** 初步处理 逻辑程序**************************************************//
        bool ReadDirFile(string strPath,vector<string> &fileList);//读取文件夹文件 并将文件名传到filelist
        
        // int getGPSInfor(string filepath,double& x,double& y,      //    根据像图像文件 获取GPS的一系列信息 
        //                 double& height,double& focal,double& xspeed,
        //                 double& yspeed, double& absheight,double& RtkStdLon,
        //                 double& GimbalPitchDegree,double& GimbalYawDegree );
        
        //  算法诊断  
        // void GetFaultPointGpsInfo(double pointx,double pointy,    //根据故障和不同分辨率计算故障点组件的GPS
        //                         double & jd,double &wd , 
        //                         int img_width,float focus,
        //                         double centerE,double centerN,
        //                         float chipsize_w  );
        
        // JSON融合
  
        void DetectCallBack(string jsonArray,string& result);      //回调给后端数据库，是每组图像的故障信息

        // *******************************************************************************************//


        //**************************   功能函数  俗称轮子*********************************************//
        void msleep(uint32_t value);
        void SplitString(const string& s, vector<string>& v, const string& c);
        void StringtoGps(const string squarename,vector<string> str,map <string,vector<PlotInfo>> & square);
        double stringToNum(const string& str);
        vector<string> splitStr(string str, char delimiter);  // 解析大疆二进制图像参数
        
        // ******************************************************************************************//


        
       
    private:
        bool m_Stop;           
        string m_sRecv;              
        int m_iLineId;                      //图片对应在数据库中的主键
        string m_sPicturePath;            //图片的存储路径
        string m_sLineName;               //航线名
        string m_sDetectionType;          //类型   是单个文件检测   还是文件夹检测
        int m_bStopFlag;                 //停止和开始标记
        int m_modelType,m_modelTypeMx;    //m_modelType 高低功率  m_modelTypeMx 电站类型
        int m_iPictureId;  


        string m_sZoomFileName;
        string m_sThrmFileName;
        double m_dTx,m_dTy,m_dTheight;             //红外图片的GPS
        double m_dZx,m_dZy,m_dZheight;             //可见光图片的GPS
        double m_Tfocus,m_Zfocus;
        double m_xspeed,m_yspeed;
        bool m_bStop;
        int m_SquareArrayFlag;

        double m_dBlc;

        Json::Value m_root;

        bool m_bIsexit;
        
        double m_redFlyHeigt;
        double m_redFlightYawDegree;   //偏向角
        double m_redFlightPitchDegree;  //姿态角

        double m_visibleFlyHeigt;
        double m_visFlightYawDegree;
        double m_visFlightPitchDegree;

        //云台参数
        double m_TGimbalPitchDegree;
        double m_TGimbalYawDegree;
        double m_ZGimbalPitchDegree;
        double m_ZGimbalYawDegree;



        map <string,vector<PlotInfo>> squareinfor;   
      
   
		string _output_path;

        public:
        // 算法库　　和　JSON　融合库　　　

        cv::Mat drawvisimg;
        // DetectInfraLogic detectInfraLogic;
        // DetectVisLogic detectVislogic;

        // std::shared_ptr<spdlog::logger> _SneSolarDetectLogFile;

        SneDetectAI SneSolarAI;
        inputdata solarinputdata;

        
};



#endif
