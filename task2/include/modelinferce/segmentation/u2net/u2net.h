/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-20 17:25:09
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-20 17:25:09
 * @FilePath: /SolarDetect_wzj/lib/include/modelinferce/segmentation/u2net/u2net.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>

using namespace cv;
using namespace std;
using namespace Ort;

class U2Net
{
public:
	U2Net();
	~U2Net();
	void ModelInit(Env& env, SessionOptions& sessionOptions, string model_path);
	void ModelInit(string model_path);
	Mat detect(Mat& cv_image);
	Mat detectM30T(Mat& M30T_image);
private:
	vector<float> input_image_;
	int inpWidth;
	int inpHeight;
	int outWidth;
	int outHeight;
	const float mean[3] = { 0.485, 0.456, 0.406 };
	const float stds[3] = { 0.229, 0.224, 0.225 };

	
	Env env = Env(ORT_LOGGING_LEVEL_ERROR, "u2net");   //需要存在
	
	Ort::Session* ort_session = nullptr;
	vector<char*> input_names;
	vector<char*> output_names;
	vector<vector<int64_t>> input_node_dims; // >=1 outputs
	vector<vector<int64_t>> output_node_dims; // >=1 outputs
};
