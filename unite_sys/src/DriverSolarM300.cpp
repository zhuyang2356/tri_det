/*
 * DriverSolarM300.cpp
 *
 *  Created on: Feb 24, 2023
 *      Author: sne
 */

#include "../inc/DriverSolarM300.h"
#include "../inc/ManagerBase.h"
// #include "../inc/detectthread.h"
// #include "pretreat_mask.h"
#include <sstream>
// #include "exif.h"

// #include "../inc/detector.h"
// #include "../inc/interface.h"
#include "string"
// #include "yoloxDetect.h"
// #include "yolo-seg.h"
// #include "YOLO5Detector.h"
#include "ctime"
#include "glog/logging.h"
//#include "../inc/SneMinIOManager.h"
#include "../inc/ShareStruct.h"
#include "../inc/DriverBase.h"
#define SNE_PI 3.14159265359
#define SNE_Z_w_pix 4000
#define SNE_Z_h_pix 3000
#define SNE_T_w_pix 1280
#define SNE_T_h_pix 1024
#define SNE_Z_w_size 6.4
#define SNE_Z_h_size 4.8
#define SNE_T_w_size 7.68
#define SNE_T_h_size 6.144

#define JSONLOG
#define ZOOMDETECT
#define THDEFAULT "电池片损坏"
#define SAVEDIR "./zoomres" //可见光处理的小图保存路径
#define SAVE_SOLASR  true
#define DEBUG_DRAW true
bool g_flag = false;        //这是用来？
//string g_sRecev;
vector<string> gvec_sRecev;
string json_seted_para; //
bool g_sendParam = false;
bool g_stop;
bool debug = false;
// bool superres =false;


DriverSolarM300::DriverSolarM300() {
	ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)初始化2.\n")));
    // TODO Auto-generated constructor stub
	pManager_= 0;
	cout << "诊断初始化" << endl;
    _output_path = "../results/pos/";
    m_Stop = 0;
}
DriverSolarM300::~DriverSolarM300() {
	// TODO Auto-generated destructor stub
	cout << "诊断完成" << endl;
}

int DriverSolarM300::Init(string procName, ManagerBase *pManager) {
	pManager_ = pManager;
	//pDetectTask_;
	return 1;
}


int DriverSolarM300::Open() {
	return 1;
}

int DriverSolarM300::Close() {
	return 1;
}

int fn_pos(string T, string P, int n){
	if(n==0)return -1;//相当于没找
	int count = 0;
	unsigned begined = 0;
	while((begined=T.find(P,begined))!=string::npos){
		count++;
		begined += P.length();
		if(n==count){
			return begined-1;
		}
	}
	return -2;//意味着没有那么多次
}

std::vector<string> listDir(string upObjLocalDir){
	std::vector<string> fileNames;
	//		遍历文件夹下的文件，上传到s3目标里--------------------
	DIR *pDir;
	struct dirent* ptr;
	if(!(pDir = opendir(upObjLocalDir.c_str()))){
		cout<<"Folder doesn't Exist!"<<endl;}
	while((ptr = readdir(pDir))!=0) {
		if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
			fileNames.push_back(ptr->d_name);
	}
	}
	delete pDir;
	delete ptr;
	return fileNames;
}

