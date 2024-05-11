#include"yolov5.h"



yolov5::yolov5(/* args */)
{

    cout<<"欢迎使用yolov5进行目标检测"<<endl;
}

yolov5::~yolov5()
{
    cout<<"使用yolov5进行目标检测结束"<<endl;

}

void  yolov5::ModelInit(string model_path,float inputWidth,float inputHeight,float confThreshold, float iouThreshold)
{   
    //*************参数*********** //
    this->inpHeight = inputHeight;      //onn是可以不需要输入参数的
    this->inpWidth = inputWidth;
    this->confThreshold = confThreshold;
    this->iouThreshold = iouThreshold;
    this->modelpath = model_path;



    Env env = Env(ORT_LOGGING_LEVEL_ERROR, "yolov5-6.1");
	

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
	ort_session = new Session(env, model_path.c_str(), sessionOptions);    // 初始化Session指针选项
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
	// this->outHeight = output_node_dims[0][2];
	// this->outWidth = output_node_dims[0][3];
  

	this->inpHeight = input_node_dims[0][2];
	this->inpWidth = input_node_dims[0][3];
	this->nout = output_node_dims[0][2];      // 5+classes
	this->num_proposal = output_node_dims[0][1];  // pre_box       320  对于 6300  640 对应25200  可以用netron查看


}

Mat yolov5::resize_image(Mat srcimg, int *newh, int *neww, int *top, int *left)
{
	int srch = srcimg.rows, srcw = srcimg.cols;
	*newh = this->inpHeight;
	*neww = this->inpWidth;
	Mat dstimg;
	if (this->keep_ratio && srch != srcw) {
		float hw_scale = (float)srch / srcw;
		if (hw_scale > 1) {
			*newh = this->inpHeight;
			*neww = int(this->inpWidth / hw_scale);
			resize(srcimg, dstimg, Size(*neww, *newh), INTER_AREA);
			*left = int((this->inpWidth - *neww) * 0.5);
			copyMakeBorder(dstimg, dstimg, 0, 0, *left, this->inpWidth - *neww - *left, BORDER_CONSTANT, 114);
		}
		else {
			*newh = (int)this->inpHeight * hw_scale;
			*neww = this->inpWidth;
			resize(srcimg, dstimg, Size(*neww, *newh), INTER_AREA);
			*top = (int)(this->inpHeight - *newh) * 0.5;
			copyMakeBorder(dstimg, dstimg, *top, this->inpHeight - *newh - *top, 0, 0, BORDER_CONSTANT, 114);
		}
	}
	else {
		resize(srcimg, dstimg, Size(*neww, *newh), INTER_AREA);
	}
	return dstimg;
}

void yolov5::normalize_(Mat img)
{
	//    img.convertTo(img, CV_32F);
	int row = img.rows;
	int col = img.cols;
	this->input_image_.resize(row * col * img.channels());  // vector大小
	for (int c = 0; c < 3; c++)  // bgr
	{
		for (int i = 0; i < row; i++)  // 行
		{
			for (int j = 0; j < col; j++)  // 列
			{
				float pix = img.ptr<uchar>(i)[j * 3 + 2 - c];  // Mat里的ptr函数访问任意一行像素的首地址,2-c:表示rgb
				this->input_image_[c * row * col + i * col + j] = pix / 255.0;

			}
		}
	}
}


