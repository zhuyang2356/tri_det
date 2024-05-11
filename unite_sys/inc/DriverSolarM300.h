/*
 * DriverSolarM300.h
 *
 *  Created on: Feb 24, 2023
 *      Author: sne
 */

#ifndef DRIVERSOLARM300_H_
#define DRIVERSOLARM300_H_

#include <iostream>
#include <unistd.h>
#include <json/json.h>
#include <vector>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <dirent.h>
#include <glog/logging.h>
#include "ShareStruct.h"



#include "DriverBase.h"
#include "DetectTask.h"

#include "json/json.hpp"
#include "../../solar/include/datatype.h"
#include "../../solar/include/algorithm.h"
using json = nlohmann::json;

using namespace std;
using namespace cv;
using namespace google;

extern bool g_flag;
extern string g_sRecev;
extern vector<string> gvec_sRecev;
extern string json_seted_para;//发送配置参数
extern bool g_stop;
extern bool g_setParam;
//extern bool g_sendParam;//发送配置参数
// extern SolarDefectDetector detector;

struct PlotInfo{
    string longitude;
    string latitude;
};

class DriverSolarM300:public DriverBase{
public:
	DriverSolarM300();
	virtual ~DriverSolarM300();
public:
	//初始化,从managerbase那里继承来的init虚函数化还是不虚函数化，没有影响。
	// int Init(string procName, ManagerBase *pManager);
    virtual int Init(string procName, ManagerBase *pManager);

	//打开
	// int Open();
    virtual int Open();

	//关闭
	// int Close();
    virtual int Close();

	//传递诊断任务数据
	// int PutTaskMessage(int tasktype, int taskNo, ACE_Message_Block* recvMsg);
    virtual int PutTaskMessage(int tasktype, int taskNo, ACE_Message_Block* recvMsg);

	// int fn_pos(string T, string P, int n);
	void run(vector<string> gvec_sRecev);    //虚拟运行  多态有关  虚函数

	void msleep(uint32_t value);
    bool HandRecvData();
    void ImagesDetect(bool status,Json::Value root);
    void DirDector();
    int loadPureAlgor();
    bool unloadPureAlgor();
    // 返回结果
    std::vector<string> m_result;
    int m_result_size,picNum,bktNamePos;
    cv::Mat drawvisimg;
    std::string strBack,bktName,DirInfo,preFixPath,pathKey,faultRayPic,faultVisPic;
    std::string UpObjPath,UpLocalPath,smallUpObjDir,upObjLocalDir,negUpObjDir,fileNameHeader;
    std::set<string> flawFileNames;
    json mResJson,faultJson;
    bool isSuccess;
    void DetectCallBack(string jsonArray);
    void DetectCallBackxml(string jsonArray,string& result);
    int getType();
    void GetSquareInfo(Json::Value root);
    void DetectCallBackError(int lineId);
    int readDirFile(string strPath, map<string, string> &tFilesList,  map<string, string> &zFilesList);
    int ReadDirFile(string strPath,vector<string> &fileList);
        string bigInfraRedPic,bigVisLgtPic;
        Mat vis_img,_T_Solar_img;
        string faultimg_pos,rectifyUpObjDir,downloadDir,attStar;
        string _output_path,rawPreFixPath;
        // virtual string pure_algori(string m_sThrmFileName,string m_sZoomFileName,string m_sPicturePath,string faultimg_pos);
        map <string,vector<PlotInfo>> squareinfor;
        void SplitString(const string &s, vector<string> &v, const string &c);
        void StringtoGps(const string squarename, vector<string> str, map<string, vector<PlotInfo>> &square);   
        //********************************************************************************//
        bool Remove(std::string file_name);
        int rm_dir_v1(std::string dir_full_path);
        // ******************************************************************************************//

        SneDetectAI SneSolarAI;
        inputdata solarinputdata;

private:
        bool m_Stop;           
        string m_sRecv,processedInfo;              
        string m_sPicturePath;            //图片的存储路径
        string m_sLineName;               //航线名
        double m_iLineId;            //图片对应在数据库中的主键
        string m_sDetectionType;          //类型   是单个文件检测   还是文件夹检测
        int m_bStopFlag;                 //停止和开始标记
        int m_modelType,m_modelTypeMx;    //m_modelType 高低功率  m_modelTypeMx
        int m_iPictureId;  
        double m_lineId;
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
		
public:
	ManagerBase *pManager_; //所连接管理指针

	// DetectTask *pDetectTask_;//诊断算法处理程序
	// DetectThread pDetectTask_;
};
#endif /* DRIVERSOLARM300_H_ */