int DriverSolarM300::PutTaskMessage(int tasktype, int taskNo, ACE_Message_Block* recvMsg)
{
	try{
    ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)111111111111.\n")));
	string strTaskMessage(recvMsg->rd_ptr(), recvMsg->size());
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)接收到诊断任务houssi------------：【%d-%d】%s.\n"), tasktype, taskNo, strTaskMessage.c_str()));	
	json infoObj= json::parse(strTaskMessage);
    json infoObj_imp;
    for(int i=0;i<infoObj["DirList"].size();i++){
        DirInfo=infoObj["DirList"][i]["folderUrl"];
        bktNamePos=fn_pos(DirInfo,"/",2);
        bktName=DirInfo.substr(0,bktNamePos);
        //preFixPath,s3目录文件夹去除了桶名字以后剩下的目录,例如/pvStore/images/杨村小李仓库1/DJI_202305291410_小李测试航线/original
        rawPreFixPath=DirInfo.substr(bktNamePos);
        preFixPath=DirInfo.substr(bktNamePos);
        // 将路径里的空格键替换为_ 
        //例如："Dongdawei Photovoltaic Power Station Diagnosis Warehouse"
        // Dongdawei_Photovoltaic_Power_Station_Diagnosis_Warehouse
        replace(preFixPath.begin(),preFixPath.end(),' ','_');
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)下載文件信息-------：【%d-%s】\n %s. \n"), bktNamePos, bktName.c_str(), preFixPath.c_str()));	
        // pathKey本地从S3下载图片的根目录，后面必须没有/
        // pathKey="/home/sne/data_7t/tri_det/imgs/src_imgs";
        pathKey="./imgs/src_imgs";
        // attStar="/*";
        // downloadDir=pathKey+preFixPath+attStar;

        //在下载文件之前，清空目标文件夹
        downloadDir=pathKey+preFixPath;
        int resDel=Remove(downloadDir);
        if (resDel==0)
        {
            ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)文件夹:%s:删除.\n"), downloadDir.c_str()));	
        }else{
            ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)文件夹:%s:没有删除.\n"), downloadDir.c_str()));	
        }
        // picNum=pManager_->downloadfile(bktName,preFixPath,pathKey,rawPreFixPath);
        picNum=pManager_->downloadfile(bktName,preFixPath,pathKey);
        // 替换进入算法的info的路径
        infoObj["DirList"][i]["folderUrl"]=pathKey+"/"+preFixPath;

    }
    // 进入算法运行
	processedInfo=infoObj.dump();
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)输入算法的参数:-- %s.\n"),processedInfo.c_str()));
	vector<string> gvec_sRecev;
	gvec_sRecev.insert(gvec_sRecev.begin(),processedInfo);
	run(gvec_sRecev);
	m_result_size=m_result.size();
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)-------------本次任务完结:---------------.\n")));
	recvMsg->release();
	pManager_->Retask(tasktype, taskNo);
    return 1;
    }
    catch(int){
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)使用了M300照片:--.\n")));
        recvMsg->release();
        pManager_->Retask(tasktype, taskNo);
        return 0;
    }
    catch(const std::exception &e){
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)进入了const报的error.\n")));
        string strBack=e.what();
        ACE_Message_Block * taskMsg = new ACE_Message_Block(strBack.size());
        taskMsg->copy(strBack.c_str(), strBack.size());
        pManager_->PutDetectMessage(1,1,taskMsg);
        recvMsg->release();
        pManager_->Retask(tasktype, taskNo);
        return 0;
    }
}

//目前只有一种故障，发来了m300的图片
void DriverSolarM300::DetectCallBackError(int lineId){
    json errorBack;
    errorBack["code"]=500;
    errorBack["msg"]="上传图片有M300图片";
    errorBack["lineId"]=lineId;
    string strBack=errorBack.dump();
    ACE_Message_Block * taskMsg = new ACE_Message_Block(strBack.size());
    taskMsg->copy(strBack.c_str(), strBack.size());
    pManager_->PutDetectMessage(1,1,taskMsg);
    ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)使用了M300照片:发回给mq的信息-- %s.\n"),strBack.c_str()));
}

