#include<vis_detect.h>
// #include<spdlog/spdlog.h>  //调用三方库都在当前CPP文件调用

DetectVisLogic::DetectVisLogic(/* args */)
{
    visbleSolarSegNet.ModelInit(visble_seg_solar_model_path);
    visbleZcSegNet.ModelInit(visble_seg_zc_model_path);

    visblePicDetectNet.ModelInit(visble_Detect_pic_model_path,1440.0,1440.0,0.4,0.4);  
    visbleSolarDetectNet.ModelInit(visble_Detect_solar_model_path,320.0,320.0,0.4,0.4);
    cout<<"可见模块初始化"<<endl;

}
DetectVisLogic:: ~DetectVisLogic()
{
    cout<<"可见模块处理完毕"<<endl;
}

cv::Mat DetectVisLogic::getZ_SolarMask(cv::Mat _ZinputImg) 
{	
	//先将可见光大图调整为 x*y份，再将 x*y份喂入 推理模块 生成也是x*y份mask ,最后根据x*y份 mask 组合完整mask。
	//1、
	cv::Mat wholeMask = cv::Mat::zeros(3888, 5184, CV_8UC1); //return 

	vector<vector<cv::Mat>>cut_imgs,cut_mask;
	cut_mask = partitionImage(wholeMask);
    cut_imgs = partitionImage(_ZinputImg);
	// cut_imgs = getPartNumImgList(_ZinputImg);
	cout << "cut_imgs_size " << cut_imgs.size() << endl;
	cout << "cut_imgs_size " << cut_mask.size() << endl;
	//
	//partNumY
	for (int ia = 0; ia < partNumY; ia++)
	{
		for (int ja = 0; ja < partNumX; ja++)
		{
			cv::Mat maskParttemp = visbleSolarSegNet.detect(cut_imgs[ia][ja]); // 1/64 mask

			resize(maskParttemp, maskParttemp, cv::Size(cut_mask[ia][ja].cols, cut_mask[ia][ja].rows)); //模型输出变换
			// cout << "yuce kejianguang mask 16 -  " << maskParttemp.cols << " " << maskParttemp.rows << endl;
			cut_mask[ia][ja] = maskParttemp;                                            // 1/64 mask
			//cut_mask[ia][ja].copyTo(wholeMask(cv::Rect(ja * 648 * 2, ia * 486 * 2, 648 * 2, 486 * 2))); // 8*8拼接还原
			cut_mask[ia][ja].copyTo(wholeMask(cv::Rect(ja * 648 * 2, ia * 486 * 2, 648 * 2, 486 * 2)));
		}
	}
    if(_ZinputImg.cols ==4000)
    {
        cv::resize(wholeMask,wholeMask,_ZinputImg.size());
    }

	return wholeMask;
}

cv::Mat DetectVisLogic::getZ_ZcMask(cv::Mat _ZinputImg) 
{
	cv::Mat zcmask = visbleZcSegNet.detect(_ZinputImg);
	//cv::resize(zcmask, zcmask, cv::Size(_ZinputImg.cols, _ZinputImg.rows));
	return zcmask;
}

// vector<vector<cv::Mat>>  DetectVisLogic::getPartNumImgList(cv::Mat _ZinputImg) 
// {	

// 	vector<vector<cv::Mat>> cut_imgs;

// 	switch (_ZinputImg.cols)
// 	{
// 	case 5184:
// 		cut_imgs = partitionImage(_ZinputImg);
// 		break;

// 	case 4000:
// 		cv::Mat imgbigged = cv::Mat::zeros(3888, 5184, CV_8UC3);
// 		cv::Rect rectM30T = cv::Rect(592, 444, 4000, 3000);
// 		_ZinputImg.copyTo(imgbigged(rectM30T));
// 		cout << " kejianguang tuxiang daxiao " << _ZinputImg.cols << " " << _ZinputImg.rows << endl;
// 		cut_imgs = partitionImage(_ZinputImg);
//         // cut_imgs = partitionImage(imgbigged);
// 		break;
// 	}
// 	return cut_imgs;
// }

