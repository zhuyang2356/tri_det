#include <detectthread.h>
#include"spdlog/spdlog.h"
#include"spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#define SNE_PI 3.14159265359
#define SNE_Z_w_pix 4000
#define SNE_Z_h_pix 3000
#define SNE_T_w_pix 1280
#define SNE_T_h_pix 1024
#define SNE_Z_w_size 6.4
#define SNE_Z_h_size 4.8
#define SNE_T_w_size 7.68
#define SNE_T_h_size 6.144
#define SAVE_SOLASR  true
#define DEBUG_DRAW true




bool g_flag = false;        //这是用来？
//string g_sRecev;
vector<string> gvec_sRecev;     //定义
string json_seted_para; //    
bool g_setParam = false;
// bool g_sendParam = false; 
bool g_stop;
bool debug = false;
bool superres =false;


DetectThread::DetectThread()
{   
    _output_path = "../results/pos/";
    m_Stop = 0;
    cout<<"图像诊断初始化"<<endl;
    cout<<"保存故障路径"<< " "<< _output_path<<endl;
    
   
}

DetectThread::~DetectThread()
{
    cout << "诊断完成" << endl;
}

void DetectThread::Start()
{
}

void DetectThread::Stop()
{
    m_Stop = 1;
}

void DetectThread::run()
{


    while (!m_Stop)
    {
        //if (g_flag == true)
        {
            // LOG(INFO) << "g_sRecev: " << g_sRecev << endl;
            auto iter = gvec_sRecev.begin();
            if(iter != gvec_sRecev.end())
            {

                m_sRecv = *iter;
                gvec_sRecev.erase(iter);
                //m_sRecv = g_sRecev;
                if (g_setParam) //需要接收配置参数    这个是web
                {
                    Json::Reader reader;
                    Json::Value mainroot1;
                    reader.parse(m_sRecv, mainroot1);
                    /*
                    detector.receive_segmentation_threshold(mainroot1["segmentation_threshold"].asDouble()); //红外分割阈值
                    detector.receive_min_area(mainroot1["min_area"].asInt());                                //红外组件最小面积
                    detector.receive_max_area(mainroot1["max_area"].asInt());                                //红外组件最大面积
                    detector.receive_min_whratio(mainroot1["min_whratio"].asDouble());                       //红外组件宽高比
                    detector.receive_max_whratio(mainroot1["max_whratio"].asDouble());                       //红外组件宽高比
                    detector.receive_extra_add(mainroot1["extra_add"].asDouble());                           //红外组件外扩阈值
                    detector.receive_dcd_min_Gmem(mainroot1["dcd_min_Gmem"].asDouble());                     //掉串相关参数
                    detector.receive_dcd_min_Bmem(mainroot1["dcd_min_Bmem"].asDouble());                     //掉串相关参数
                    detector.receive_dcd_min_Std(mainroot1["dcd_min_Std"].asDouble());                       //掉串相关参数
                    detector.receive_hot_spots_threshold(mainroot1["hot_spots_threshold"].asDouble());       //掉串相关参数

                    */
                    g_setParam = false;
                }
                else
                    HandRecvData(); //目前使用这个
                    cout<<"*************一批任务结束 HandRecvData 结束 *************"<<endl;
                g_flag = false;
            }
        }
        msleep(500);
    }

}