void DriverSolarM300::DetectCallBack(string jsonArray)
{
    try
    {
        cout<<" json测试最终结果" << jsonArray <<endl;
		mResJson= json::parse(jsonArray);
		//如果返回结果字符串里发现有faultlist，那么判定是检测出问题
		if(mResJson.find("visibleFaultList") == mResJson.end() && mResJson.find("infraFaultList") == mResJson.end()){
			//没有检出问题，返回诊断任务结果
			mResJson["folderUrl"]=bktName+"/"+preFixPath;
			mResJson["imgStatus"]=0;
            mResJson["lineId"] = m_iLineId;
			strBack=mResJson.dump();
			ACE_Message_Block * taskMsg = new ACE_Message_Block(strBack.size());;
			taskMsg->copy(strBack.c_str(), strBack.size());
			pManager_->PutDetectMessage(1,1,taskMsg);
            ACE_DEBUG((LM_INFO,ACE_TEXT("\n(%D)在算法里返回结果没有检测出缺陷 \n 算法返回给mq的字符串:: %s\n"),strBack.c_str()));
			// sleep(10);
		}
		else{			
			//返回诊断任务结果
			mResJson["bkMsg"]=32;
            mResJson["folderUrl"]=bktName+"/"+preFixPath;
            mResJson["lineId"] = m_iLineId;

			// 返回json前上传小图和大图
            int ori_len=DirInfo.find_last_of("/")-bktNamePos;
            rectifyUpObjDir=DirInfo.substr(bktNamePos,ori_len)+"/rectify";
            bigInfraRedPic=mResJson["infraRedName"];
            bigVisLgtPic=mResJson["visibleLightName"];
            flawFileNames.insert(bigInfraRedPic);
            flawFileNames.insert(bigVisLgtPic);
            //----------------------------------------------------------
			//上传文件到文件夹里， 
			set<string>::iterator it=flawFileNames.begin();
			while(it!=flawFileNames.end()){
				fileNameHeader=it->substr(0,3);
				ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D) 图片文件头三个名字:%s .\n"),fileNameHeader.c_str()));
                UpLocalPath=faultimg_pos+*it;
                UpObjPath=rectifyUpObjDir+"/"+*it;
                ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D) bktName:%s.\n preFixPath:%s \n rectifyUpObjDir:%s \nUpLocalPath:%s \n UpObjPath :%s"),
            bktName.c_str(),preFixPath.c_str(),rectifyUpObjDir.c_str(),UpLocalPath.c_str(),UpObjPath.c_str()));
                isSuccess=pManager_->uploadfile(bktName,UpObjPath,UpLocalPath);
				if (isSuccess){
					ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)--上传成功--\n bktName:%s 上传到路径:%s \n 本地文件路径:%s.\n"),
							bktName.c_str(),UpObjPath.c_str(),UpLocalPath.c_str()));
				}
				else{
					ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)--上传失败--\n bktName:%s 上传到路径:%s \n 本地文件路径:%s.\n"),
							bktName.c_str(),UpObjPath.c_str(),UpLocalPath.c_str()));
				}
				it++;
			}
			//清空文件集合
			flawFileNames.clear();
            //返回json
            strBack=mResJson.dump();
			ACE_Message_Block * taskMsg = new ACE_Message_Block(strBack.size());;
			taskMsg->copy(strBack.c_str(), strBack.size());
			pManager_->PutDetectMessage(1,1,taskMsg);
            ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)	在算法里检测出问题-----返回结果:-----.\n %s \n"),strBack.c_str()));
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)\n  ")));
		}
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)-------------算法输出结果分隔行:---------------.\n")));
        return;
    }
    catch (...)
    {
        std::cerr << "DETECTCALLBACK异常" << std::endl;
        return;
    }
}

void DriverSolarM300::run(vector<string> gvec_sRecev)
{
    // CV_VERSION是const_char，所以不需要带个c_str
    ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)cv版本-------：【%s】. \n"), CV_VERSION));	

    // 每一次进入算法的时候，算法的返回结果先清空一遍 important notice 
    m_result.clear();
    {
        LOG(INFO) << "g_sRecev: "  << endl;
        cout << "detectThread从这里开始:gvec_sRecev信息" << gvec_sRecev.size()<<endl;
        for(int i=0;i<gvec_sRecev.size();i++){
            cout<<"倚天屠龙"<<gvec_sRecev[i]<<endl;
            ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)下倚天屠龙-------：【%s】. \n"), gvec_sRecev[i].c_str()));	
        }
        auto iter = gvec_sRecev.begin();
        if(iter != gvec_sRecev.end())
        {
            m_sRecv = *iter;
            gvec_sRecev.erase(iter);
            cout<<"从这里开始进行解决"<<endl;
            ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)从这里开始进行解决. \n")));
            HandRecvData(); //目前使用这个
            g_flag = false;
        }
    }
}