/*
void yolov5::nmsBOX(vector<Detectiontype>& input_boxes)
{
	sort(input_boxes.begin(), input_boxes.end(), [](Detectiontype a, Detectiontype b) { return a.confidence > b.confidence; }); // 降序排列
	vector<float> vArea(input_boxes.size());
	for (int i = 0; i < input_boxes.size(); ++i)
	{
		// vArea[i] = (input_boxes[i].x2 - input_boxes[i].x1 + 1)
		// 	* (input_boxes[i].y2 - input_boxes[i].y1 + 1);

        vArea[i] = input_boxes[i].box.width* input_boxes[i].box.height;

	}
	// 全初始化为false，用来作为记录是否保留相应索引下pre_box的标志vector
	vector<bool> isSuppressed(input_boxes.size(), false);  
	for (int i = 0; i < input_boxes.size(); ++i)
	{
		if (isSuppressed[i]) { continue; }
		for (int j = i + 1; j < input_boxes.size(); ++j)
		{
			if (isSuppressed[j]) { continue; }
			float xx1 = max(input_boxes[i].x1, input_boxes[j].x1);
			float yy1 = max(input_boxes[i].y1, input_boxes[j].y1);
			float xx2 = min(input_boxes[i].x2, input_boxes[j].x2);
			float yy2 = min(input_boxes[i].y2, input_boxes[j].y2);

			float w = max(0.0f, xx2 - xx1 + 1);
			float h = max(0.0f, yy2 - yy1 + 1);
			float inter = w * h;	// 交集
			if(input_boxes[i].label == input_boxes[j].label)
			{
				float ovr = inter / (vArea[i] + vArea[j] - inter);  // 计算iou
				if (ovr >= this->nmsThreshold)
				{
					isSuppressed[j] = true;
				}
			}	
		}
	}
	// return post_nms;
	int idx_t = 0;
       // remove_if()函数 remove_if(beg, end, op) //移除区间[beg,end)中每一个“令判断式:op(elem)获得true”的元素
	input_boxes.erase(remove_if(input_boxes.begin(), input_boxes.end(), [&idx_t, &isSuppressed](const BoxInfo& f) { return isSuppressed[idx_t++]; }), input_boxes.end());
	// 另一种写法
	// sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; }); // 降序排列
	// vector<bool> remove_flags(input_boxes.size(),false);
	// auto iou = [](const BoxInfo& box1,const BoxInfo& box2)
	// {
	// 	float xx1 = max(box1.x1, box2.x1);
	// 	float yy1 = max(box1.y1, box2.y1);
	// 	float xx2 = min(box1.x2, box2.x2);
	// 	float yy2 = min(box1.y2, box2.y2);
	// 	// 交集
	// 	float w = max(0.0f, xx2 - xx1 + 1);
	// 	float h = max(0.0f, yy2 - yy1 + 1);
	// 	float inter_area = w * h;
	// 	// 并集
	// 	float union_area = max(0.0f,box1.x2-box1.x1) * max(0.0f,box1.y2-box1.y1)
	// 					   + max(0.0f,box2.x2-box2.x1) * max(0.0f,box2.y2-box2.y1) - inter_area;
	// 	return inter_area / union_area;
	// };
	// for (int i = 0; i < input_boxes.size(); ++i)
	// {
	// 	if(remove_flags[i]) continue;
	// 	for (int j = i + 1; j < input_boxes.size(); ++j)
	// 	{
	// 		if(remove_flags[j]) continue;
	// 		if(input_boxes[i].label == input_boxes[j].label && iou(input_boxes[i],input_boxes[j])>=this->nmsThreshold)
	// 		{
	// 			remove_flags[j] = true;
	// 		}
	// 	}
	// }
	// int idx_t = 0;
    // // remove_if()函数 remove_if(beg, end, op) //移除区间[beg,end)中每一个“令判断式:op(elem)获得true”的元素
	// input_boxes.erase(remove_if(input_boxes.begin(), input_boxes.end(), [&idx_t, &remove_flags](const BoxInfo& f) { return remove_flags[idx_t++]; }), input_boxes.end());
}*/