void DetectThread::HandRecvData()
{
    Json::Reader reader; // 解析json用Json::Reader
    Json::Value root;    // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array
    Json::Value mainroot;

    if (reader.parse(m_sRecv, mainroot))
    {   
        cout<<"输出当前存在所有任务的JSON: "<<mainroot<<endl;

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // *************mainroot 只有 dirlist 和 stopFlag   分别表示任务多少  和是否暂停******************* //
        // *************dirlist 包含当前json 任务******************************************************** //
        // ************* json 任务包含以下内容  任务开始时间 任务结束时间 任务是文件夹还是单个图片 航线id***** //
        // ************* 航线区域列表 文件夹尾缀 文件夹路径 任务iD 任务名称 模型类型 电站类型 报告id  ******** //
        // ************* 航线区域 是否停止 任务飞行距离 任务number 图像数量组 图像上传时间点 **************** //
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        m_bStopFlag = mainroot["stopFlag"].asInt(); //停止标志位
        LOG(INFO) << "m_bStopFlag: " << m_bStopFlag << endl;

        for (int i = 0; i < mainroot["DirList"].size(); i++)
        {
            root = mainroot["DirList"][i]; //多少个需要在线诊断
            cout<<"输出当前任务的JSON: "<<root<<endl;
            if (m_bStopFlag == false)
            {
                ImagesDetect(m_bStopFlag, root); //该函数调用了ImageDetect 中的文件夹检测
                cout<<"*************一组文件夹图  ImagesDetect  结束 ***************"<<endl;
            }
            else
            {
                LOG(INFO) << "Detect is end..." << endl;
            }

            if (g_stop == true)
            {
                LOG(INFO) << "-=====================000000000000000000" << endl;
                while (!m_bIsexit)
                    msleep(100);
                LOG(INFO) << "images Detect is pause..." << endl;
                break;
            }
        }
    }
    else
    {
        LOG(INFO) << "m_sRecv is error..." << endl;
        // return false;
    }

    m_bIsexit = false;
    // return true;
}

void DetectThread::ImagesDetect(bool status, Json::Value root) //在run中被调用
{
    if (status == false) //开始检测 状态为0表示开始
    {
        m_sPicturePath = root["folderUrl"].asString(); //图片的存储路径
        LOG(INFO) << "m_sPicturePath: " << m_sPicturePath << endl;

        m_sLineName = root["lineName"].asString(); //航线名
        LOG(INFO) << "m_sLineName: " << m_sLineName << endl;

        m_iLineId = root["lineId"].asInt(); //航线ID
        LOG(INFO) << "m_iLineId: " << m_iLineId << endl;

        m_sDetectionType = root["diagnoseType"].asString(); //类型   是单个文件检测   还是文件夹检测
        LOG(INFO) << "m_sDetectionType: " << m_sDetectionType << endl;

        m_modelType = root["modelType"].asInt(); // 高低功率      

        m_modelTypeMx = root["modelTypeMx"].asInt(); //电站类型

        m_SquareArrayFlag = root["squareArrayFlag"].asInt();

        if (m_SquareArrayFlag == 1)
        {
            GetSquareInfo(root);
        }


        if (m_sDetectionType == "d")                        //关于这块的结果   因为这边检测的时图片单个检测 即文件夹模式是由后端数据来处理
        {
            LOG(INFO) << "This is DIR Dector..." << endl
                      << endl;
            cout << "文件夹检测"
                 << "\n";
            DirDector(); //文件夹检测  //正常模式*****
            cout<<"一组图结束"<<endl;
        }
        else //预防使用    单个图像诊断   云尚诊断  中会有任务暂停情况
        {
            LOG(INFO) << "This is singer file Dector..." << endl
                      << endl;
            //单个图片检测
            cout << "单个图片检测"
                 << "\n";
            for (int i = 0; i < root["fltTaskRecordList"].size(); i++)
            {
                msleep(100);
                if (g_stop == true)
                {
                    LOG(INFO) << "singer file Dector recev pause command..." << endl;
                    m_bIsexit = true;
                    break;
                }

                string t_jsonResult;
                string tfilename = root["fltTaskRecordList"][i]["infraRedName"].asString();
                string zfilename = root["fltTaskRecordList"][i]["visibleLightName"].asString();
                m_iPictureId = root["fltTaskRecordList"][i]["id"].asInt();

                m_sZoomFileName = zfilename;
                m_sThrmFileName = tfilename;

                LOG(INFO) << "m_sThrmFileName: " << m_sThrmFileName << " m_sZoomFileName: " << zfilename << endl;

                bool iscopyKJG =true ;
                SingleFileDetect(tfilename, zfilename, t_jsonResult); //如果这张图片有故障，重新组JSON回复
                cout << "红外检查返回：" << t_jsonResult << endl
                     << endl;

                if (m_sZoomFileName.find("_Z") == string::npos)  //无可见光
                    iscopyKJG = false;
                if (iscopyKJG == true)  
                {
                    string result ,jsonsend ;
                 
                    DetectCallBack(jsonsend, result);
                   
                }
                else           //单张红外  或者单张
                {
                    Json::Value root;

                    root["folderUrl"] = m_sPicturePath;
                    root["visibleLightName"] = m_sZoomFileName; //可见光
                    root["infraRedName"] = m_sThrmFileName;     //红外

                    root["lineId"] = m_iLineId;
                    root["id"] = m_iPictureId;
                    root["imgStatus"] = 0;

                    Json::StyledWriter sw;
                    string jsonsend = sw.write(root);

                    string result;
                    DetectCallBack(jsonsend, result);
                    LOG(INFO) << "result: " << result << endl;
                    LOG(INFO) << endl;
            
                }
            }
        }
    }
}

