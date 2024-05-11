#pragma once

#include<iostream>
#include<opencv2/opencv.hpp>

#include<string>
#include<vector>
#include<map>

#include<solar_utils.h>

//**********************算法工具库******************************//
#include<modelinferce/segmentation/u2net/u2net.h>                  //改写成模块比较合适 就无需  比如就调用segmentation.h 这里有个专门用于识别的工具库
#include<modelinferce/detection/yolov5/yolov5.h>            //改写成模块比较合适 就无需  比如就调用detecttion.h 这里有个专门用于识别的工具库
//**********************算法工具库******************************//

//温度  

//************************日志库*******************************//
#include<spdlog/spdlog.h>
#include<spdlog/sinks/rotating_file_sink.h>
//************************日志库*******************************//

// #define real_shijiao_x 128
// #define real_shijiao_y 128      // old
// #define real_shijiao_w 1024
// #define real_shijiao_h 768

// #define real_shijiao_x 95
// #define real_shijiao_y 90     
// #define real_shijiao_w 1100
// #define real_shijiao_h 834

#define real_shijiao_x 140
#define real_shijiao_y 128                        
#define real_shijiao_w 1000
#define real_shijiao_h 750                       //尺寸 128 750 146
using namespace std;                    
using namespace cv;


class DetectInfraLogic
{
private:
    /* data */
    string thermal_Seg_solar_model_path = "models/sne_seg_infra_solar.onnx"; //或者配置文件
    string thermal_Seg_zc_model_path = "models/sne_seg_infra_zc.onnx";

	string thermal_Detect_spot_model_path = "models/sne_obj_infra_spot.onnx";
	string thermal_Detect_solar_model_path = "models/sne_obj_infra_solar.onnx";  //二极管 聚集性热斑 组件短路
	string thermal_Seg_dc_model_path = "models/sne_seg_dc.onnx";


    cv::Mat _T_ZcMask, _T_SolarMask;  // 内部调用
	string Faulttype[4] = { "热斑","二极管","聚集性热斑","组件短路" };  //可以作为配置文件

	// string Faulttypesolar[3] = { "热斑","聚集性热斑","二极管" };//组件级别  "多次讨论得到"

	// string FaulttypePic[2] = { "组件短路","掉串" };//图像级别 可以是配置文件


public:
    DetectInfraLogic(/* args */);
    ~DetectInfraLogic();

    //********图像分割类   U2Net类******************//
	//图像分割 办法 //   后续如果用别的图像分割方法
	U2Net thermalSolarSegNet;            //对于更换分割网络 只需要更换类即可  比如 HrNet U2Net thermalSolarSegNet;          
	U2Net thermalZcSegNet; 
	U2Net thermalDcSegNet;           
	
	//********图像分割类   U2Net类******************//


	//********图像识别类   yolo识别类******************//
	yolov5 thermalDetectNetspot;
	yolov5 thermalDetectNetsolar;            
	vector<Detectiontype> FaultSolarDeetctsOutputs,FaultSpotDeetctsOutputs,FaultDCtDeetctsOutputs;
	
	vector<string > faultTypespot = {"Spot"};
	vector<string > faultTypesolar = {"ejggz","jjxrb","zjdl"};

	//********图像识别类   yolo识别类******************//


    cv::Mat T_Solarmask,T_Zcmask;   //外界调用
	float stdRect_w = 0.0, stdRect_h = 0.0, stddelta=0;  //这个参数是为了对 没有分割得组件用 box去拟合他的位置
	

    vector<InfrashowZcPar>infraZcParas; //用map 比较合适
    map<int, cv::RotatedRect>infraZcParasmap;     //保存组串信息

	vector<InfraSolarPar> infraSolarParas;        //保存组件信息
	vector<infraFaultJson> infrafaultinfos;       //保存故障信息

	cv::Rect shijiaorect = cv::Rect(real_shijiao_x, real_shijiao_y, real_shijiao_w, real_shijiao_h);

	cv::Mat getT_ZcMask(cv::Mat _TinputImg);
	// cv::Mat getT_SolarMask(cv::Mat  _TinputImg);
	cv::Mat getT_SolarMask(cv::Mat  Tsolarmask);

	cv::Mat jibian(cv::Mat images, float k1, float k2, float p1, float p2);// 在思考是读取参数使用还是
	
	void inferThermalImage(cv::Mat _Tinputmask);//后续改进 //需要继续改进

    void Mask2ZcInfo(cv::Mat& Mask, float _Tmaskpara);
	void Mask2ZcInfox(cv::Mat& Mask, float _Tmaskpara,string _maskname);
	void Mask2SolarInfo(cv::Mat& Mask, float _Tmaskpara);
	void Mask2SolarInfox(cv::Mat& Mask, float _Tmaskpara,string _maskname);

	void getFaultSpot(cv::Mat _TinputImg,vector<Detectiontype>&detectRes); //整图识别
	void getFaultSolar(cv::Mat _TinputImg,vector<Detectiontype>&detectRes); //整图识别
	void getDCinfo(cv::Mat & _Tinputimg,vector<Detectiontype>&detectRes,string namesave); //新增在20230608

	void writeDrawdetectedImg(cv::Mat _TinputImg, string  savepath, vector<Detectiontype>&detectRes,vector<string > class_names); 
	
	void combineDetectinfoback();//将故障组件和组件信息保存到  // 也需要进行 截取shijiaorect

	void combineDetectinfo();//将故障组件和组件信息保存到  // 也需要进行 截取shijiaorect

	void get_shijiao_img(cv::Mat& _Tinputimg);

	// void sortfaultdata(vector<infraFaultJson> infrafaultinfos);// 在solar_utils中处理

	void debug();//  //   测试组串标注是否正确
            		//   测试id
            		//   测试组件分割

};


