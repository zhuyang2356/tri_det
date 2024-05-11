#include "read_dji_thremal.h"

 Read_Dji_Thermal::Read_Dji_Thermal(string rjpeg_file_path)
{
	rjpeg_file_path_ = rjpeg_file_path;
	raw_data_ = "";
}

 //��ȡ��raw����
 bool Read_Dji_Thermal::getDjiRawData()
 {
	int ret = 0;
	DIRP_HANDLE dirp_handle = nullptr;

	long max_temp =0;

	dirp_resolution_t rjpeg_resolution = {0};

	int32_t out_size = 0;
	void *raw_out = nullptr;


	ret = access(rjpeg_file_path_.c_str(), 0);

	if (0 != ret)
	{
		cout << "ERROR: source file " << rjpeg_file_path_.c_str() << " not exist" << endl;
		return false;
	}

	ifstream fs_i_rjpeg;
	cout << "R-JPEG file path : " << rjpeg_file_path_.c_str() << endl;

	struct stat rjpeg_file_info;
	stat(rjpeg_file_path_.c_str(), &rjpeg_file_info);

	int32_t  rjpeg_size = (uint32_t)rjpeg_file_info.st_size;
	uint8_t *rjpeg_data = (uint8_t *)malloc(rjpeg_size);
	if (nullptr == rjpeg_data)
	{
		cout << "ERROR: malloc failed" << endl;
		goto ERR_DIRP_RET;
	}

	fs_i_rjpeg.open(rjpeg_file_path_.c_str(), ios::binary);
	FSTREAM_OPEN_CHECK(fs_i_rjpeg , "rjpeg.jpg", ERR_DIRP_RET);
	fs_i_rjpeg.read((char *)rjpeg_data, rjpeg_size);

	/* Create a new DIRP handle */
	cout<<"大疆测温图像："<<*rjpeg_data<<" "<<rjpeg_size<<" "<<dirp_handle<<endl;
	ret = dirp_create_from_rjpeg(rjpeg_data, rjpeg_size, &dirp_handle);
	if (DIRP_SUCCESS != ret)
	{
		cout << "ERROR: create R-JPEG dirp handle failed , error no:" << ret << endl;
		goto ERR_DIRP_RET;
	}

	/* get rjpeg resolution */
	ret = dirp_get_rjpeg_resolution(dirp_handle, &rjpeg_resolution);
	if (DIRP_SUCCESS != ret)
	{
		cout << "ERROR: call dirp_get_rjpeg_version failed" << endl;
		goto ERR_DIRP_RET;
	}
	cout << "R-JPEG resolution size" << endl;
	cout << "      image  width : " << rjpeg_resolution.width  << endl;
	cout << "      image height : " << rjpeg_resolution.height << endl;
	image_w_ = rjpeg_resolution.width ;
	image_h_ = rjpeg_resolution.height ;
	//****************************************************************************
	out_size = rjpeg_resolution.width * rjpeg_resolution.height * sizeof(int16_t);

	raw_out = (void *)malloc(out_size);
	if (nullptr == raw_out)
	{
		cout << "ERROR: malloc memory failed" << endl;
		ret = -1;
		goto ERR_DIRP_RET;
	}

	ret = dirp_measure(dirp_handle, (int16_t *)raw_out, out_size);

	if (DIRP_SUCCESS != ret)
	{
		cout << "ERROR: call dirp_get_[original_raw/measure/proess] failed" << endl;
		goto ERR_DIRP_RET;
	}

	raw_data_.assign((char *)raw_out, out_size);

ERR_DIRP_RET:
	/* Destroy DIRP handle */
	if (dirp_handle)
	{
		int status = dirp_destroy(dirp_handle);
		if (DIRP_SUCCESS != status)
		{
			cout << "ERROR: destroy dirp handle failed. "<< endl;
		}
	}

	if (fs_i_rjpeg.is_open())
		fs_i_rjpeg.close();

	if (rjpeg_data)
		free(rjpeg_data);

	if(raw_out)
		free(raw_out);
	return true;

	if(DIRP_SUCCESS == ret)
		return true;

	return false;

}

Read_Dji_Thermal::~Read_Dji_Thermal()
{

}


bool Read_Dji_Thermal::getPointTemperature(int x, int y, long& temperature)
{
	long offset = pointToIndex(x, y);

	//ƫ������������߽����˳�
	if (offset ==  -1)
	{
		return false;
	}

	//ÿ���¶�����ռ�������ֽ�16λ
	unsigned char tmpValue[2];
	//��ȡ�¶�
	tmpValue[0] = raw_data_.at(offset);
	tmpValue[1] = raw_data_.at(offset + 1);

	temperature = (tmpValue[1] << 8) + (tmpValue[0]);
	return true;
}

