/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-25 18:31:37
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-27 13:57:46
 * @FilePath: /SolarDetect_wzj/lib/include/resultPackage.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include<iostream>
#include<solar_utils.h>
#include<DJI/read_dji_thremal.h>
#include<opencv2/opencv.hpp>
#include<string>
#include<vector>
#include <json/json.h>
#include <exif/exif.h>

// #include<detectthread.h>

using namespace std;
using namespace cv;



// bool Read_Dji_Thermal::getRangeTemperature(RectArea rect , float& temperature , PixTemperature& maxPT , PixTemperature& minPT )

//**************Json*****************//
//  
//struct SolarJson
/*{
	float maxTempture;
    float minTempture;
    float meanTempture;

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

}; */


      
class resPack
{
public:
	resPack();
	~resPack();

	vector<infraFaultJson> infrafaultinfos;
	vector<visbleFaultJson> visblefaultinfos;
	vector<infraFaultJson> infrapackages;
    // vector<infraFaultJson> Spotpackages;
    // vector<infraFaultJson> Solarpackages;
    vector<visbleFaultJson> visblepackages;
    // vector<visbleFaultJson> Picpackages;
    // vector<visbleFaultJson> Solarpackages;
	// 逻辑  先处理红外
	vector<infraFaultJson> infrafaultjsonpackage(vector<infraFaultJson> infrafaultinfos);
	vector<visbleFaultJson> visblefaultjsonpackage(vector<visbleFaultJson> visblefaultinfos);

	// vector<infraFaultJson> infrafaultjsonpackage(vector<infraFaultJson> infrafaultinfos);
	// vector<visbleFaultJson> visblefaultjsonpackage(vector<visbleFaultJson> visblefaultinfos);
	void infrafaultjsonpackageDJIthermal(vector<infraFaultJson> &infrafaultinfos,string Timgpath,bool superres);

	void addinfrafaultpointgps(vector<infraFaultJson> &infrafaultinfos,string Timgpath);
	void addvisblefaultpointgps(vector<visbleFaultJson> &visblefaultinfos,string Zimgpath);

	void VisbleInfraCombine(vector<infraFaultJson> &infrafaultinfos,vector<visbleFaultJson> &visblefaultinfo,bool superres);

	void JsonPackagesComine(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfo,cv::Mat timg,cv::Mat zimg);

	void JsonPackagesComineOld(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfo);

	void JsonPackagesComineYS_split(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfo,cv::Mat timg,cv::Mat zimg);
	void JsonPackagesComineYS_merge(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfo,cv::Mat timg,cv::Mat zimg);
};