vector<vector<cv::Mat>> DetectVisLogic::partitionImage(cv::Mat src)
{	
	int rows = partNumY;
	int cols = partNumX;
	vector<vector<cv::Mat>> array(rows, vector<cv::Mat>(cols));

	int irows = src.rows, icols = src.cols;
	int dr = irows / rows, dc = icols / cols;
	int delt_y = (irows % rows) / 2, delt_x = (icols % cols) / 2;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			int x = j * dc + delt_x, y = i * dr + delt_y;
			array[i][j] = src(cv::Rect(x, y, dc, dr));
		}
	}
	return array;
}

void DetectVisLogic::Mask2ZcInfo(cv::Mat& Mask, float _Zmaskpara)
{
    visbleZcParasmap.clear();
    visbleZcParas.clear();
    
    cv::Mat drawmask = cv::Mat::zeros(cv::Size(Mask.cols, Mask.rows), CV_8UC3);

    cv::threshold(Mask, Mask, _Zmaskpara * 255, 255, cv::THRESH_BINARY);

    cv::Mat element;
    element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(17,17));

    //    第一种实现方法
    //morphologyEx(Mask, Mask, cv::MORPH_OPEN, element);

    cv::erode(Mask, Mask, element);



    vector<vector<cv::Point>>contours;
    vector<cv::Vec4i>hierarchy;


    cv::findContours(Mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    vector<cv::Point> hull(contours.size()); //外轮廓
    vector<cv::RotatedRect> rorated_rectsAll, rorated_rectsfilted; //存放旋转矩形   

    int id=0 ;
    for (int i = 0; i < contours.size(); i++)
    {   
        float marea;
        float maxarea = 0;
       
        cv::Point2d rectP[4];
        //vector<cv::Point>rectP;
        cv::RotatedRect mrect = cv::minAreaRect(contours[i]);
        
        marea = mrect.size.area();
        
        
        if (marea > 300000) 
        {   
            // cout << "maera" << marea << endl;

            fillminrect(mrect, Mask);  //主要保证组串mask是被遮挡影响。

            //cv::drawContours(Mask, rectP, 255, -1);
            id += 1;
            
            visbleZcParasmap[id] = mrect;
            
            if (marea > maxarea) {
                maxarea = marea;
                stddelta = mrect.angle;
            }
        }
        

    }
    
}

