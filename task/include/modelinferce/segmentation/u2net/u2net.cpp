#include "u2net.h"

U2Net::U2Net(){
	cout << "U2Net 创建." << endl;
}
U2Net::~U2Net(){

	cout << "U2Net 分割结束" << endl;
}
void U2Net::ModelInit(Env& env, SessionOptions &sessionOptions, string model_path)
{
	//string model_path = "u2net.onnx";
	//std::wstring widestr = std::wstring(model_path.begin(), model_path.end());
	//OrtStatus* status = OrtSessionOptionsAppendExecutionProvider_CUDA(sessionOptions, 0);
	//使用gpu分割
	auto availableProviders = Ort::GetAvailableProviders();
	auto cudaAvailable = std::find(availableProviders.begin(), availableProviders.end(), "CUDAExecutionProvider");
	OrtCUDAProviderOptions cudaOption;
	if (cudaAvailable != availableProviders.end())
	{
		std::cout << "U2net Inference device: GPU" << std::endl;
		sessionOptions.AppendExecutionProvider_CUDA(cudaOption);
	}

	sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);
	ort_session = new Session(env, model_path.c_str(), sessionOptions);
	size_t numInputNodes = ort_session->GetInputCount();
	size_t numOutputNodes = ort_session->GetOutputCount();
	AllocatorWithDefaultOptions allocator;

	for (int i = 0; i < numInputNodes; i++)
	{
		input_names.push_back(ort_session->GetInputName(i, allocator));
		Ort::TypeInfo input_type_info = ort_session->GetInputTypeInfo(i);
		auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
		auto input_dims = input_tensor_info.GetShape();
		input_node_dims.push_back(input_dims);
	}

	for (int i = 0; i < numOutputNodes; i++)
	{
		output_names.push_back(ort_session->GetOutputName(i, allocator));
		Ort::TypeInfo output_type_info = ort_session->GetOutputTypeInfo(i);
		auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
		auto output_dims = output_tensor_info.GetShape();
		output_node_dims.push_back(output_dims);
	}

	this->inpHeight = input_node_dims[0][2];
	this->inpWidth = input_node_dims[0][3];
	this->outHeight = output_node_dims[0][2];
	this->outWidth = output_node_dims[0][3];
	//return ;
}
 void U2Net::ModelInit(string model_path)
 {
	Env env = Env(ORT_LOGGING_LEVEL_ERROR, "u2net");
	SessionOptions sessionOptions = SessionOptions();
	auto availableProviders = Ort::GetAvailableProviders();
	auto cudaAvailable = std::find(availableProviders.begin(), availableProviders.end(), "CUDAExecutionProvider");
	OrtCUDAProviderOptions cudaOption;
	if (cudaAvailable != availableProviders.end())
	{
		std::cout << "U2net Inference device: GPU" << std::endl;
		sessionOptions.AppendExecutionProvider_CUDA(cudaOption);
	}

	sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);
	ort_session = new Session(env, model_path.c_str(), sessionOptions);
	size_t numInputNodes = ort_session->GetInputCount();
	size_t numOutputNodes = ort_session->GetOutputCount();
	AllocatorWithDefaultOptions allocator;

	for (int i = 0; i < numInputNodes; i++)
	{
		input_names.push_back(ort_session->GetInputName(i, allocator));
		Ort::TypeInfo input_type_info = ort_session->GetInputTypeInfo(i);
		auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
		auto input_dims = input_tensor_info.GetShape();
		input_node_dims.push_back(input_dims);
	}

	for (int i = 0; i < numOutputNodes; i++)
	{
		output_names.push_back(ort_session->GetOutputName(i, allocator));
		Ort::TypeInfo output_type_info = ort_session->GetOutputTypeInfo(i);
		auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
		auto output_dims = output_tensor_info.GetShape();
		output_node_dims.push_back(output_dims);
	}

	this->inpHeight = input_node_dims[0][2];
	this->inpWidth = input_node_dims[0][3];
	this->outHeight = output_node_dims[0][2];
	this->outWidth = output_node_dims[0][3];
 }


