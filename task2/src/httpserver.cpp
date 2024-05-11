#include <httpserver.h>

// #include "detector.h" cousin20230601
// #include "interface.h"  cousin20230601

#include <sstream>



RootHandler::RootHandler()
{

}

RootHandler::~RootHandler()
{
    _thread.join();
}

void RootHandler::handleRequest(HTTPServerRequest& request,HTTPServerResponse& response)
{
    Poco::StreamCopier::copyToString(request.stream(), m_sRecv);

    //LOG(INFO) << "m_sRecv: " << m_sRecv << endl;

    Json::Value root;                          // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array
    Json::Reader reader;  
    if(reader.parse(m_sRecv,root))
    {
        //m_bStopFlag = root["stopFlag"].asBool();                        //停止标志位
		m_bStopFlag = root["stopFlag"].asInt();                        //停止标志位
		cout << "m_bStopFlag: " << m_bStopFlag << endl;
        g_stop = m_bStopFlag;      //g_stop 为1表示退出检测
    }

    if(m_bStopFlag == 0)     //0 表示开始处理图片
    {
        /*if(g_flag == true)    //表示正在处理图片
        {
                LOG(INFO) << "Recev new task and old asks are working return NO..." << endl;
                response.setChunkedTransferEncoding(true);
                response.setContentType("text/html");
                char ack[] = "NO";
                response.sendBuffer((const void*)ack,sizeof(ack));
        }
        else  */                //表示图片处理结束
        {
                g_flag = true;
                gvec_sRecev.insert(gvec_sRecev.begin(),m_sRecv);
                // g_sRecev. = m_sRecv;
				g_setParam = false;
				g_setParam = false;
                response.setChunkedTransferEncoding(true);
                response.setContentType("text/html");
                char ack[] = "OK";
                response.sendBuffer((const void*)ack,sizeof(ack));
        }
    }
	else if (m_bStopFlag == 2)//接收配置参数
	{
		g_flag = true;
        gvec_sRecev.insert(gvec_sRecev.begin(),m_sRecv);
		// g_sRecev = m_sRecv;
		g_setParam = true;//接收配置参数
		cout <<"receiving parameter..." << endl;
		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		char ack[] = "OK";
		response.sendBuffer((const void*)ack, sizeof(ack));
	}
	else if (m_bStopFlag == 3)//发送配置参数
	{
		cout << "sending parameter..." << endl;
		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		//g_sendParam =true;//发送配置参数
		// detector.send_seted_para(json_seted_para);
		//char *ack = (char *)json_seted_para.c_str();
		response.sendBuffer((char *)json_seted_para.c_str(), json_seted_para.length());
	}
	else{
        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");
        char ack[] = "OK";
        response.sendBuffer((const void*)ack,sizeof(ack));
    }
}

int RootHandler::StartDetect()
{
    HandRecvData();
}

bool RootHandler::SingleFileDetect(string t_filename,string z_filename,string& result)
{
    bool ret;
    if(t_filename.find("_T") != string::npos)           //不使用
    {
            string test_img = m_sPicturePath + "/" + t_filename;
            cout << test_img << endl;
            string json_result;
            //ret = solution(test_img, 0, detector, json_result);
            if(ret == true)
            {
                //回调处理后的结果
                //Compress(z_filename);
            }
            cout << json_result << endl;
    }
    return ret;
}

