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

	//�����ص������ת��Ϊ�ڴ�����
	long pointToIndex(int x, int y);
public:
	bool getDjiRawData();
	//ͨ�����ص�����xy��ȡ��Ӧ���¶ȣ�temperatureΪ10���¶ȵ��������¶�ȡ1λ��Чλ��
	bool getPointTemperature(int x , int y , long& temperature ); //tenfold
	//����ָ����Χ�����ص��¶ȵ�ƽ��ֵ�����ֵ�����ֵ���ꡢ���ֵ�����ֵ����
	bool getRangeTemperature(RectArea rect , float& temperature , PixTemperature& maxPT , PixTemperature& minPT );
	//����ָ�����ص��ƽ��ֵ�����ֵ�����ֵ���ꡢ���ֵ�����ֵ����
	// bool getVectorTemperature(vector<Point> vec_points, float& temperature , PixTemperature& maxPT , PixTemperature& minPT);
private:
	//ԭʼͼƬ·��
	string rjpeg_file_path_;
	//ͼ���͸�
	int image_w_;
	int image_h_;
	//�¶�����raw
	string raw_data_;


};


