#pragma once

#include<iostream>
#include<opencv2/opencv.hpp>
#include<vector>
#include<string>
#include<math.h>

//***********测试读取文件信息***************//
#include <stdio.h>
#include <dirent.h>
#include <sstream>
#include <exif/exif.h>
#include<datatype.h>
//***********测试读取文件信息***************//

// #include<opencv2/highgui/highgui.hpp>
using namespace std;
using namespace cv;

//*********组串参数 start**********
struct InfrashowZcPar 
{
	int id;
	cv::RotatedRect rotate_rect;
};
struct InfrashowZcENinfo
{
	int id;
	cv::RotatedRect rotate_rect;
};

struct VisbleshowZcPar
{
	int id;
	cv::RotatedRect rotate_rect;
};
struct VisbleshowZcENinfo
{
	int id;
	cv::RotatedRect rotate_rect;
};

struct zcpara
{
	int id;
	cv::RotatedRect rotate_rect;
};

//*********组串参数 end**********

//*********组件参数 start **********


struct InfraSolarPar 
{
	int zc_id;
	cv::RotatedRect rotate_rect;
};


struct VisblesolarPar
{
	int zc_id;
	cv::RotatedRect rotate_rect;
};

struct solarpara
{
	int zc_id;
	cv::RotatedRect rotate_rect;
};     //需要修改   代替掉 不需要红外和可见分开


//*********组件参数 end **********

struct infraFaultJson 	
{	
	//string picdir;
	string _Tpicname;
	
	int ZC_id;
	//vector<cv::Point> ZCquads;
	cv::RotatedRect ZCquads;
	//vector<cv::Point> ZCquadsEN;
	string commonfaultid;
	string infra_fault_id; //A类 B类 C类 D类 E类
	//vector<cv::Point> infrasolarquads;
	cv::RotatedRect infrasolarquads;           //故障组件
	cv::Point2d infrasolarcenterEN;
	vector<cv::Point2d>infrasolarpointEN;
	//vector<cv::RotatedRect> infrasolarrect; //都有可能
	cv::Rect infrafaultsolarbox;        //如果用组件识别方式可以标记处热板大小，如果是全图识别故障组件 box较大且统一
	int infrasolarboxarea;
	double soalrmeanTemptrue;  //去除组件温度的数据
	double soalrmaxTemptrue;   //融合的时候统一记录温度数据
	float confidence;   //置信度  到那时有些可能没有 
};

struct visbleFaultJson 
{	
	string _Zpicname;
	int ZC_id;
	cv::RotatedRect ZCquads;

	string commonfaultid;               //是为了表示和红外求一些共有部分
	string visble_fault_id; //A类 B类 C类 D类 E类
	cv::RotatedRect visblesolarquads;      //暂时未用

	cv::Point2d visblesolarcenterEN;
	vector<cv::Point2d> visblesolarpointEN;


	cv::Rect visblefaultsolarbox;        //
	int visblesolarboxarea;
	float confidence;   //置信度  到那时有些可能没有 


};

struct FaultJson 
{	

	int ZC_id;
	cv::RotatedRect ZCquads;
	string fault_id; //A类 B类 C类 D类 E类
	cv::RotatedRect solarquads;
	cv::Rect faultsolarbox;        //如果用组件识别方式可以标记处热板大小，如果是全图识别故障组件 box较大且统一
	int solarboxarea;
	float confidence;   //置信度  到那时有些可能没有 
};

struct SolarJson
{
	float Tempture;
	string PicDir;
	string _T_imgName;
	string _Z_imgName;
	string _Fault_T_imgname;
	string _Fault_Z_ImgName;
	string FaultDir;
	string FaultType;
	string E;
	string N;
	string FaultId;  
	string lu, ru, ld, rd;

};


// struct DJIinfraCamInfo
// {
// 	float f; //焦距
// 	float 

// };
struct camerainfo     // 焦距 飞行速度  高度(相对目标距离) 云台俯仰角 云台偏航角 当前图像中心GPS 
{
	float f; 
	float speed_y;
	float speed_x;
	float height;
	float m_TGimbalPitchDegree;
	float m_TGimbalYawDegree;
	float m_gps_e;
	float m_gps_n;
};



void fillminrect(cv::RotatedRect rect,cv::Mat mask);

bool Isinrotaterect(cv::Point pot, cv::RotatedRect rect);

bool Isinline(cv::Point pot, cv::Point s_pot, cv::Point e_pot);
cv::RotatedRect boxTorotateRect(cv::Rect box);

float getboxiouratio(cv::Rect rect1,cv::Rect rect2);
float getboxiouminarea(cv::Rect rect1,cv::Rect rect2);
float getboxioumaxarea(cv::Rect rect1,cv::Rect rect2);

void rebuildcrossedrect(cv::Rect &crossedrect,cv::Mat img);
void CVapprox(vector<cv::Point> &cons);
Mat getquadsolarimg(vector<cv::Point>cons , cv::Mat img);
Mat extractPerspective(const cv::Mat& image, cv::RotatedRect &ro_rect);//抠出旋转矩形区域
void jibian(Mat img ,float k1,float k2,float p1,float p2);
void SouthAndNorth(Mat m_zpic,double m_ZGimbalYawDegree);
void debugwriteimg(Mat inputMat,string outputname,int quality , bool debug);


// int getGPSInfor(string filepath,double& x,double& y,      //    根据像图像文件 获取GPS的一系列信息 
//                 double& height,double& focal,double& xspeed,
//                 double& yspeed, double& absheight,double& RtkStdLon,
//                 double& GimbalPitchDegree,double& GimbalYawDegree );

double stringToNum(const string &str);
vector<string> splitStr(string str, char delimiter);


int getimginfo(string filepath,DJIdata &djiimginfo);