bool RootHandler::HandRecvData()
{
    Json::Reader reader;    // 解析json用Json::Reader
    Json::Value root;       // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array

    if(reader.parse(m_sRecv,root))
    {
        m_sPicturePath = root["folderUrl"].asString();               //图片的存储路径
        cout << "m_sPicturePath: " << m_sPicturePath << endl;

        m_sLineName = root["lineName"].asString();                   //航线名
        cout << "m_sLineName: " << m_sLineName << endl;
        
        m_iLineId = root["id"].asInt();                        //航线ID
        cout << "m_iLineId: " << m_iLineId << endl;

        m_sDetectionType = root["diagnoseType"].asString();              //类型   是单个文件检测   还是文件夹检测
        cout << "m_sDetectionType: " << m_sDetectionType << endl;

        m_bStopFlag = root["stopFlag"].asInt();                        //停止标志位
        cout << "m_bStopFlag: " << m_bStopFlag << endl;

        m_modelType = root["modelType"].asInt();     // 高低功率
        
        m_modelTypeMx = root["modelTypeMx"].asInt();  //地理的模型

        if(m_bStopFlag == 0)
        {
            if(m_sDetectionType == "d")
            {
                DirDector();
            }
            else
            {
                cout << "---------------------" << endl;
                //单个图片检测
                for(int i =0;i<root["fltTaskRecordList"].size();i++)
                {
                    //if(m_bStop != true)
                      //  break;

                    string jsonResult;
                    string tfilename = root["fltTaskRecordList"][i]["infraRedName"].asString();
                    string zfilename = root["fltTaskRecordList"][i]["visibleLightName"].asString();
                    m_iPictureId = root["fltTaskRecordList"][i]["id"].asInt();
                    
                    m_sZoomFileName = zfilename;
                    m_sThrmFileName= tfilename;
                    
                    cout << "tfilename: " << tfilename << endl;
                    cout << "zfilename: " << zfilename << endl;

                    bool iscopyKJG = SingleFileDetect(tfilename,zfilename,jsonResult);    //如果这张图片有故障，重新组JSON回复
                    
                    if(iscopyKJG == true)
                    {
                        string jsonsend;
                    
                        PacketJson(jsonResult,jsonsend);     
                        string result;            
                        DetectCallBack(jsonsend,result);
                        cout << "result: " << result;
                    }
                    else{
                        Json::Value root;

                        root["folderUrl"] = m_sPicturePath;   
                        root["visibleLightName"] = m_sZoomFileName;        //可见光
                        root["infraRedName"] = m_sThrmFileName;            //红外

                        root["linId"] = m_iLineId;                   
                        root["id"] = m_iPictureId;
                        root["imgStatus"] = 0;

                        Json::StyledWriter sw;

                        cout << sw.write(root) << endl << endl;
                    }
                }
            }
        }
        else if(m_bStopFlag == false)
        {
            m_bStop = false;
        }
    }
    else
    {
        return false;
    }

    return true;
}



void RootHandler::DirDector()
{
    vector<string> tFileList;
    AdjustFileList(m_sPicturePath,tFileList);
    for(int i = 0; i < tFileList.size();i++)
    {
        cout << tFileList.at(i) << endl;
        if(tFileList.at(i).find("_T") != string::npos)
        {
            string test_img = m_sPicturePath + "/"  + tFileList.at(i);
            cout << test_img << endl;
            string t_jsonresult;
            bool t_iscopyKJG; 
            //solution(test_img, 0, detector, t_jsonresult);
            cout << t_jsonresult << endl;
            if(t_iscopyKJG == true)
            {
                string t_sendjson;
                PacketJson(t_jsonresult,t_sendjson);
                string t_ack;
                DetectCallBack(t_sendjson,t_ack);
                if(t_ack == "OK")
                {
                    cout << "A picture is handled" << endl;

                }
                else {
                    cout << "Recev Ack is error" << endl;
                    break;
                }

            }
            else{
                //根节点属性
                    Json::Value root;

                    root["folderUrl"] = m_sPicturePath;   
                    root["visibleLightName"] = m_sZoomFileName;        //可见光
                    root["infraRedName"] = m_sThrmFileName;            //红外

                    root["linId"] = m_iLineId;                   
                    root["id"] = m_iPictureId;
                    root["imgStatus"] = t_iscopyKJG;

                    Json::StyledWriter sw;
                    cout << sw.write(root) << endl << endl;
                    string t_sendjson = sw.write(root);
                    string t_result;;
                    DetectCallBack(t_sendjson,t_result);

            }
        }
    }

}

