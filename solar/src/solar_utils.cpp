#include<solar_utils.h>

void fillminrect(cv::RotatedRect rect,cv::Mat mask) 
{
	cv::Point2f rectP[4];
	//cv::Mat m(cv::Size(mask.rows, mask.cols), CV_8UC1, 255);
	
	rect.points(rectP);
	vector<cv::Point> connerP;
	cv::Point tempp;
	for (int i = 0; i < 4; i++)
	{	
		tempp.x = int(rectP[i].x);
		tempp.y = int(rectP[i].y);
		connerP.push_back(tempp);
		//cout << "ZC connerP " << tempp << endl;
	}

	//cv::drawContours(mask, connerP, 255, -1);
	cv::fillPoly(mask, connerP, cv::Scalar(255));


}

// bool Isinpolygon(PLOTS pot, vector<PlotInfo> pots)
// {
//     int counts = 0;
//     for (int i = 0; i < (pots.size() - 1); i++)
//     {
//         PlotInfo start_pot = pots[i];
//         PlotInfo end_pot = pots[i + 1];

//         double start_x = stod(start_pot.longitude);
//         double start_y = stod(start_pot.latitude);
//         double end_x = stod(end_pot.longitude);
//         double end_y = stod(end_pot.latitude);

//         PLOTS s_pot, e_pot;
//         s_pot.x = start_x;
//         s_pot.y = start_y;
//         e_pot.x = end_x;
//         e_pot.y = end_y;

//         if (Isinline(pot, s_pot, e_pot))
//         {
//             counts++;
//         }
//     }

//     if (counts % 2 == 1)
//     {
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }


bool Isinrotaterect(cv::Point pot, cv::RotatedRect rect) 
{
	cv::Point2f connerP[4];
	rect.points(connerP);
	float minx =9999,miny=9999,maxx=0,maxy=0;
	int counts = 0;
	for (int i = 0; i < (4 - 1); i++)
	{
		cv::Point start_pot = connerP[i];
		cv::Point end_pot = connerP[i + 1];

		
		if(start_pot.x<minx){
			minx=start_pot.x;
		}
		if(start_pot.y<miny){
			miny=start_pot.y;
		}
		if(start_pot.x>maxx){
			maxx=start_pot.x;
		}
		if(start_pot.y>maxy){
			maxy=start_pot.y;
		}


		if (Isinline(pot, start_pot, end_pot))
		{
			counts++;
		}
	}
	bool inpolgn=false;
	if(pot.x>minx && pot.y>miny&& pot.x<maxx && pot.y<maxy)
	{
		inpolgn=true;
		// cout<<"最大最小点："<<minx<<" "<<miny<<" "<<maxx<<" "<<maxy<<endl;
	}
	
	

	if (counts % 2 == 1 &&  inpolgn)
	{	
		// cout<<"第个点： "<<connerP[0].x<<" "<<connerP[1].x<<" "<<connerP[2].x<<" "<<connerP[3].x<<endl;
		// cout<<"第个点： "<<connerP[0].y<<" "<<connerP[1].y<<" "<<connerP[2].y<<" "<<connerP[3].y<<endl;
		// cout<<"两个组建的问题"<<pot<<" "<<endl;
		return true;
	}
	else
	{
		return false;
	}

	

}