void DetectThread::SingleFileDetect(string t_filename, string z_filename, string &result)  //指发生在云尚诊断离线诊断 暂停任务的继续诊断 
{
   
    cout<<"单个文件检测,  是单独的 文件任务  主要出现在云尚诊断的项目暂停的情况下"<<endl;
    if (t_filename.find("_T") != string::npos)
    {
        string test_img = m_sPicturePath + "/" + t_filename;
        
        LOG(INFO) << test_img << endl;
        string json_result;

        cv::Mat rgb_img = cv::imread(test_img, cv::IMREAD_COLOR);  // wzj 20230302 修改

        cv::Mat mask;
        float k1 = -0.02;
        float k2 = 0.02;
        float p1 = -0.07;
        float p2 = 1.0-k1-k2-p1;
        // rgb_img =jibian(rgb_img,k1,k2,p1,p2);
        cout << "图像大小:" << rgb_img.cols << endl;
        

        result = json_result;

    }

    // return true;
}

void DetectThread::DirDector() //文件夹检测
{   
   
    vector<string> tFileList;
    if (ReadDirFile(m_sPicturePath, tFileList) == 0)         
        return;
    std::sort(tFileList.begin(),tFileList.end());//wzj 

    for (int i = 0; i < tFileList.size() ; i++)
    {   
        bool peidui=false;
        time_t time1 = time(nullptr);
        msleep(10); // 10   10ms
        cout<<"文件夹文件名:"<<tFileList[i]<<endl;

        if (g_stop == true)
        {
            LOG(INFO) << "Dir Dector recev pause command..." << endl;
            cout<< "收到暂停指令" <<endl;
            m_bIsexit = true;
            break;
        }
        if (tFileList.at(i).find("_T") != string::npos)
        {   
            m_sThrmFileName = tFileList.at(i);

            for(int j = 0; j < tFileList.size() ; j++)
            {   
                // cout<<"测试T文件名编号问题："<<m_sThrmFileName.substr(18,6)<<endl;  兼容御三
                if((tFileList[j].find("Z",0)!=string::npos &&  tFileList[j].find(m_sThrmFileName.substr(18,6),0)!=string::npos)|| (tFileList[j].find("V",0)!=string::npos &&  tFileList[j].find(m_sThrmFileName.substr(18,6),0)!=string::npos))
                {   
                    cout<<"测试Z文件名编号问题："<<tFileList[j].substr(19,5)<<endl;
                    m_sZoomFileName =tFileList[j];
                    peidui =true;
                    break;
                }
            }

             if(!peidui)
            {   
          
                continue;
            }
           
            time_t time3 = time(nullptr);
            // 红外处理模块
            string test_img = m_sPicturePath + "/" + m_sThrmFileName; // hongwai
            string t_jsonresult;
            string zoomResult;
            cout << "test_img:" << test_img << endl;
            // string json_package;

            solarinputdata.dirname = m_sPicturePath;
            solarinputdata.m_timagename = m_sThrmFileName;
            solarinputdata.m_zimagename = m_sZoomFileName;

            // vector<infraFaultJson> infrapackages;
            // vector<visbleFaultJson> visblepackages;
            
            SneSolarAI.run(solarinputdata);



            //*********对含有公共部分取conmonNum*****************//

            //**********融合重新构建红外和可见的合并的**************//
            

            //**********打包JSON 和后端接口**************//
            string RESjson;
            Json::Value root;
            Json::FastWriter writer;
            string SolarDetectPicsPath = "home/sne/data_7t/SolarDetectPics/";
            root["folderUrl"] = SolarDetectPicsPath;
            root["lineId"] = m_iLineId;
            root["infraRedName"] = m_sThrmFileName;
            root["visibleLightName"] = m_sZoomFileName;
            root["savepath"] = _output_path;
            RESjson = writer.write(root);
            SneSolarAI.SolarJsonFusionIVCV(RESjson);

        
            
            // SouthAndNorth(Mat debugZimgDraw,double m_ZGimbalYawDegree);
            

            if(SAVE_SOLASR) 
            {   
                SneSolarAI.SolarIvCvReProcess(DEBUG_DRAW);
            }


            string t_result;
            DetectCallBack(RESjson, t_result);

        }

        time_t time2 = time(nullptr);
        cout<<"每轮程序运行时间："<<time2-time1<<" s"<<endl;
      
    }
    if (tFileList.at(tFileList.size() - 1).find("_T") != string::npos) //判断最后一张是T还是Z
    {
        //根节点属性
        Json::Value root;

        root["folderUrl"] = m_sPicturePath;
        root["visibleLightName"] = tFileList.at(tFileList.size() - 1); //可见光
        root["infraRedName"] = tFileList.at(tFileList.size() - 1);     //红外

        root["lineId"] = m_iLineId;
        root["id"] = m_iPictureId;
        root["imgStatus"] = false;

        Json::StyledWriter sw;
        string t_sendjson = sw.write(root);

        string t_result;
        DetectCallBack(t_sendjson, t_result);
        cout<<"t_result: "<<t_result<<endl;
    }

    

   
}