bool DriverSolarM300::HandRecvData()
{
    Json::Reader reader; // 解析json用Json::Reader
    Json::Value root;    // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array
    Json::Value mainroot;
    cout << "m_bStopFlag:123 " << endl;
    if (reader.parse(m_sRecv, mainroot))
    {
        m_bStopFlag = mainroot["stopFlag"].asInt(); //停止标志位
        LOG(INFO) << "m_bStopFlag: " << m_bStopFlag << endl;

        for (int i = 0; i < mainroot["DirList"].size(); i++)
        {
            root = mainroot["DirList"][i]; //多少个需要在线诊断
            if (m_bStopFlag == false)
            {
                ImagesDetect(m_bStopFlag, root); //该函数调用了ImageDetect 中的文件夹检测
            }
            else
            {
                LOG(INFO) << "Detect is end..." << endl;
            }
            if (g_stop == true)
            {
                LOG(INFO) << "-==------" << endl;
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
        return false;
    }

    m_bIsexit = false;
    return true;
}

void DriverSolarM300::ImagesDetect(bool status, Json::Value root) //在run中被调用
{
    if (status == false) //开始检测 状态为0表示开始
    {
        m_sPicturePath = root["folderUrl"].asString(); //图片的存储路径
        LOG(INFO) << "m_sPicturePath: " << m_sPicturePath << endl;

        m_sLineName = root["lineName"].asString(); //航线名
        LOG(INFO) << "m_sLineName: " << m_sLineName << endl;

        m_iLineId = root["lineId"].asDouble(); //航线ID
        LOG(INFO) << "m_iLineId: " << m_iLineId << endl;

        m_sDetectionType = root["diagnoseType"].asString(); //类型   是单个文件检测   还是文件夹检测
        LOG(INFO) << "m_sDetectionType: " << m_sDetectionType << endl;

        m_modelType = root["modelType"].asInt(); // 高低功率

        m_modelTypeMx = root["modelTypeMx"].asInt(); //地理的模型

        m_SquareArrayFlag = root["squareArrayFlag"].asInt();

        if (m_SquareArrayFlag == 1)
        {
            GetSquareInfo(root);
        }

        LOG(INFO) << "getType sucess: " << getType() << endl;
        //读取高低功率模型
        //        detectorSolar.SolarReadOnnx(getType());

        LOG(INFO) << "getType sucess , " << getType() << endl;
        //使用文件夹检测
        if (m_sDetectionType == "d")
        {
            LOG(INFO) << "This is DIR Dector..." << endl
                      << endl;
            cout << "文件夹检测"
                 << "\n";
            DirDector(); //文件夹检测  //正常模式*****
        }
        //使用单个图片检测
    }
}


void DriverSolarM300::DirDector() //文件夹检测
{   
    try{
    vector<string> tFileList;
    // if (AdjustFileList(m_sPicturePath, tFileList) == 0) //
    //     return;
    if (ReadDirFile(m_sPicturePath, tFileList) == 0)
        return;
    std::sort(tFileList.begin(),tFileList.end());//wzj 
    std::vector<time_t> time_list(13);
    faultimg_pos = "../results/SneSolarPics/";
// 红外图地址、可见光地址、上传图片地址
    for (int i = 0; i < tFileList.size() ; i++)
    {   
        time_t time1 = time(nullptr);
        // string resJson=pure_driver.pDriverBase->pure_algori(tFileList,i,m_sPicturePath,faultimg_pos);
        bool peidui=false;
        cout<<"文件夹文件名:"<<tFileList[i]<<endl;
        if (tFileList.at(i).find("_T") != string::npos)
        {   
            m_sThrmFileName = tFileList.at(i);
            for(int j = 0; j < tFileList.size() ; j++)
            {   
                if(tFileList[j].find("Z",0)!=string::npos && tFileList[j].find(m_sThrmFileName.substr(18,6),0)!=string::npos)
                {  
                    cout<<"测试Z文件名编号问题："<<tFileList[j].substr(19,5)<<endl;
                    m_sZoomFileName =tFileList[j];
                    peidui=true;
                    break;
                }
            }
            if(!peidui){
                return;
            }
        }
        solarinputdata.dirname = m_sPicturePath;
        solarinputdata.m_timagename = m_sThrmFileName;
        solarinputdata.m_zimagename = m_sZoomFileName;
        // vector<infraFaultJson> infrapackages;
        // vector<visbleFaultJson> visblepackages;
        cout<<"测11111"<<endl;
        SneSolarAI.run(solarinputdata);
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
            faultimg_pos = "../results/SneSolarPics/";
            SneSolarAI.SolarJsonFusionFGX(RESjson,faultimg_pos);
            
            // 演示用
            // SouthAndNorth(Mat debugZimgDraw,double m_ZGimbalYawDegree);
        

            string t_result;
        DetectCallBack(RESjson);
        time_t time2 = time(nullptr);
        cout<<"每轮程序运行时间："<<time2-time1<<" s"<<endl;
        time_t tot_time=time2-time1;
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)每轮程序运行时间:-%d..\n"),tot_time));
    }
    // unloadPureAlgor();
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

        DetectCallBack(t_sendjson);

        LOG(INFO) << endl;
    }
    } catch(...){
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)抓到了错误.\n")));
    }
}


