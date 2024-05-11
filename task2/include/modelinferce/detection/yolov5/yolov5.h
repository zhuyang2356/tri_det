#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <cuda_provider_factory.h>   // 提供cuda加速
#include <onnxruntime_cxx_api.h>	 // C或c++的api

// 命名空间
using namespace std;
using namespace cv;
using namespace Ort;

struct Detectiontype
{
    cv::Rect box;
    int classid;
    float confidence;
    // cv::RotatedRect rotaterect;
};


class yolov5
{
private:
    /* data */
    int inpWidth;
	int inpHeight;
    int outHeight;
    int outWidth;

    int nout;
	int num_proposal;  


    float confThreshold;
    float iouThreshold;  //nmsiou
    string modelpath;

    vector<float> input_image_;
    const bool keep_ratio = true;

    vector<char*> input_names;
	vector<char*> output_names;
    vector<vector<int64_t>> input_node_dims; // >=1 outputs
	vector<vector<int64_t>> output_node_dims; // >=1 outputs
    Ort::Session *ort_session = nullptr;    // 初始化Session指针选项

public:
    yolov5(/* args */);
    ~yolov5();

    void ModelInit(string model_path,float inputWidth,float inputHeight,float confThreshold, float iouThreshold);
    void detect(cv::Mat frame ,vector<Detectiontype>& detect_boxes,vector<string> m_classNames);
    void drawresult(cv::Mat img ,vector<Detectiontype>detectRes,vector<string>class_names);

    

    void normalize_(Mat img);//归一化函数
    cv::Mat resize_image(Mat srcimg, int *newh, int *neww, int *top, int *left);




};