bool DetectThread::ReadDirFile(string strPath, vector<string> &fileList) //获取文件夹所有文件名字，不区分可见和红外
{
    DIR *dir;
    struct dirent *ptr;
    dir = opendir(strPath.c_str());

    if (dir == NULL)
    {
        LOG(INFO) << "open dir failed..." << endl;
        closedir(dir);
        return false;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        string filename;
        filename = ptr->d_name;
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) /// current dir OR parrent dir
            continue;

        if (filename.find(".JPG") != string::npos || filename.find(".jpg") != string::npos|| filename.find(".jpeg") != string::npos)
            fileList.push_back(filename); //传进来的fileList为空Vector 因为是传地址 所以用->
                                        //   cout<<"文件夹文件名:"<<filename<<"\t"<<"随机读取"<<"\n";   //
    }
    closedir(dir);
    return true;
}


void DetectThread::DetectCallBack(string jsonArray, string &result)                //必须
{
    try
    {
        cout<<" json测试" << jsonArray <<endl;
        // json
        string body(jsonArray);

        // uri
        Poco::URI uri("http://127.0.0.1:8050/facility/task/fltTaskBack");
        //        Poco::URI uri("http://172.16.201.49:9216/task/fltTaskBack");
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        session.setKeepAlive(true);
        // request
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), Poco::Net::HTTPRequest::HTTP_1_1);
        request.setContentType("application/json");
        request.setContentLength(body.length());
        session.sendRequest(request) << body;

        // response
        Poco::Net::HTTPResponse res;
        string recv_string;
        istream &is = session.receiveResponse(res);
        int t_result = (int)res.getStatus();
        // cout << "result:" << result << ", reason:" << res.getReason() << std::endl;
        Poco::StreamCopier::copyToString(is, recv_string);
        // cout << "recv : " << std::endl << recv_string << std::endl;
        result = recv_string;
    }
    catch (Poco::Exception &exc)
    {
        std::cerr << exc.displayText() << std::endl;
        return;
    }
}




