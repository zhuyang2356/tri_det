#include"yoloxDetect.h"

yolox::yolox(string modelpath, float confThreshold, float nmsThreshold)
{
	this->prob_threshold = confThreshold;
	this->nms_threshold = nmsThreshold;

//	ifstream ifs(this->classesFile.c_str());
//	string line;
//	while (getline(ifs, line)) this->classes.push_back(line);
	this->classes.push_back("niaofen");
	this->classes.push_back("zhedang");
	this->num_class = this->classes.size();
	this->net = readNet(modelpath);

	
	this->net.setPreferableBackend(dnn::DNN_BACKEND_CUDA);
	this->net.setPreferableTarget(dnn::DNN_TARGET_CUDA_FP16);

	cout<<"可见识别使用GPU    ***********************yolox gpu *************"<<endl;
	// net.setPreferableBackend();


}

Mat yolox::resize_image(Mat srcimg, float* scale)
{
	float r = std::min(this->input_shape[1] / (srcimg.cols * 1.0), this->input_shape[0] / (srcimg.rows * 1.0));
	*scale = r;
	// r = std::min(r, 1.0f);
	int unpad_w = r * srcimg.cols;
	int unpad_h = r * srcimg.rows;
	Mat re(unpad_h, unpad_w, CV_8UC3);
	resize(srcimg, re, re.size());
	Mat out(this->input_shape[1], this->input_shape[0], CV_8UC3, Scalar(114, 114, 114));
	re.copyTo(out(Rect(0, 0, re.cols, re.rows)));
	return out;
}

void yolox::normalize(Mat& img)
{
	cvtColor(img, img, cv::COLOR_BGR2RGB);                //20221122--wzj -- https://github.com/hpc203/yolox-opencv-dnn 根据提示 在最新版 是不需要 进行 BGR和RGB的转换 和减去均值和方差几步
	img.convertTo(img, CV_32F);																			
	int i = 0, j = 0;
	for (i = 0; i < img.rows; i++)
	{
		float* pdata = (float*)(img.data + i * img.step);
		for (j = 0; j < img.cols; j++)
		{
			// pdata[0] = (pdata[0] / 255.0 - this->mean[0]) / this->std[0];
			// pdata[1] = (pdata[1] / 255.0 - this->mean[1]) / this->std[1];
			// pdata[2] = (pdata[2] / 255.0 - this->mean[2]) / this->std[2];
			pdata[0] = pdata[0];
			pdata[1] = pdata[1];
			pdata[2] = pdata[2];
			pdata += 3;
		}
	}
}
// void yolox::normalize(Mat& img)
// {
// 	// cvtColor(img, img, cv::COLOR_BGR2RGB);                //20221122--wzj -- https://github.com/hpc203/yolox-opencv-dnn 根据提示 在最新版 是不需要 进行 BGR和RGB的转换 和减去均值和方差几步
// 	img.convertTo(img, CV_32F);																			
// 	int i = 0, j = 0;
// 	for (i = 0; i < img.rows; i++)
// 	{
// 		float* pdata = (float*)(img.data + i * img.step);
// 		// for (j = 0; j < img.cols; j++)
// 		// {
// 		// 	pdata[0] = (pdata[0] / 255.0 - this->mean[0]) / this->std[0];
// 		// 	pdata[1] = (pdata[1] / 255.0 - this->mean[1]) / this->std[1];
// 		// 	pdata[2] = (pdata[2] / 255.0 - this->mean[2]) / this->std[2];
// 		// 	pdata += 3;
// 		// }
// 	}
// }

int yolox::get_max_class(float* scores)
{
	float max_class_socre = 0, class_socre = 0;
	int max_class_id = 0, c = 0;
	for (c = 0; c < this->num_class; c++) //// get max socre
	{
		if (scores[c] > max_class_socre)
		{
			max_class_socre = scores[c];
			max_class_id = c;
		}
	}
	return max_class_id;
}