int RootHandler::AdjustFileList(string strPath,vector<string> &fileList)
{
    string temp;

    ReadDirFile(strPath,fileList);

    for(int i = 0; i < fileList.size() - 1; i++)
    {
        for(int j = 0; j < fileList.size() - i - 1; j++)
        {
            if(strcmp(fileList.at(j).c_str(), fileList.at(j + 1).c_str()) > 0)
            {
                temp = fileList[j];
                fileList[j] = fileList[j + 1];
                fileList[j + 1] = temp;
            }
        } 
    }

    return 1;
}

int RootHandler::ReadDirFile(string strPath,vector<string> &fileList)
{
    DIR *dir;
    struct dirent *ptr;
    dir = opendir(strPath.c_str());

    if(dir == NULL)
    {
        cout << "open dir failed..."<< endl; 
    }

    while ((ptr=readdir(dir)) != NULL)
    {
       string filename;
       filename = ptr->d_name;
       if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
             continue;
       
       if(filename.find(".JPG") != string::npos || filename.find(".jpg") != string::npos)
            fileList.push_back(filename);
            //cout << filename << endl;
     }
     closedir(dir);
     return 1;
}


void RootHandler::PacketJson(string srcjson,string& outjson)                    //有故障的图片，重新组JSON 回复
{
    Json::Reader reader;
    Json::Value root;
    Json::Value out_root;

    getGPS(m_sPicturePath +"/" + m_sThrmFileName,m_dTx,m_dTy);
    getGPS(m_sPicturePath + "/" + m_sZoomFileName,m_dZx,m_dZy);

    if(reader.parse(srcjson,root))
    {
        double jd,wd;
        m_dBlc = root["pixScale"].asDouble();                     //组件比例尺
        for(int i =0;i < root["Faultlist"].size();i++)
        {    
            //定义精度，纬度，比例尺，像素
            istringstream ss;
            double jd,wd,xsx,xsy;
            double longitude,latitude;
				 
            string faultpic = root["Faultlist"][i]["faultpic"].asString();      //小图文件名
            string faulttype = root["Faultlist"][i]["faulttype"].asString();
            string xs = root["Faultlist"][i]["pixinfo"].asString();             //相对像素点

            vector<string> vxs = splitStr(xs,',');      //x,y 像数点分割 "-71,-73"
            xsx=stringToNum(vxs[0]);
            xsy=stringToNum(vxs[1]);

            GetFaultGPS(xsx,xsy,jd,wd);      // 获取故障组件的经纬度

            //组repose 的 json

            //子节点
            Json::Value partner;
            char gpsinfo[60];
            sprintf(gpsinfo,"%lf,%lf",jd,wd);
            //子节点属性
            partner["moduleGps"] = gpsinfo;      //组件坐标
            partner["faultType："] = faulttype;           //故障类型
            partner["faultCause"] = "";         //诱发原因
            partner["faultpic"] = faultpic;           //小图
            
            //数组形式
            out_root["fltTaskRecordList"].append(partner);

        }
            //根节点属性
	        out_root["folderUrl"] = m_sPicturePath;
            out_root["visibleLightName"] = m_sZoomFileName;        //可见光
            out_root["infraRedName"] = m_sThrmFileName;            //红外

            out_root["linId"] = m_iLineId;                   
	        out_root["id"] = m_iPictureId;
            out_root["imgStatus"] = 1;

            Json::StyledWriter sw;
	        cout << sw.write(out_root) << endl << endl;
    }   
}


void RootHandler::DetectCallBack(string jsonArray,string& result)
{
    try
    {
        // json
        string body(jsonArray);

        // uri
        Poco::URI uri("http://127.0.0.1:1050/facility/task/fltTaskBack");     //8050  cousin 20230601
        // Poco::URI uri("http://127.0.0.1:1070/facility/task/fltTaskBack");  //8070  cousin 20230601
        
        
        //Poco::URI uri("http://127.0.0.1:1080/");
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
        istream & is = session.receiveResponse(res);
        int result = (int)res.getStatus();
        cout << "result:" << result << ", reason:" << res.getReason() << std::endl;
        Poco::StreamCopier::copyToString(is, recv_string);
        cout << "recv : " << std::endl << recv_string << std::endl;

        string recv_body = res.get("body");
        cout << "body : " << std::endl << recv_body << std::endl;
    }
	catch (Poco::Exception& exc)
	{
		std::cerr << exc.displayText() << std::endl;
        return;
	}
}