bool Isinline(cv::Point pot, cv::Point s_pot, cv::Point e_pot)
{
	if ((pot.y < s_pot.y || pot.y < e_pot.y) && (abs(pot.x - s_pot.x) + abs(pot.x - e_pot.x) == abs(e_pot.x - s_pot.x)))
	{	
		if (s_pot.x -e_pot.x ==0)
		{	
			double delta = 0.0001;
			auto y = e_pot.y + (pot.x - e_pot.x) / delta * (s_pot.y - e_pot.y);
			if (y > pot.y)
			{
				return true;
			}
			else
			{
				return false;
			}
			
		}
		else {
			auto y = e_pot.y + (pot.x - e_pot.x) / (s_pot.x - e_pot.x) * (s_pot.y - e_pot.y);
			if (y > pot.y)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		
	}
	else
	{
		return false;
	}
}

cv::RotatedRect boxTorotateRect(cv::Rect box) 
{
	cv::RotatedRect rotatedBox;
	rotatedBox.angle = 0;
	rotatedBox.size.height = box.height;
	rotatedBox.size.width = box.width;
	rotatedBox.center.x = box.x + box.width / 2;
	rotatedBox.center.y = box.y + box.height / 2;
	return rotatedBox;

}

float getboxiouratio(cv::Rect rect1,cv::Rect rect2)
{
	cv::Rect urect = rect1|rect2;
	cv::Rect irect = rect1&rect2;
	float x = min(rect1.area(),rect2.area());
	cout<<"两个矩形框IOU计算  "<<irect.area()*1.0 /x<<endl;
	return irect.area()*1.0 /x;
}

float getboxiouminarea(cv::Rect rect1,cv::Rect rect2)
{
	cv::Rect irect = rect1&rect2;
	return irect.area()*1.0;
}


void rebuildcrossedrect(cv::Rect &crossedrect,cv::Mat img)
{
	if((crossedrect.x)<0)
	{
		crossedrect.x=0;
	}
	if((crossedrect.y)<0)
	{
		crossedrect.y=0;
	}
	if((crossedrect.x+crossedrect.width)>img.cols)
	{
		crossedrect.x=img.cols-crossedrect.width-1;
		if((crossedrect.x)<0)
		{
			crossedrect.x=0;
			crossedrect.width = img.cols-2;
		}
		cout<<"x修正"<<endl;
	}
	if((crossedrect.y+crossedrect.height)>img.rows)
	{
		crossedrect.y=img.rows-crossedrect.height-1;
		if((crossedrect.y)<0)
		{
			crossedrect.y=0;
			crossedrect.height = img.rows-2;
		}
		cout<<"y修正"<<endl;
	}


	// if(crossedrect.x+crossedrect.width>img.cols)
	// {
	// 	crossedrect.width = img.cols-2-crossedrect.x;
	// }
	// if(crossedrect.y+crossedrect.height>img.rows)
	// {
	// 	crossedrect.height = img.rows-2-crossedrect.y;
	// }

	if(crossedrect.width<=0 )
	{
		crossedrect.width=1;
	}
	if(crossedrect.height<=0 )
	{
		crossedrect.height=1;
	}
	
}

void CVapprox(vector<cv::Point> &cons)
{
	// 魔改了opencv库的approxPolyDP
	double n = 0.02;
	// cout<<"四边形拟合输入点："<<cons<<endl;
	double epsilon  = cv::arcLength(cons,true)*n;
	cv::approxPolyDP(cons,cons,epsilon ,true);
	while (1)
	{
		if(cons.size()<=4)break;
		else{
			n*=1.5;
			epsilon = cv::arcLength(cons,true)*n;
			cv::approxPolyDP(cons,cons,epsilon ,true);
			continue;
		}
		
	}
	// cout<<"四边形拟合输出点："<<cons<<endl; //wzj20230711
	
}	

Mat getquadsolarimg(vector<cv::Point>cons , cv::Mat img)
{
	// 输入轮廓点集  输出为透视变换的 矩形
	// 详情见 wzj博客公众号
	
	
	CVapprox(cons); //判断 左上 左下 等点的分布情况 

	int length = int(sqrt((cons[0].x-cons[1].x)*(cons[0].x-cons[1].x)+ (cons[0].y-cons[1].y)*(cons[0].y-cons[1].y)));
	int width =  int(sqrt((cons[1].x-cons[2].x)*(cons[1].x-cons[2].x)+ (cons[1].y-cons[2].y)*(cons[1].y-cons[2].y)));
	
	vector<Point2f> src_corners(4);
	src_corners[0] = Point2f(cons[0]);
	src_corners[1] = Point2f(cons[1]);
	src_corners[2] = Point2f(cons[2]);
	src_corners[3] = Point2f(cons[3]);


 
	Mat result_images = Mat::zeros(width, length, CV_8UC3);
	vector<Point2f> dst_corners(4);
	dst_corners[0] = Point2f(0, 0);
	dst_corners[1] = Point2f(length, 0);
	dst_corners[2] = Point2f(length, width);
	dst_corners[3] = Point2f(0, width);
	
	// cout<<"输入角点： "<<src_corners<<" "<<"输出角点 "<<dst_corners<<endl;
	Mat warpmatrix = getPerspectiveTransform(src_corners, dst_corners); //获取透视变换矩阵
	warpPerspective(img, result_images, warpmatrix, result_images.size()); 
	
	return result_images;
}

//抠出旋转矩形区域
Mat extractPerspective(const cv::Mat &image, cv::RotatedRect &ro_rect)
{
    // 仿射后的坐标
    float w = ro_rect.size.width;
    float h = ro_rect.size.height;
    cv::Point2f corners[4]; //顺序：左下-左上-右上-右下
    ro_rect.points(corners);
    //数组转容器
    vector<cv::Point2f> corners1;
    for (int i = 0; i < 4; i++)
        corners1.push_back(corners[i]);

    // 仿射变换得到 perspected_solar_panel
    std::vector<cv::Point2f> tgt_corners(4); //左下-左上-右上-右下
    tgt_corners[0] = cv::Point2f(0.0, h - 1);
    tgt_corners[1] = cv::Point2f(0.0, 0.0);
    tgt_corners[2] = cv::Point2f(w - 1, 0.0);
    tgt_corners[3] = cv::Point2f(w - 1, h - 1);
    cv::Mat warp = cv::getPerspectiveTransform(corners1, tgt_corners);
    cv::Mat perspected_solar_panel;
    cv::warpPerspective(image, perspected_solar_panel, warp, cv::Size(w, h));

    // 保证仿射后的图像 h > w
    if (perspected_solar_panel.rows > perspected_solar_panel.cols)
        perspected_solar_panel = perspected_solar_panel.t();
    //横向减小4像素，纵向减小1
    // cv::Rect rect1(4, 1, 120, 62);
    // perspected_solar_panel = perspected_solar_panel(rect1);
    return perspected_solar_panel;
}


void jibian(Mat img ,float k1,float k2,float p1,float p2)
{
    int img_w = img.cols;
    int img_h = img.rows;
    float cen_x = (img_w -1.0)/2.0;
    float cen_y = (img_h -1.0)/2.0;
    float norm_xy = (cen_x + cen_y)/2.0;
    Mat drawimg = Mat::zeros(Size(img_w,img_h),CV_8UC3);
    for (int j = 0; j < img_w; j++)
    {   
        double xoff = (j-cen_x)/norm_xy;
        // cout<<"x"<<xoff<<endl;
        for (int i = 0; i < img_h; i++)
        {
            double yoff = (i-cen_y)/norm_xy;
            // cout<<"y"<<yoff<<endl;
            double r = sqrt(xoff*xoff+yoff*yoff);
            double r2 = r*r;
            double r3 = r*r*r;
            double r4 = r2*r2;
            double rdest = (k1 *r4 + k2*r3 +p1*r2 +p2 *r)*norm_xy;
            double angle =atan2(yoff,xoff);
            double targetx = cen_x +(rdest *cos(angle));
            double targety = cen_y +(rdest *sin(angle));
            double xbase = floor(targetx);
            double delX  =targetx -xbase ;
            double ybase = floor(targety);
            double delY  =targety -ybase ;

            if( targetx>=0 && targetx < img_w-1 && targety>=0 && targety < img_h-1 )
            {   
                
                drawimg.at<Vec3b>(i,j)[0] =img.at<Vec3b>(int(targety),int(targetx))[0];
                drawimg.at<Vec3b>(i,j)[1] =img.at<Vec3b>(int(targety),int(targetx))[1];
                drawimg.at<Vec3b>(i,j)[2] =img.at<Vec3b>(int(targety),int(targetx))[2];
             
            }
            
        }
        
    }
	img = drawimg.clone();
    // return drawimg;
}


void SouthAndNorth(Mat m_zpic,double m_ZGimbalYawDegree)
{
    // 设计如下，在可见光图像左下角 选择300*300的区域 画指南针效果图，取针长100像素，字体30像素 ,用于30T,如果想通用 可以做个判断。
    int center_x = 150;
    int center_y = 2850;
    int r =80;

    // 指南坐标系和数学坐标系转换
    double N_altha = 0.0;
    double E_altha = 0.0;


    if (m_ZGimbalYawDegree>0 && m_ZGimbalYawDegree<=180){
        N_altha = 90 - m_ZGimbalYawDegree;
        
    }
    else
    {
        N_altha = -270 - m_ZGimbalYawDegree;
    }
    N_altha = N_altha/180*3.1415926;
    
    E_altha = N_altha + 3.1415926/2;
    int N_Point_x ,N_Point_y,E_Point_x,E_Point_y;

    N_Point_x = int(center_x-r*cos(N_altha));
    N_Point_y = int(center_y-r*sin(N_altha)); //北

    int N_x =  int(center_x-(r+50)*cos(N_altha));
    int N_y =  int(center_y-(r+50)*sin(N_altha)); 

    cout<<"云台角度: "<<m_ZGimbalYawDegree<<endl;
    

    E_Point_x = int(center_x-r*cos(E_altha));
    E_Point_y = int(center_y-r*sin(E_altha)); //东

    int E_x =  int(center_x-(r+50)*cos(E_altha));
    int E_y =  int(center_y-(r+50)*sin(E_altha)); 

    cout<<"北向点："<<N_Point_x<<" "<<N_Point_y<<"  南向点： "<<E_Point_x<<" "<<E_Point_y<<endl;

    cv::line(m_zpic,Point(center_x,center_y),Point(N_Point_x,N_Point_y),Scalar(0,0,255),3);
    cv::line(m_zpic,Point(center_x,center_y),Point(E_Point_x,E_Point_y),Scalar(0,0,255),3);

    
    cv::putText(m_zpic, "N", Point(N_x,N_y), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 4, 8, 0);
    cv::putText(m_zpic, "E", Point(E_x,E_y), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 4, 8, 0);


}

// vector<int> compression_params;
//     compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);  //选择jpeg
//     compression_params.push_back(100); 

void debugwriteimg(Mat inputMat,string outputname,int quality , bool debug)
{
	if(debug)
	{
		if(inputMat.empty())
		{
			return ;
		}
		else
		{	
			vector<int> compression_params;
    		compression_params.push_back(IMWRITE_JPEG_QUALITY);  //选择jpeg
    		compression_params.push_back(quality); 

			cv::imwrite(outputname,inputMat,compression_params);

		}


	}
	else
	{
		return ;
	}
}

//********************************
double stringToNum(const string &str)
{
    istringstream iss(str);
    double num;
    iss >> num;
    return num;
}
vector<string> splitStr(string str, char delimiter)
{
    vector<string> r;
    string tmpstr;
    while (!str.empty())
    {
        int ind = str.find_first_of(delimiter);
        if (ind == -1)
        {
            r.push_back(str);
            str.clear();
        }
        else
        {
            r.push_back(str.substr(0, ind));
            str = str.substr(ind + 1, str.size() - ind - 1);
        }
    }
    return r;
}





int getimginfo(string filepath,DJIdata &djiimginfo)
{
	FILE *fp = fopen(filepath.c_str(), "rb");
    if (!fp)
    {
        printf("Can't open file.\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    unsigned long fsize = ftell(fp);
    rewind(fp);
    unsigned char *buf = new unsigned char[fsize];
    if (fread(buf, 1, fsize, fp) != fsize)
    {
        printf("Can't read file.\n");
        delete[] buf;
        return -2;
    }
    fclose(fp);

    // Parse EXIF
    easyexif::EXIFInfo result;
    int code = result.parseFrom(buf, fsize);
    delete[] buf;
    if (code)
    {
        printf("Error parsing EXIF: code %d\n", code);
        return -3;
    }

	djiimginfo.ImgJD = result.GeoLocation.Longitude;
	djiimginfo.ImgWD = result.GeoLocation.Latitude;
    djiimginfo.Focallength = result.FocalLength;


    //再次打开文件
    fp = fopen(filepath.c_str(), "rb");
    char *line = NULL;
    if (!fp)
    {
        cout << "Can't open file." << endl;
        return -1;
    }

    size_t len = 0;
    ssize_t read;
    int iCount = 0;
    //遍历读取每一行
    while ((read = getline(&line, &len, fp)) != -1)
    {
        string strTemp = line;

        //*****************无人机镜头型号*******************//
	 	if(strTemp.find("tiff:Model")!= string::npos)
        {
            vector<string> vxs = splitStr(strTemp, '"'); // 无人机镜头型号
			// djiimginfo.Cameratype = vxs[1];
			// djiimginfo.Cameratype = vxs[1].data();
			for( int i=0;i<vxs[1].length();i++)
			{
    			djiimginfo.Cameratype[i] = vxs[1][i];
			}
            cout<<"无人机镜头型号为 "<<djiimginfo.Cameratype<<endl;
        }

        //*****************无人机飞行高度*******************//
        else if (strTemp.find("drone-dji:LRFTargetDistance") != string::npos) 
        {
            vector<string> vxs = splitStr(strTemp, '"'); // drone-dji:FlightYSpeed="-2.7"
            djiimginfo.Flyheight = stringToNum(vxs[1]);
            cout<<"无人机飞行高度为 "<<djiimginfo.Flyheight<<endl;
        } 

		//*****************无人机云台俯仰角*******************//
        else if (strTemp.find("drone-dji:GimbalPitchDegree") != string::npos)
        {
            vector<string> vxs = splitStr(strTemp, '"'); // drone-dji:FlightYSpeed="-2.7"
           	djiimginfo.GimbalPitch = stringToNum(vxs[1]);
			cout<<"无人机云台俯仰角为 "<<djiimginfo.GimbalPitch<<endl;
        } 
		
		//*****************无人机云台偏航角*******************//
        else if (strTemp.find("drone-dji:GimbalYawDegree") != string::npos)
        {
            vector<string> vxs = splitStr(strTemp, '"'); // drone-dji:GimbalYawDegree="-89.7"
			djiimginfo.GimbalYaw = stringToNum(vxs[1]);
			cout<<"无人机云台偏航角为 "<<djiimginfo.GimbalYaw<<endl;
       
        }
       
    }

    if (line)
        free(line);
    //关闭文件
    fclose(fp);
}