int DriverSolarM300::getType()
{
    int type = 0x01;
    if (m_modelType == 0)
    {
        type = m_modelTypeMx;
    }
    else if (m_modelType == 1)
    {
        type = 0x10 + m_modelTypeMx;
    }
    return type;
}

//获取方阵的点集
void DriverSolarM300::GetSquareInfo(Json::Value root)
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

}


//分割WEB端传过来字符串
//"[121.8257956264,38.5870601408],[100.3068375118,38.7062343576],[103.6547203823,37.427069415],[101.6547203823,37.427069415],[99.8257956264,38.5870601408]"
// 121.8257956264,38.5870601408  结果
void DriverSolarM300::SplitString(const string &s, vector<string> &v, const string &c)
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
void DriverSolarM300::StringtoGps(const string squarename, vector<string> str, map<string, vector<PlotInfo>> &square)
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


int DriverSolarM300::ReadDirFile(string strPath, vector<string> &fileList) //获取文件夹所有文件名字，不区分可见和红外
{
    DIR *dir;
    struct dirent *ptr;
    dir = opendir(strPath.c_str());

    if (dir == NULL)
    {
        LOG(INFO) << "open dir failed..." << endl;
        closedir(dir);
        return 0;
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
    return 1;
}

//Recursively delete all the file in the directory.
int DriverSolarM300::rm_dir_v1(std::string dir_full_path) {
    DIR* dirp = opendir(dir_full_path.c_str());
    if(!dirp)
    {
        return -1;
    }
    struct dirent *dir;
    struct stat st;
    while((dir = readdir(dirp)) != NULL)
    {
        if(strcmp(dir->d_name,".") == 0
           || strcmp(dir->d_name,"..") == 0)
        {
            continue;
        }
        std::string sub_path = dir_full_path + '/' + dir->d_name;
        if(lstat(sub_path.c_str(),&st) == -1)
        {
            //Log("rm_dir:lstat ",sub_path," error");
            continue;
        }
        if(S_ISDIR(st.st_mode))
        {
            if(rm_dir_v1(sub_path) == -1) // 如果是目录文件，递归删除
            {
                closedir(dirp);
                return -1;
            }
            rmdir(sub_path.c_str());
        }
        else if(S_ISREG(st.st_mode))
        {
            unlink(sub_path.c_str());     // 如果是普通文件，则unlink
        }
        else
        {
            //Log("rm_dir:st_mode ",sub_path," error");
            continue;
        }
    }
    if(rmdir(dir_full_path.c_str()) == -1)//delete dir itself.
    {
        closedir(dirp);
        return -1;
    }
    closedir(dirp);
    return 0;
}

//Remove files or dirs
bool DriverSolarM300::Remove(std::string file_name) {
    std::string file_path = file_name;
    struct stat st;
    if (lstat(file_path.c_str(),&st) == -1) {
        return EXIT_FAILURE;
    }
    if (S_ISREG(st.st_mode)) {
        if (unlink(file_path.c_str()) == -1) {
            return EXIT_FAILURE;
        }
    }
    else if(S_ISDIR(st.st_mode)) {
        if(file_name == "." || file_name == "..") {
            return EXIT_FAILURE;
        }
        if(rm_dir_v1(file_path) == -1)//delete all the files in dir.
        {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
