#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include<spdlog/spdlog.h>

#include<spdlog/sinks/rotating_file_sink.h>
#include "httpserver.h"

#include "detectthread.h"

#define VERSION "V22.1"
class MyRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory{
public:
    MyRequestHandlerFactory(){};
    ~MyRequestHandlerFactory(){};
public:
    virtual Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
   {
        return new RootHandler;
   };
};

void glogInit()
{
    
    string logDir = "/home/sne/data_7t/Detect_M30T/camera/gglog/";
    
    google::InitGoogleLogging("");
    string info_log = logDir + "snefly_info_";
    google::SetLogDestination(google::INFO, info_log.c_str());
    string warning_log = logDir + "snefly_warning_";
    google::SetLogDestination(google::WARNING, warning_log.c_str());
    string error_log = logDir + "snefly_error_";
    google::SetLogDestination(google::ERROR, error_log.c_str());
    string fatal_log = logDir + "snefly_fatal_";
    google::SetLogDestination(google::FATAL, fatal_log.c_str());
    FLAGS_logbufsecs = 0;                                               //缓冲日志输出，默认为30秒，此处改为立即输出
    FLAGS_max_log_size = 100;                                           //最大日志大小为 100MB
    FLAGS_stop_logging_if_full_disk = true;                             //当磁盘被写满时，停止日志输出
    
}

int main()
{   
    cout << "camra_detect_target Software version is " << VERSION << endl;
    glogInit();

    std::shared_ptr<spdlog::logger> _SneSolarDetectLogFile;
    _SneSolarDetectLogFile = spdlog::rotating_logger_mt("detect_logger","SNE_SolarDetect_Log.txt",1024 * 1024 * 5, 3);
    _SneSolarDetectLogFile->flush_on(spdlog::level::trace);
   
    _SneSolarDetectLogFile->info("task start detect ");

    LOG(INFO) << "camra_detect_target Software version is " << VERSION << endl;
    Poco::UInt16 port = 1050;                      ////设置端口   M30 端口1050  对应web那边是8050    
    _SneSolarDetectLogFile->info("set port 8050 "); 
    
    Poco::Net::HTTPServerParams * pParams = new Poco::Net::HTTPServerParams;
    pParams->setMaxQueued(10);   //设置最大队列和最大线程
    pParams->setMaxThreads(10);

    Poco::Net::ServerSocket svs(port); //设置HTTP服务的端口

    Poco::Net::HTTPServer srv(new MyRequestHandlerFactory(), svs, pParams);

    //start the HTTPServer
    srv.start();

    LOG(INFO) << "http server start" << endl;

    Thread _thread; //定义线程

    DetectThread thread; //图像处理线程

    _thread.start(thread);  //启动图像处理线程

    while(1)
    {
        sleep(200);
        // sleep(20);
    }


    thread.Stop();
    
    _thread.join();
        
    cout << "http server getchar" << endl;

    srv.stop();

    return 1;
}