//����ָ����Χ�����ص��¶ȵ�ƽ��ֵ�����ֵ�����ֵ���ꡢ���ֵ�����ֵ����
bool Read_Dji_Thermal::getRangeTemperature(RectArea rect , float& temperature , PixTemperature& maxPT , PixTemperature& minPT )
{
	if (raw_data_.empty())
	{
		return false;
	}

	//�ж������Ƿ񳬳��߽�
	if (rect.x < 0
		|| rect.y < 0
		|| rect.w < 0
		|| rect.h < 0
		|| rect.x > image_w_
		|| rect.y > image_h_
		|| (rect.x + rect.w) > image_w_
		|| (rect.y + rect.h) > image_h_)
	{
		return false;
	}

	PixPoint maxTPP, minTPP, bottomRight;

	int maxTemp = -9999;
	int minTemp = 9999;

	bool pointsReady = false;//�������¶ȳ�ʼ�����

	long pointCount = 0;
	unsigned char _temp[2];
	//���е��¶�֮�͡�ʹ��__int64�������㹻��֤����ʱ���������
	//�¶ȱ���16λ����Χ�ǡ�32767��_int64��ʾ��Χ�ǡ�9,223,372,036,854,775,808
	//�������ֵ���㣬�ɱ���281,474,976,710,656������֮��
	//һ��1080*1080��ͼƬֻ��1,166,400������
	long long _temperature = 0;
	bottomRight.x = rect.x + rect.w;
	bottomRight.y = rect.y + rect.h;
	for (int x = rect.x; x < bottomRight.x; ++x)
	{
		for (int y = rect.y; y < bottomRight.y; ++y)
		{
			long pointTemp;//���¶�

			if(!getPointTemperature(x, y, pointTemp)){
				return false;
			}

			//�ж���ߡ�����¶Ȳ���¼����
			if (pointTemp < minTemp)
			{
				minTemp = pointTemp;
				minTPP.x = x;
				minTPP.y = y;
			}

			if (pointTemp > maxTemp)
			{
				maxTemp = pointTemp;
				maxTPP.x = x;
				maxTPP.y = y;
			}

			_temperature += pointTemp;
			++pointCount;
		}
	}
	if (pointCount == 0)
	{
		return false;
	}
	temperature = _temperature / pointCount * 0.1;
	maxPT.point = maxTPP;
	maxPT.temperature = maxTemp * 0.1;
	minPT.point = minTPP;
	minPT.temperature = minTemp * 0.1;
	return true;
}


//����ָ�����ص��ƽ��ֵ�����ֵ�����ֵ���ꡢ���ֵ�����ֵ����
//bool Read_Dji_Thermal::getVectorTemperature(vector<Point> vec_points, float& temperature , PixTemperature& maxPT , PixTemperature& minPT)
//{
//	if (raw_data_.empty())
//	{
//		return false;
//	}
//
//	PixPoint maxTPP, minTPP;
//
//	int maxTemp = -9999;
//	int minTemp = 9999;
//	long pointCount = 0;
//	unsigned char _temp[2];
//	long _temperature = 0;
//
//	for(unsigned int i = 0; i < vec_points.size(); ++i)
//	{
//		int pointTemp;//���¶�
//
//		if(!getPointTemperature(vec_points[i].x, vec_points[i].y, pointTemp)){
//			continue;
//		}
//
//		//�ж���ߡ�����¶Ȳ���¼����
//		if (pointTemp < minTemp)
//		{
//			minTemp = pointTemp;
//			minTPP.x = vec_points[i].x;
//			minTPP.y = vec_points[i].y;
//		}
//
//		if (pointTemp > maxTemp)
//		{
//			maxTemp = pointTemp;
//			maxTPP.x = vec_points[i].x;
//			maxTPP.y = vec_points[i].y;
//		}
//
//		_temperature += pointTemp;
//		++pointCount;
//	}
//
//	if (pointCount == 0)
//	{
//		return false;
//	}
//
//	temperature = _temperature / pointCount * 0.1;
//	maxPT.point = maxTPP;
//	maxPT.temperature = maxTemp * 0.1;
//	minPT.point = minTPP;
//	minPT.temperature = minTemp * 0.1;
//	return true;
//}

//bool Read_Dji_Thermal::rangeCaculate(RectArea rect , float& temperature )
//{
//	PixTemperature maxPT, minPT;
//	return rangeCaculate(rect, temperature, maxPT, minPT);
//}


long Read_Dji_Thermal::pointToIndex(int x, int y)
{
	//raw����Ϊ�գ����˳�
	if (raw_data_.empty())
	{
		return -1;
	}

	//�жϿ����Ƿ�Խ��
	if (image_w_ <= x)
	{
		return -1;
	}

	//�жϸ߶��Ƿ�Խ��
	if (image_h_ <= y)
	{
		return -1;
	}
	//ÿ���¶�����ռ�������ֽ�16λ
	long index = (image_w_ * y + x) * 2;

	if(index > raw_data_.size() - 2)
		return -1;

	return index;
}