int yolox::detect(Mat& srcimg)
{
	float scale = 1.0;
	Mat dstimg = this->resize_image(srcimg, &scale);
	//this->normalize(dstimg);
	Mat blob = blobFromImage(dstimg);

	this->net.setInput(blob);
	vector<Mat> outs;
	this->net.forward(outs, this->net.getUnconnectedOutLayersNames());
	if (outs[0].dims == 3)
	{
		const int num_proposal = outs[0].size[1];
		outs[0] = outs[0].reshape(0, num_proposal);
	}
	/////generate proposals, decode outputs
	vector<int> classIds;
	vector<float> confidences;
	vector<Rect> boxes;
	float ratioh = (float)srcimg.rows / this->input_shape[0], ratiow = (float)srcimg.cols / this->input_shape[1];
	int n = 0, i = 0, j = 0, nout = this->classes.size() + 5, row_ind = 0;
	float* pdata = (float*)outs[0].data;
	for (n = 0; n < 3; n++)   ///�߶�
	{
		const int num_grid_x = (int)(this->input_shape[1] / this->stride[n]);
		const int num_grid_y = (int)(this->input_shape[0] / this->stride[n]);
		for (i = 0; i < num_grid_y; i++)
		{
			for (j = 0; j < num_grid_x; j++)
			{
				float box_score = pdata[4];
				/*for (int class_idx = 0; class_idx < this->num_class; class_idx++)
				{
					float cls_score = pdata[5 + class_idx];
					float box_prob = box_score * cls_score;
					if (box_prob > this->prob_threshold)
					{
						float x_center = (pdata[0] + j) * this->stride[n];
						float y_center = (pdata[1] + i) * this->stride[n];
						float w = exp(pdata[2]) * this->stride[n];
						float h = exp(pdata[3]) * this->stride[n];
						float x0 = x_center - w * 0.5f;
						float y0 = y_center - h * 0.5f;

						classIds.push_back(class_idx);
						confidences.push_back(box_prob);
						boxes.push_back(Rect(int(x0), int(y0), (int)(w), (int)(h)));
					}
				}*/

				//int class_idx = this->get_max_class(pdata + 5);
				Mat scores = outs[0].row(row_ind).colRange(5, outs[0].cols);
				Point classIdPoint;
				double max_class_socre;
				// Get the value and location of the maximum score
				minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
				int class_idx = classIdPoint.x;

				float cls_score = pdata[5 + class_idx];
				float box_prob = box_score * cls_score;
				if (box_prob > this->prob_threshold)
				{
					float x_center = (pdata[0] + j) * this->stride[n];
					float y_center = (pdata[1] + i) * this->stride[n];
					float w = exp(pdata[2]) * this->stride[n];
					float h = exp(pdata[3]) * this->stride[n];
					float x0 = x_center - w * 0.5f;
					float y0 = y_center - h * 0.5f;

					classIds.push_back(class_idx);
					confidences.push_back(box_prob);
					boxes.push_back(Rect(int(x0), int(y0), (int)(w), (int)(h)));
				}

				pdata += nout;
				row_ind++;
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	vector<int> indices;
	NMSBoxes(boxes, confidences, this->prob_threshold, this->nms_threshold, indices);
	int maxClasses = -1;
	// float maxConfidences = 0;
	float maxConfidences = this->prob_threshold;
	
	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		Rect box = boxes[idx];
		//�������
		if(classIds[idx] == 0)
		{	
			// cout<<"niaofen confindence："<<confidences[idx]<<endl;
			maxClasses = classIds[idx];
			break;
		}

		if(confidences[idx] >= maxConfidences)
		{
			maxConfidences = confidences[idx];
			maxClasses = classIds[idx];
		}

		// adjust offset to original unpadded
		float x0 = (box.x) / scale;
		float y0 = (box.y) / scale;
		float x1 = (box.x + box.width) / scale;
		float y1 = (box.y + box.height) / scale;
//
//		// clip
		x0 = std::max(std::min(x0, (float)(srcimg.cols - 1)), 0.f);
		y0 = std::max(std::min(y0, (float)(srcimg.rows - 1)), 0.f);
		x1 = std::max(std::min(x1, (float)(srcimg.cols - 1)), 0.f);
		y1 = std::max(std::min(y1, (float)(srcimg.rows - 1)), 0.f);

		yoloxbox =cv::Rect(x0,y0,x1-x0,y1-y0);
//
//		rectangle(srcimg, Point(x0, y0), Point(x1, y1), Scalar(0, 0, 255), 2);
//		//Get the label for the class name and its confidence
//		string label = format("%.2f", confidences[idx]);
//		label = this->classes[classIds[idx]] + ":" + label;
//
//		std::cout << "the category for the detected objects " << idx << " is: " << classes[classIds[idx]] << std::endl;
//		std::cout << "the probability for the detected objects " << idx << " is: " << confidences[idx] << std::endl;
//
//		//Display the label at the top of the bounding box
//		int baseLine;
//		Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
//		y0 = std::max(y0, (float)labelSize.height);
//		//rectangle(frame, Point(left, top - int(1.5 * labelSize.height)), Point(left + int(1.5 * labelSize.width), top + baseLine), Scalar(0, 255, 0), FILLED);
//		putText(srcimg, label, Point(x0, y0), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 1);
	}
	// cout << "maxClasses:" << maxClasses << endl;
	// cout <<"maxConfidences"<<maxConfidences<<endl;
	return maxClasses;
}