void RootHandler::GetFaultGPS(double xsx,double xsy,double& jd,double& wd)     // 输入是故障组件的相对像数点
{
        
        //换算地理位置
        //算经纬度
            //东西向
            if(abs(m_dZx - m_dTx)>abs(m_dZy - m_dTy))
            {
                //西->东
                //纬度-x 精度+y
                if(m_dZx > m_dTx)
                {
                    cout<<"1"<<endl;
                    jd = m_dTx + (double)(xsy/m_dBlc)*0.00001141;
                    wd = m_dTy - (double)(xsx/m_dBlc)*0.00000899;
                }
                //东->西
                //纬度+x 精度-y
                else
                {
                    cout<<"2"<<endl;
                    jd = m_dTx - (double)(xsy/m_dBlc)*0.00001141;
                    wd = m_dTy + (double)(xsx/m_dBlc)*0.00000899;
                }
            }
            else
            //南北向
            {
                   //南->北
                   //精度+x 纬度+y
                   if(m_dZy > m_dTy)
                   {
                      cout<<"3"<<endl;
                      jd = m_dTx + (double)(xsx/m_dBlc)*0.00001141;
                      wd = m_dTy + (double)(xsy/m_dBlc)*0.00000899;
                   }
                   else
                   //北->南
                   //精度-x 纬度-y
                   {
                      cout<<"4"<<endl;
                      jd = m_dTx - (double)(xsx/m_dBlc)*0.00001141;
                      wd = m_dTx - (double)(xsy/m_dBlc)*0.00000899;
                   }
            }
}

void RootHandler::Compress(string z_filename)
{
     string in_filename = m_sPicturePath + z_filename;
     Mat img = imread(in_filename);
 
     vector<int> compression_params;

     compression_params.push_back(IMWRITE_JPEG_QUALITY);
     compression_params.push_back(30);                         //在这个填入你要的图片质量

    string out_jpg = m_sPicturePath + z_filename;
    cout << "out_jpg: " << out_jpg <<endl;
    imwrite(out_jpg, img, compression_params);
}

vector<string> RootHandler::splitStr(string str, char delimiter)
{
        vector<string> r;
        string tmpstr;
        while (!str.empty()){
            int ind = str.find_first_of(delimiter);
            if (ind == -1){
                r.push_back(str);
                str.clear();
            }
            else{
                r.push_back(str.substr(0, ind));
                str = str.substr(ind + 1, str.size() - ind - 1);
            }
        }
        return r;
}

//template <class Type>  
double RootHandler::stringToNum(const string& str)  
{  
    istringstream iss(str);  
    double num;  
    iss >> num;  
    return num;      
}

int RootHandler::getGPS(string filepath,double& x,double& y)
{
  // Read the JPEG file into a buffer
  FILE *fp = fopen(filepath.c_str(), "rb");
  if (!fp) {
    printf("Can't open file.\n");
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  unsigned long fsize = ftell(fp);
  rewind(fp);
  unsigned char *buf = new unsigned char[fsize];
  if (fread(buf, 1, fsize, fp) != fsize) {
    printf("Can't read file.\n");
    delete[] buf;
    return -2;
  }
  fclose(fp);

  // Parse EXIF
  easyexif::EXIFInfo result;
  int code = result.parseFrom(buf, fsize);
  delete[] buf;
  if (code) {
    printf("Error parsing EXIF: code %d\n", code);
    return -3;
  }
  
  x=result.GeoLocation.Longitude;
  y=result.GeoLocation.Latitude;
  //anger = result.
  
}