void yolov5::detect(cv::Mat frame,vector<Detectiontype>& detect_boxes,vector<string> m_classNames)
{

    int num_classes = m_classNames.size();

	int newh = 0, neww = 0, padh = 0, padw = 0;
	Mat dstimg = this->resize_image(frame, &newh, &neww, &padh, &padw);
	// cout<<"新版可见识别yolo图像尺寸"<<dstimg.cols<<" "<<dstimg.rows<<endl;
	this->normalize_(dstimg);
	// 定义一个输入矩阵，int64_t是下面作为输入参数时的类型
	array<int64_t, 4> input_shape_{ 1, 3, this->inpHeight, this->inpWidth };

    //创建输入tensor
	auto allocator_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	Value input_tensor_ = Value::CreateTensor<float>(allocator_info, input_image_.data(), input_image_.size(), input_shape_.data(), input_shape_.size());

	// 开始推理
	vector<Value> ort_outputs = ort_session->Run(RunOptions{ nullptr }, &input_names[0], &input_tensor_, 1, output_names.data(), output_names.size());   // 开始推理
	/////generate proposals
	

    

	float ratioh = (float)frame.rows / newh, ratiow = (float)frame.cols / neww;
	float* pdata = ort_outputs[0].GetTensorMutableData<float>(); // GetTensorMutableData

    vector<int> class_ids;
    vector<float> confidences;
    vector<cv::Rect> boxes;

	for(int i = 0; i < num_proposal; ++i) // 遍历所有的num_pre_boxes
	{
		int index = i * nout;      // prob[b*num_pred_boxes*(classes+5)]  
		float obj_conf = pdata[index + 4];  // 置信度分数
		if (obj_conf > this->confThreshold)  // 大于阈值
		{
			int class_idx = 0;
			float max_class_socre = 0;
			for (int k = 0; k < num_classes; ++k)
			{
				if (pdata[k + index + 5] > max_class_socre)
				{
					max_class_socre = pdata[k + index + 5];
					class_idx = k;
				}
			}
			max_class_socre *= obj_conf;   // 最大的类别分数*置信度
			if (max_class_socre > this->confThreshold) // 再次筛选          //这里的box宽高需要考虑研究是否正确
			{ 
				//const int class_idx = classIdPoint.x;
				float cx = pdata[index];  //x  中心
				float cy = pdata[index+1];  //y
				float w = pdata[index+2];  //w
				float h = pdata[index+3];  //h

                float xmin = (cx - padw - 0.5 * w)*ratiow;
				float ymin = (cy - padh - 0.5 * h)*ratioh;
				// float xmax = (cx - padw + 0.5 * w)*ratiow;
				// float ymax = (cy - padh + 0.5 * h)*ratioh;

                boxes.emplace_back(cv::Rect(xmin, ymin, w*ratiow, h*ratioh));
				confidences.emplace_back(max_class_socre);
                class_ids.emplace_back(class_idx);
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences

    

    vector<int> nms_result;
	
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, iouThreshold, nms_result);
    // vector<Detectiontype> detect_boxes;

    for (int i = 0; i < nms_result.size(); i++)
    {
        int idx = nms_result[i];
        Detectiontype result;
        result.classid = class_ids[idx];
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        detect_boxes.emplace_back(result);
    }

    
    
}

 void yolov5::drawresult(cv::Mat img ,vector<Detectiontype>detectRes,vector<string>class_names)
 {
	for (size_t i = 0; i < detectRes.size(); ++i)
	{
		int xmin = int(detectRes[i].box.x);
		int ymin = int(detectRes[i].box.y);

		int xmax = int(detectRes[i].box.x+detectRes[i].box.width);
		int ymax = int(detectRes[i].box.y+detectRes[i].box.height);
		rectangle(img, Point(xmin, ymin), Point(xmax, ymax), Scalar(0,255, 0), 2);
		string label = format("%.2f", detectRes[i].confidence);
		// label =to_string(detectRes[i].classid) + ":" + label;
		label = class_names[detectRes[i].classid]+":"+label;
		putText(img, label, Point(xmin, ymin - 5), FONT_HERSHEY_SIMPLEX, img.cols/2000, Scalar(0, 255, 0), img.cols/800 );
	}
	// cout<<"新版红外识别："<<detectRes.size()<<endl;
	// cv::imwrite("aa00.jpg",img);
 }