vector<string> DetectThread::splitStr(string str, char delimiter)
{
    vector<string> r;
    string tmpstr;
    while (!str.empty())
    {
        int ind = str.find_first_of(delimiter);
        if (ind == -1)
        {
            r.push_back(str);
            str.clear();
        }
        else
        {
            r.push_back(str.substr(0, ind));
            str = str.substr(ind + 1, str.size() - ind - 1);
        }
    }
    return r;
}

// template <class Type>
double DetectThread::stringToNum(const string &str)
{
    istringstream iss(str);
    double num;
    iss >> num;
    return num;
}



void DetectThread::msleep(uint32_t value)
{
    usleep(value * 1000); // 1ms
}


//获取方阵的点集
void DetectThread::GetSquareInfo(Json::Value root)
{

    squareinfor.clear();
    for (int i = 0; i < root["fltSquareArrayList"].size(); i++)
    {
        string matrixKmlName = root["fltSquareArrayList"][i]["matrixKmlName"].asString();

        string coordinates = root["fltSquareArrayList"][i]["coordinates"].asString();

        vector<string> strgps;

        SplitString(coordinates, strgps, "],[");
        StringtoGps(matrixKmlName, strgps, squareinfor);
    }

    /*
    map <string,vector<PlotInfo>>::iterator  iter;
    for(iter = squareinfor.begin(); iter != squareinfor.end(); iter++)
    {
        cout << iter->first << endl;
        for(int i = 0;i < iter->second.size();i++ )
            cout << iter->second.at(i).longitude << "  " << iter->second.at(i).latitude << endl;
    }
    */
}

//分割WEB端传过来字符串
//"[121.8257956264,38.5870601408],[100.3068375118,38.7062343576],[103.6547203823,37.427069415],[101.6547203823,37.427069415],[99.8257956264,38.5870601408]"
// 121.8257956264,38.5870601408  结果
void DetectThread::SplitString(const string &s, vector<string> &v, const string &c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (string::npos != pos2)
    {
        string value;
        value = s.substr(pos1, pos2 - pos1);

        if (value.find("[") != string::npos)
            value = value.substr(pos1 + 1, pos2 - pos1);

        else if (value.find("]") != string::npos)
            value = value.substr(pos1, pos2 - pos1 - 1);

        v.push_back(value);

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }

    if (pos1 != s.length())
    {
        string value = s.substr(pos1);
        if (value.find("]") != string::npos)
        {
            v.push_back(value.substr(0, value.find("]") - 1));
        }
    }
}

//所有的信息保存在Map结构中
void DetectThread::StringtoGps(const string squarename, vector<string> str, map<string, vector<PlotInfo>> &square)
{
    string::size_type pos1;
    vector<PlotInfo> V_gps;
    for (int i = 0; i < str.size(); i++)
    {
        PlotInfo gps;
        pos1 = str.at(i).find(",");
        gps.longitude = str.at(i).substr(0, pos1);
        gps.latitude = str.at(i).substr(pos1 + 1);
        V_gps.push_back(gps);

        // cout << "gps.longitude: " << gps.longitude << " " << "gps.latitude: " << gps.latitude << endl;
    }
    square.insert(pair<string, vector<PlotInfo>>(squarename, V_gps));
}







