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


//************************日志库*******************************//
#include<spdlog/spdlog.h>
#include<spdlog/sinks/rotating_file_sink.h>
//************************日志库*******************************//

using namespace std;
using namespace cv;

class DetectVisLogic
{
private:
	//参数
	int partNumX = 4, partNumY = 4;
	string visble_seg_solar_model_path = "models/sne_seg_vis_solar.onnx";
	string visble_seg_zc_model_path = "models/sne_seg_vis_zc.onnx";

	string visble_Detect_solar_model_path = "models/sne_obj_vis_solar.onnx";  //组件部分
    string visble_Detect_pic_model_path = "models/sne_obj_vis_all.onnx";  //全图

	string Faulttype[8] = { "遮挡鸟粪","遮挡阴影","遮挡草木","遮挡灰尘","遮挡异物","翻落缺失","碎裂","未知"};  //可以作为配置文件
	
	string Faulttypesolar[4] = { "遮挡鸟粪","遮挡异物","遮挡灰尘","碎裂" }; //组件检测

	string FaulttypePic[3] = { "遮挡阴影","遮挡草木","翻落缺失" };  //全图检测


    // vector<string > vissolarfaluttype = { "zd_nf","zd_hc","zd_yw","sl" };      
	// vector<string > vispicfaluttype = { "fl_qx","fl","back","fengxi","zd_yy","zd_cm"}; //翻落倾斜 翻落  白板  缝隙  遮挡阴影 遮挡草木  

	//////////////////////////////////////////////////////////////////////////////////////////////
	// 说明  小的遮挡 统称为 异物  表现为 小物体在组件上，										//
	// 鸟粪  组件上  较明显   不明显的表现为 小于1/3电池片面积                                  //
	// 灰尘 为表面 拐角区域  堆积 较多，  组件级别检测                                          //
	// 翻落 为重要级 现象 样本不多       图像级别检测                                           //
	// 碎裂为重要级 现象  样本不多       组件级别检测                                           //
	// 阴影                              组件级别阴影和图像级别阴影  组件级别阴影  作为异物处理 //
	// 草木                              图像级别和组件及别          组件级别草木  作为异物处理 //
	//////////////////////////////////////////////////////////////////////////////////////////////


	vector<vector<cv::Mat>> getPartNumImgList(cv::Mat _ZinputImg);//根据设置机型 选择 获取分割后的图像
	vector<vector<cv::Mat>>partitionImage(cv::Mat src);//分割图像
	cv::Mat partImgToWholePic(vector<vector<cv::Mat>>);


public:
	DetectVisLogic();
	~DetectVisLogic();
	//U2net声明，这样就不需要多次声明了,用于生成

	U2Net visbleSolarSegNet;                 //对于更换分割网络 只需要更换类即可  比如 HrNet U2Net thermalSolarSegNet; 
	U2Net visbleZcSegNet;
	

	// //vector<string > visallfaluttype = { "zd_yy","zd_cm" };
	// vector<string > visallfaluttype = { "fl_qx","fl","back","fengxi","zd_yy","zd_cm"};
	//********图像识别类   yolo识别类******************//
	
	yolov5 visbleSolarDetectNet;
	yolov5 visblePicDetectNet;

	Detectiontype SolarResinfo ;//

	vector<Detectiontype> visFaultSolarDetects,visFaultSolarDetectsOutputs,visFaultPicDetectsOutputs;//
	
	vector<visbleFaultJson>solardetectres;  
	
	vector<string > vissolarfaluttype = { "zd_nf","zd_hc","zd_yw","sl" };      
	vector<string > vispicfaluttype = { "fl_qx","fl","back","fengxi","zd_yy","zd_cm"}; //翻落倾斜 翻落  白板  缝隙  遮挡阴影 遮挡草木 

	//********图像识别类   yolo识别类******************//

	cv::Mat drawmask;
	cv::Mat _ZinputImg;
	cv::Mat  _Z_ZcMask, _Z_SolarMask;
	vector<cv::Mat> cropsoalrimg;
	float stdRect_w, stdRect_h , stddelta;  //角度在山地 倾斜摆布时获取的数据不对

	vector<VisbleshowZcPar>visbleZcParas; //用map 比较合适
	map<int, cv::RotatedRect>visbleZcParasmap;
	vector<VisblesolarPar> visbleSolarParas;        //保存组件信息 
	vector<visbleFaultJson> visblefaultinfos;       //保存故障信息

	vector<vector<cv::Point>>visblecontoursolar;  //wzj20230711


	cv::Mat getZ_ZcMask(cv::Mat _ZinputImg);
	cv::Mat getZ_SolarMask(cv::Mat _ZinputImg);  //建议组串模型不需要这么大 是在太大了

	void Mask2ZcInfo(cv::Mat& Mask, float _Zmaskpara);

	void Mask2SolarInfo(cv::Mat& Mask, float _Zmaskpara);

	void getFaultSolar(cv::Mat _ZinputImg,vector<Detectiontype>&detectRes);
	void getFaultPic(cv::Mat _ZinputImg,vector<Detectiontype>&detectRes);

	void classidchange(vector<Detectiontype>&detectRes);  //用于训练的模型和实际对应上

	void writeDrawdetectedImg(cv::Mat _ZinputImg, string  savepath, vector<Detectiontype>&detectRes,vector<string > class_names); 

	void combineDetectinfo();//将故障组件和组件信息保存到  

};