Mat U2Net::detect(Mat& srcimg)
{	
	
	Mat dstimg;
	// cvtColor(srcimg,x,cv::COLOR_BGR2RGB);
	resize(srcimg, dstimg, Size(this->inpWidth, this->inpHeight));
	// cvtColor(dstimg,dstimg,cv::COLOR_BGR2RGB);
	// cout<<"红外变形:"<<Size(this->inpWidth, this->inpHeight)<<endl; 
	this->input_image_.resize(this->inpWidth * this->inpHeight * dstimg.channels());

	for (int c = 0; c < 3; c++)
	{
		for (int i = 0; i < this->inpHeight; i++)
		{
			for (int j = 0; j < this->inpWidth; j++)
			{
				float pix = dstimg.ptr<uchar>(i)[j * 3 + 2 - c];
				this->input_image_[c * this->inpHeight * this->inpWidth + i * this->inpWidth + j] = (pix / 255.0 - mean[c]) / stds[c];
			}
		}
	}

	array<int64_t, 4> input_shape_{ 1, 3, this->inpHeight, this->inpWidth };

	auto allocator_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	Value input_tensor_ = Value::CreateTensor<float>(allocator_info, input_image_.data(), input_image_.size(), input_shape_.data(), input_shape_.size());
	vector<Value> ort_outputs = ort_session->Run(RunOptions{ nullptr }, &input_names[0], &input_tensor_, 1, output_names.data(), output_names.size());   // ��ʼ����
	float* pred = ort_outputs[0].GetTensorMutableData<float>();
	Mat result(outHeight, outWidth, CV_32FC1, pred);
	double min_value, max_value;
	minMaxLoc(result, &min_value, &max_value, 0, 0);
	result = (result - min_value) / (max_value - min_value);
	result *= 255;
	result.convertTo(result, CV_8UC1);
	
	
	return result; //输出是320*320
}

Mat U2Net::detectM30T(Mat& M30T_image)
{	
	// cout<<"在运行U2Net红外模型识别"<<endl;
	vector<Mat> quarterImagesup,quarterImagesdown, halfpart;
	Mat resultu,resultd,result;
	Mat quarterimagelu,quarterimageru,quarterimageld,quarterimagerd;
	Mat quarterimagelumask,quarterimagerumask,quarterimageldmask,quarterimagerdmask;

	quarterimagelu=M30T_image(Rect(0,0,640,512));
	quarterimageru=M30T_image(Rect(640,0,640,512));
	quarterimageld=M30T_image(Rect(0,512,640,512));
	quarterimagerd=M30T_image(Rect(640,512,640,512));
	
	quarterimagelumask=detect(quarterimagelu);
	cv::resize(quarterimagelumask, quarterimagelumask, cv::Size(640, 512));

	quarterimagerumask=detect(quarterimageru);
	cv::resize(quarterimagerumask, quarterimagerumask, cv::Size(640, 512));

	quarterimageldmask=detect(quarterimageld);
	cv::resize(quarterimageldmask, quarterimageldmask, cv::Size(640, 512));

	quarterimagerdmask=detect(quarterimagerd);
	cv::resize(quarterimagerdmask, quarterimagerdmask, cv::Size(640, 512));

	quarterImagesup.push_back(quarterimagelumask);
	quarterImagesup.push_back(quarterimagerumask);

	quarterImagesdown.push_back(quarterimageldmask);
	quarterImagesdown.push_back(quarterimagerdmask);
	cv::hconcat(quarterImagesup,resultu);
	cv::hconcat(quarterImagesdown,resultd);
	halfpart.push_back(resultu);
	halfpart.push_back(resultd);
	cv::vconcat(halfpart,result);
	cout<<"重合后mask大小"<<result.cols<<" "<<result.rows<<endl;
	cout<<"U2Net模型运行完成"<<endl;
	return result;


}
