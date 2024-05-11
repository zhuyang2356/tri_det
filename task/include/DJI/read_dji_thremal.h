#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <string.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <unistd.h>
#include <fcntl.h>
#include <opencv2/opencv.hpp>
#include "dirp_api.h"

using namespace std;

#define FSTREAM_OPEN_CHECK(fs, name, go) \
            { \
                if(!fs.is_open()) \
                { \
                    cout << "ERROR: open " << name << " file failed!" << endl; \
                    ret = -1; \
                    goto go; \
                } \
            }

typedef enum
{
    dirp_action_type_extract = 0,
    dirp_action_type_measure,
    dirp_action_type_process,
    dirp_action_type_num,
} dirp_action_type_e;


typedef enum
{
    dirp_measure_format_int16 = 0,
    dirp_measure_format_float32,
    dirp_measure_format_num,
} dirp_measure_format_e;

struct RectArea
{
	int x;
	int y;
	int w;
	int h;
};

struct PixPoint
{
	int x;
	int y;
};

struct PixTemperature
{
	PixPoint point;
	float temperature;
};

class Read_Dji_Thermal
{
public:
	Read_Dji_Thermal(string rjpeg_file_path);
	~Read_Dji_Thermal();
private:
//	int32_t prv_get_rjpeg_output_size(const dirp_action_type_e action_type, const dirp_resolution_t *resolution);
//	int32_t prv_action_run(DIRP_HANDLE dirp_handle);

	//将像素点的坐标转换为内存索引
	long pointToIndex(int x, int y);
public:
	bool getDjiRawData();
	//通过像素的坐标xy获取对应的温度，temperature为10倍温度的整数（温度取1位有效位）
	bool getPointTemperature(int x , int y , long& temperature ); //tenfold
	//计算指定范围内像素点温度的平均值、最高值、最高值坐标、最低值、最低值坐标
	bool getRangeTemperature(RectArea rect , float& temperature , PixTemperature& maxPT , PixTemperature& minPT );
	//计算指定像素点的平均值、最高值、最高值坐标、最低值、最低值坐标
	// bool getVectorTemperature(vector<Point> vec_points, float& temperature , PixTemperature& maxPT , PixTemperature& minPT);
private:
	//原始图片路径
	string rjpeg_file_path_;
	//图像宽和高
	int image_w_;
	int image_h_;
	//温度数据raw
	string raw_data_;


};