void DetectVisLogic::Mask2SolarInfo(cv::Mat& Mask,float _Zmaskpara)
{   
    // infraSolarParas.clear();//每张图像需要清除记录
	//用来显示和调试的          
	// cv::Mat drawmask= cv::Mat::zeros(cv::Size(Mask.cols, Mask.rows), CV_8UC3);
    drawmask= cv::Mat::zeros(cv::Size(Mask.cols, Mask.rows), CV_8UC3);
    
	cv::threshold(Mask, Mask,_Zmaskpara*255,255, cv::THRESH_BINARY);
    vector<vector<cv::Point>>contours;
    vector<cv::Vec4i>hierarchy;
    cv::findContours(Mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    vector<cv::Point> hull(contours.size()); //外轮廓
    vector<cv::RotatedRect> rorated_rectsAll, rorated_rectsfilted; //存放旋转矩形    
    vector<float>solarheight, solarwidth, solarareas;       //会保存在

    //
    stdRect_w = 0.0, stdRect_h = 0.0; 
    int solarnum = 0;
    VisblesolarPar visblesolarpar;
    int k=0 ;
    cout<<"可见光mask 轮廓数目 "<<contours.size()<<endl;
    for (int i = 0; i < contours.size(); i++)
    {   
        float conarea,fillratio,aspectratio;
        cv::RotatedRect rect = cv::minAreaRect(contours[i]);
        conarea = cv::contourArea(contours[i]);
        // cout<<"轮廓面积："<<conarea<<endl;
        fillratio = conarea / rect.size.area();
        solarareas.push_back(rect.size.area());                      
        // rect.size.width += _extra_w;
        // rect.size.height += _extra_h;
        rorated_rectsAll.push_back(rect);
        
        float solarH, solarW;
        solarH = rect.size.height < rect.size.width ? rect.size.height : rect.size.width;
        solarW = rect.size.height < rect.size.width ? rect.size.width : rect.size.height;
        aspectratio = solarW / solarH;
         
       
        cv::Point2f connerP[4];
        rect.points(connerP);  
        /*connerP[2]*/  //这些参数是要获取的

        //不能超出边界
        int outofboundary = 2;
     
        for (int pt = 0; pt < 4; pt++) //点不能超出视野 
        //
        {
            if(connerP[pt].x<=1 || connerP[pt].x>=Mask.cols-2 ||  connerP[pt].y<=1||connerP[pt].y>=Mask.rows-2  )
            {
                outofboundary = 3;  //超出视野
                break;
            }
             
        }
        
        //该参数

        if (fillratio>0.8 && aspectratio>1.05 && aspectratio<3.5 && conarea<120000 && conarea>20000  && outofboundary ==2 )
        {   
            //cout << "solar area   " << conarea << endl;
            k+=1;
            // cout<<"组件框点: "<<k<<" ";
            for (int line_i = 0; line_i < 4; line_i++)
            {   
                
                // cout<< connerP[line_i];
                cv::line(drawmask, connerP[line_i], connerP[(line_i + 1) % 4], cv::Scalar(255, 100, 200), 2);
            }


            // CVapprox(contours[i]);// wzj20230711
            // cout<<"测试四边形拟合： "<<contours[i][0]<<" "<<contours[i][1]<<" "<<contours[i][2]<<" "<<contours[i][3]<<endl;   
            visblecontoursolar.push_back(contours[i]); // wzj20230711

            //记录组件信息
            visblesolarpar.rotate_rect = rect;  
            visblesolarpar.zc_id = 0;   //默认不在  0表示不在任何一个组串
        
            bool  is_inzc= false;

            // cout << " ZC length " << infraZcParas.size() << endl;
            for (int zc_id = 0; zc_id < visbleZcParas.size(); zc_id++)
            {   
                is_inzc= Isinrotaterect(rect.center, visbleZcParas[zc_id].rotate_rect);
                if (is_inzc)
                {   
                    cout << "zai ZC  " << visbleZcParas[zc_id].id << " nei" << endl;
                    visblesolarpar.zc_id = visbleZcParas[zc_id].id;
                    break;
                }
               
         
            }
            if(is_inzc==false) cv::drawContours(drawmask, contours, i, cv::Scalar(255, 128, 22), 1);
            visbleSolarParas.push_back(visblesolarpar);
            

            stdRect_w += solarW;
            stdRect_h += solarH;
           
            solarnum ++;
            // cout<<"第几个："<<solarnum<<"  "<<stdRect_w<<" "<<stdRect_h<<endl;

        }

       
    }
    if(solarnum>0)
    {
        stdRect_w = stdRect_w / solarnum;
        stdRect_h = stdRect_h / solarnum;
    }
    
    

    cout << " 可见光组件平均宽高     " << stdRect_w << " " << stdRect_h << endl;
    

    //

}


void DetectVisLogic::getFaultPic(cv::Mat _ZinputImg,vector<Detectiontype>&detectRes)
{   
    visblePicDetectNet.detect(_ZinputImg,detectRes,vispicfaluttype);
                         
    cout<<"新版可见全图检测故障树: "<<detectRes.size()<<endl;
}


void DetectVisLogic::getFaultSolar(cv::Mat _ZinputImg,vector<Detectiontype>&detectRes)
{
    visbleSolarDetectNet.detect(_ZinputImg,detectRes,vissolarfaluttype);
    //取置信度大于一定值 比如 都大于0.9  然后取面积最大的  否则取置信度最大。
 
    float maxconf =0.0;
    float maxarea =0.0;
    int MaxAreaNum =0 , MaxConfNum =0;
    // SolarResinfo = {}; //结构体初始化为空
    SolarResinfo.classid =-1;
    for (int i = 0; i < detectRes.size(); i++)
    {
        if (detectRes[i].confidence>maxconf)
        {
            maxconf = detectRes[i].confidence;
            MaxConfNum = i;
        }
        SolarResinfo = detectRes[MaxConfNum];
    }
    if(detectRes.size()>0){
        cout<<"新版可见组件检测故障树: "<<detectRes.size() <<" "<<SolarResinfo.classid<<" "<<SolarResinfo.confidence <<endl; 
    }
    
}






void DetectVisLogic::writeDrawdetectedImg(cv::Mat _ZinputImg, string  savepath, vector<Detectiontype>&detectRes,vector<string > class_names)
{
    visblePicDetectNet.drawresult(_ZinputImg,detectRes,class_names);
    cv::imwrite(savepath,_ZinputImg);
}

/*
void DetectVisLogic::combineDetectinfo()
{
    visbleFaultJson visblefaultjson;
 
    //  故障组件的识别 和 故障 大图 需要去重

    for (int i = 0; i < visFaultSolarDetectsOutputs.size(); i++)
    {   

        cv::Point2d falutcenter;
        falutcenter.x = int(visFaultSolarDetectsOutputs[i].box.x + visFaultSolarDetectsOutputs[i].box.width / 2);
        falutcenter.y = int(visFaultSolarDetectsOutputs[i].box.y + visFaultSolarDetectsOutputs[i].box.height / 2);

       
        
        for (int j = 0; j < visbleSolarParas.size(); j++)  //第一步在solar 分割效果有关
        {
           
            if (Isinrotaterect(falutcenter, visbleSolarParas[j].rotate_rect)) 
            {   
                visbleFaultJson.ZC_id = visbleSolarParas[j].zc_id;  //组串id

                visbleFaultJson.ZCquads = visbleZcParasmap[visbleSolarParas[j].zc_id];

                visbleFaultJson.visblesolarquads = visbleSolarParas[j].rotate_rect;
                //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id; 
                //这里的故障id对应Faulttype[8]
                visbleFaultJson.visble_fault_id = vissolarfaluttype[visFaultSolarDetectsOutputs[i].classid];

                visbleFaultJson.confidence = visFaultSolarDetectsOutputs[i].confidence;

                visbleFaultJson.visblefaultsolarbox = visFaultSolarDetectsOutputs[i].box;

                visbleFaultJson.visblesolarboxarea = visFaultSolarDetectsOutputs[i].box.area();
                // cout << "this faultsolar in solar vector " << endl;
              
                break;  
            }
            // cout << "this faultsolar  not in solar vector " << endl;
        }
        
       
        visblefaultinfos.push_back(visbleFaultJson);       //故障组件的添加

    }
    cout<<"可见故障组件数1: "<<visblefaultinfos.size()<<endl;

}*/



void DetectVisLogic::combineDetectinfo()
{
    visbleFaultJson visblefaultjson;
    // /*
    for (int i = 0; i < solardetectres.size(); i++)
    {   
        // visblefaultjson = solardetectres[i];

        visblefaultinfos.push_back(solardetectres[i]);
    }
    // */
   cout<<"可见故障组件数1: "<<visblefaultinfos.size()<<endl;

   for (int i = 0; i < visFaultPicDetectsOutputs.size(); i++)
   {
        //这个不需要统计组件 即框代替组件
        cv::Point2d falutcenter;
        falutcenter.x = int(visFaultPicDetectsOutputs[i].box.x + visFaultPicDetectsOutputs[i].box.width / 2);
        falutcenter.y = int(visFaultPicDetectsOutputs[i].box.y + visFaultPicDetectsOutputs[i].box.height / 2);

        bool isinzc = false;
        for (auto it = visbleZcParasmap.begin(); it != visbleZcParasmap.end(); it++)
        {
            if(Isinrotaterect(falutcenter, it->second))
            {
                visblefaultjson.ZC_id = it->first;
                visblefaultjson.ZCquads = it->second;
                visblefaultjson.visblefaultsolarbox = visFaultPicDetectsOutputs[i].box;
                visblefaultjson.visblesolarboxarea = visFaultPicDetectsOutputs[i].box.area();
                visblefaultjson.confidence = visFaultPicDetectsOutputs[i].confidence;
                visblefaultjson.visble_fault_id = vispicfaluttype[visFaultPicDetectsOutputs[i].classid];
                isinzc = true;
                break;
            }
            

        }
        if(isinzc==false)
            {
                visblefaultjson.ZC_id = 0;
                visblefaultjson.visblefaultsolarbox = visFaultPicDetectsOutputs[i].box;
                visblefaultjson.visblesolarboxarea = visFaultPicDetectsOutputs[i].box.area();
                visblefaultjson.confidence = visFaultPicDetectsOutputs[i].confidence;
                visblefaultjson.visble_fault_id = vispicfaluttype[visFaultPicDetectsOutputs[i].classid];
            
            }
            visblefaultinfos.push_back(visblefaultjson);
   }
   cout<<"可见故障组件数1: "<<visblefaultinfos.size()<<endl;
   
    
}
