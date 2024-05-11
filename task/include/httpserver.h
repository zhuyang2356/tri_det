#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/DNS.h"
#include "Poco/Net/ServerSocket.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>



#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <json/json.h>

#include <opencv2/opencv.hpp>
#include <dirent.h>
#include <vector>

// #include "detectthread.h"
#include<detectthread.h>


using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using namespace cv;
using namespace std;


class RootHandler:public HTTPRequestHandler
{
    public:
        RootHandler();
        ~RootHandler();
        void handleRequest(HTTPServerRequest& request,HTTPServerResponse& response);
        bool HandRecvData();
        int StartDetect();
        bool SingleFileDetect(string t_filename,string z_filename,string& result);
        void Compress(string z_filename);
        void DetectCallBack(string jsonArray,string& result);
        void PacketJson(string srcjson,string& outjson);
        void GetFaultGPS(double xsx,double xsy,double& jd,double& wd);
        int ReadDirFile(string strPath,vector<string> &fileList);
        int AdjustFileList(string strPath,vector<string> &fileList);
        void DirDector();
        vector<string> splitStr(string str, char delimiter);
        double stringToNum(const string& str);
        int getGPS(string filepath,double& x,double& y);

        ///virtual void run();

        //void Start();
        //void Stop();

        //Poco::Thread _thread;

        DetectThread *thread;
        Thread _thread;


    private:
        string m_sRecv;
        string m_sPicturePath;            //图片的存储路径
        string m_sLineName;               //航线名
        int m_iLineId;            //图片对应在数据库中的主键
        string m_sDetectionType;          //类型   是单个文件检测   还是文件夹检测
        int m_bStopFlag;                 //停止和开始标记

        int m_modelType,m_modelTypeMx;     //m_modelType 高低功率  m_modelTypeMx

        int m_iPictureId;  

        string m_sZoomFileName;
        string m_sThrmFileName;

        double m_dTx,m_dTy;             //红外图片的GPS
        double m_dZx,m_dZy;             //可见光图片的GPS   
        bool m_bStop;

        double m_dBlc;

};

#endif