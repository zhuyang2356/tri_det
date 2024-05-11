#include<infra_detect.h>




DetectInfraLogic::DetectInfraLogic(/* args */)
{
    thermalSolarSegNet.ModelInit(thermal_Seg_solar_model_path);
    // thermalZcSegNet.ModelInit(thermal_Seg_zc_model_path);  //后续可以融合在一起
    thermalDcSegNet.ModelInit(thermal_Seg_dc_model_path);

    // thermalDetectNetspot(thermal_Detect_spot_model_path,true,cv::Size(640,640));
    thermalDetectNetspot.ModelInit(thermal_Detect_spot_model_path,640.0,640.0,0.5,0.4);
    thermalDetectNetsolar.ModelInit(thermal_Detect_solar_model_path,640.0,640.0,0.5,0.4);


    cout<<"红外模块初始化"<<endl;
}

DetectInfraLogic::~DetectInfraLogic()
{
    cout<<"红外模块处理完毕"<<endl;
}
/*  取消原因为 不单独使用
cv::Mat DetectInfraLogic::getT_ZcMask(cv::Mat _TinputImg)
{
    switch (_TinputImg.cols)            
	{
	case 640:
		_T_ZcMask = thermalZcSegNet.detect(_TinputImg);

		break;

	case 1280:
		_T_ZcMask = thermalZcSegNet.detectM30T(_TinputImg);   //有的需要
		break;
	}
	return _T_ZcMask;
}  */   //

cv::Mat DetectInfraLogic::getT_ZcMask(cv::Mat Tsolarmask)
{
     cv::Mat element,Mask;
     cv::threshold(Tsolarmask, Mask, 0.4 * 255, 255, cv::THRESH_BINARY);

    element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(13,13));

    cv::dilate(Mask, Mask, element);

    
    return Mask;
}

	
cv::Mat DetectInfraLogic::getT_SolarMask(cv::Mat  _TinputImg)
{
    switch (_TinputImg.cols)             //根据这个还得做畸变矫正
	{
	case 640:
		_T_SolarMask = thermalSolarSegNet.detect(_TinputImg);

		break;

	case 1280:
		_T_SolarMask = thermalSolarSegNet.detectM30T(_TinputImg);   //有的需要
		break;
	}
	return _T_SolarMask;
}

void DetectInfraLogic::Mask2ZcInfo(cv::Mat& Mask, float _Tmaskpara)
{
    infraZcParas.clear();
    infraZcParasmap.clear();
    
    cv::Mat drawmask = cv::Mat::zeros(cv::Size(Mask.cols, Mask.rows), CV_8UC3);

    cv::threshold(Mask, Mask, _Tmaskpara * 255, 255, cv::THRESH_BINARY);

    cv::Mat element;
    element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7));

    //    第一种实现方法
    //morphologyEx(Mask, Mask, cv::MORPH_OPEN, element);
    // cv::dilate(Mask, Mask, element);
    cv::erode(Mask, Mask, element);



    vector<vector<cv::Point>>contours;
    vector<cv::Vec4i>hierarchy;


    cv::findContours(Mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    vector<cv::Point> hull(contours.size()); //外轮廓
    vector<cv::RotatedRect> rorated_rectsAll, rorated_rectsfilted; //存放旋转矩形   

    
    InfrashowZcPar   infraZcPara;
    int id=0 ;
    for (int i = 0; i < contours.size(); i++)
    {   
        float marea;
        float maxarea = 0;
       
        cv::Point2f rectP[4];
        //vector<cv::Point>rectP;
        cv::RotatedRect mrect = cv::minAreaRect(contours[i]);
        
        marea = mrect.size.area();
        
        
        if (marea > 10000) 
        {   
            cout << "maera" << marea << endl;

            fillminrect(mrect, Mask);  //主要保证组串mask是被遮挡影响。

            //cv::drawContours(Mask, rectP, 255, -1);
            id += 1;
            infraZcPara.id =id;
            infraZcParasmap[id] = mrect;

            infraZcPara.rotate_rect= mrect;
            if (marea > maxarea) {
                maxarea = marea;
                stddelta = mrect.angle;
            }
            infraZcParas.push_back(infraZcPara);
             mrect.points(rectP);  
           
            cout<<"红外组串参数： "<<rectP[0].x<<" "<<rectP[1].x<<" "<<rectP[2].x<<" "<<rectP[3].x<<endl;
		    cout<<"红外组串参数： "<<rectP[0].y<<" "<<rectP[1].y<<" "<<rectP[2].y<<" "<<rectP[3].y<<endl;
        }
        

    }
    
}

void DetectInfraLogic::Mask2ZcInfox(cv::Mat& Mask, float _Tmaskpara,string _maskname)
{
    infraZcParas.clear();
    infraZcParasmap.clear();
    
    cv::Mat drawmask = cv::Mat::zeros(cv::Size(Mask.cols, Mask.rows), CV_8UC3);

    cv::threshold(Mask, Mask, _Tmaskpara * 255, 255, cv::THRESH_BINARY);

    cv::Mat element;
    element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7));

    //    第一种实现方法
    //morphologyEx(Mask, Mask, cv::MORPH_OPEN, element);
    
    cv::erode(Mask, Mask, element);
    // cv::dilate(Mask, Mask, element);

    cv::imwrite(_maskname,Mask);

    vector<vector<cv::Point>>contours;
    vector<cv::Vec4i>hierarchy;


    cv::findContours(Mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    vector<cv::Point> hull(contours.size()); //外轮廓
    vector<cv::RotatedRect> rorated_rectsAll, rorated_rectsfilted; //存放旋转矩形   

    
    InfrashowZcPar   infraZcPara;
    int id=0 ;
    for (int i = 0; i < contours.size(); i++)
    {   
        float marea;
        float maxarea = 0;
       
        cv::Point2d rectP[4];
        //vector<cv::Point>rectP;
        cv::RotatedRect mrect = cv::minAreaRect(contours[i]);
        
        marea = mrect.size.area();
        
        
        if (marea > 10000) 
        {   
            cout << "maera" << marea << endl;

            fillminrect(mrect, Mask);  //主要保证组串mask是被遮挡影响。

            //cv::drawContours(Mask, rectP, 255, -1);
            id += 1;
            infraZcPara.id =id;
            infraZcParasmap[id] = mrect;

            infraZcPara.rotate_rect= mrect;
            if (marea > maxarea) {
                maxarea = marea;
                stddelta = mrect.angle;
            }
            infraZcParas.push_back(infraZcPara);
        }
        

    }


    
}


void DetectInfraLogic::Mask2SolarInfo(cv::Mat& Mask,float _Tmaskpara)
{   
    // infraSolarParas.clear();//每张图像需要清除记录
	//用来显示和调试的          
	cv::Mat drawmask= cv::Mat::zeros(cv::Size(Mask.cols, Mask.rows), CV_8UC3);

    
	cv::threshold(Mask, Mask,_Tmaskpara*255,255, cv::THRESH_BINARY);
    vector<vector<cv::Point>>contours;
    vector<cv::Vec4i>hierarchy;
    cv::findContours(Mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    vector<cv::Point> hull(contours.size()); //外轮廓
    vector<cv::RotatedRect> rorated_rectsAll, rorated_rectsfilted; //存放旋转矩形    
    vector<float>solarheight, solarwidth, solarareas;       //会保存在

    //
    int solarnum = 0;

     cout << " 红外组件平均宽高初始化   " << stdRect_w << " " << stdRect_h << " "<<solarnum<<endl;
    InfraSolarPar infrasolarpar;
    int k=0 ;
    for (int i = 0; i < contours.size(); i++)
    {   
        float conarea,fillratio,aspectratio;
        cv::RotatedRect rect = cv::minAreaRect(contours[i]);
        conarea = cv::contourArea(contours[i]);
        fillratio = conarea / rect.size.area();
        solarareas.push_back(rect.size.area());                      
        // rect.size.width += _extra_w;
        // rect.size.height += _extra_h;
        rorated_rectsAll.push_back(rect);
        
        float solarH=0.0, solarW=0.0;
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
        

        if (fillratio>0.8 && aspectratio>1.05 && aspectratio<3.5 && conarea<6000  && outofboundary ==2 )
        {   
            //cout << "solar area   " << conarea << endl;
            k+=1;
            // cout<<"组件框点: "<<k<<" ";
            for (int line_i = 0; line_i < 4; line_i++)
            {   
                
                // cout<< connerP[line_i];
                cv::line(drawmask, connerP[line_i], connerP[(line_i + 1) % 4], cv::Scalar(255, 100, 200), 2);
            }
            // cout<<endl;
            // rect.angle,rect.center,1;
            // cv::getRotationMatrix2D(rect.center,rect.angle,1);
            // cv::warpAffine(drawmask)


            
            //记录组件信息
            infrasolarpar.rotate_rect = rect;  
            infrasolarpar.zc_id = 0;   //默认不在  0表示不在任何一个组串
            //infraZcParas.
            bool  is_inzc= false;

            // cout << " ZC length " << infraZcParas.size() << endl;
            for (int zc_id = 0; zc_id < infraZcParas.size(); zc_id++)
            {   
                

                is_inzc= Isinrotaterect(rect.center, infraZcParas[zc_id].rotate_rect);
                if (is_inzc)
                {   
                    cout << "zai ZC  " << infraZcParas[zc_id].id << " nei" << endl;
                    infrasolarpar.zc_id = infraZcParas[zc_id].id;
                    break;
                }
               
         
            }
            if(is_inzc==false) cv::drawContours(drawmask, contours, i, cv::Scalar(255, 128, 22), 1);
            infraSolarParas.push_back(infrasolarpar);


            stdRect_w += solarW;
            stdRect_h += solarH;
           
            solarnum ++;

        }

       
    }
     cout << " 红外组件宽高和    " << stdRect_w << " " << stdRect_h << " "<<solarnum<<endl;
    stdRect_w = stdRect_w / solarnum;
    stdRect_h = stdRect_h / solarnum;
    //stddelta = stddelta / solarnum;

    cout << " 红外组件平均宽高个数    " << stdRect_w << " " << stdRect_h << " "<<solarnum<<endl;

    //

}



void DetectInfraLogic::Mask2SolarInfox(cv::Mat& Mask,float _Tmaskpara,string _maskname)
{   
    // infraSolarParas.clear();//每张图像需要清除记录
	//用来显示和调试的          
	cv::Mat drawmask= cv::Mat::zeros(cv::Size(Mask.cols, Mask.rows), CV_8UC3);

    
	cv::threshold(Mask, Mask,_Tmaskpara*255,255, cv::THRESH_BINARY);
    cv::Mat element;
    element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    cv::erode(Mask, Mask, element);
    // cv::dilate(Mask, Mask, element);

    

    vector<vector<cv::Point>>contours;
    vector<cv::Vec4i>hierarchy;
    cv::findContours(Mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    vector<cv::Point> hull(contours.size()); //外轮廓
    vector<cv::RotatedRect> rorated_rectsAll, rorated_rectsfilted; //存放旋转矩形    
    vector<float>solarheight, solarwidth, solarareas;       //会保存在

    //
    int solarnum = 0;

     cout << " 红外组件平均宽高初始化   " << stdRect_w << " " << stdRect_h << " "<<solarnum<<endl;
    InfraSolarPar infrasolarpar;
    int k=0 ;
    for (int i = 0; i < contours.size(); i++)
    {   
        float conarea,fillratio,aspectratio;
        cv::RotatedRect rect = cv::minAreaRect(contours[i]);
        conarea = cv::contourArea(contours[i]);
        fillratio = conarea / rect.size.area();
        solarareas.push_back(rect.size.area());                      
        // rect.size.width += _extra_w;
        // rect.size.height += _extra_h;
        rorated_rectsAll.push_back(rect);
        
        float solarH=0.0, solarW=0.0;
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
        

        if (fillratio>0.8 && aspectratio>1.05 && aspectratio<3.5 && conarea<6000  && outofboundary ==2 )
        {   
            //cout << "solar area   " << conarea << endl;
            k+=1;
            // cout<<"组件框点: "<<k<<" ";
            for (int line_i = 0; line_i < 4; line_i++)
            {   
                
                // cout<< connerP[line_i];
                cv::line(drawmask, connerP[line_i], connerP[(line_i + 1) % 4], cv::Scalar(255, 100, 200), 2);
            }
            // cout<<endl;
            // rect.angle,rect.center,1;
            // cv::getRotationMatrix2D(rect.center,rect.angle,1);
            // cv::warpAffine(drawmask)


            
            //记录组件信息
            infrasolarpar.rotate_rect = rect;  
            infrasolarpar.zc_id = 0;   //默认不在  0表示不在任何一个组串
            //infraZcParas.
            bool  is_inzc= false;

            // cout << " ZC length " << infraZcParas.size() << endl;
            for (int zc_id = 0; zc_id < infraZcParas.size(); zc_id++)
            {   
                

                is_inzc= Isinrotaterect(rect.center, infraZcParas[zc_id].rotate_rect);
                if (is_inzc)
                {   
                    // cout << "zai ZC  " << infraZcParas[zc_id].id << " nei" << endl;
                    infrasolarpar.zc_id = infraZcParas[zc_id].id;
                    break;
                }
               
         
            }
            if(is_inzc==false) cv::drawContours(drawmask, contours, i, cv::Scalar(255, 128, 22), 1);
            infraSolarParas.push_back(infrasolarpar);


            stdRect_w += solarW;
            stdRect_h += solarH;
           
            solarnum ++;
          
            cv::imwrite(_maskname,drawmask);

        }

       
    }
     cout << " 红外组件宽高和    " << stdRect_w << " " << stdRect_h << " "<<solarnum<<endl;
    stdRect_w = stdRect_w / solarnum;
    stdRect_h = stdRect_h / solarnum;
    //stddelta = stddelta / solarnum;

    cout << " 红外组件平均宽高个数    " << stdRect_w << " " << stdRect_h << " "<<solarnum<<endl;

    //

}



void DetectInfraLogic::getFaultSpot(cv::Mat _TinputImg,vector<Detectiontype>&detectRes)
{   
//    FaultSpotDeetctsOutputs.clear();

    thermalDetectNetspot.detect(_TinputImg, detectRes, faultTypespot);  //输入尺寸可以在这定也可以在类里面顶                          

    cout<<"新版红外热斑检测故障树: "<<detectRes.size()<<endl;
//    cv::Mat drawimg = _TinputImg.clone();
//    drawresult()
    // thermalDetectNetspot.drawresult(drawimg,FaultSpotDeetctsOutputs);
    
//    cv::imwrite(,drawimg);

   //cout << "show img size  " << drawimg.cols << " " << drawimg.rows << endl;
 
   
}

void DetectInfraLogic::getFaultSolar(cv::Mat _TinputImg,vector<Detectiontype>&detectRes)
{   
//    FaultSpotDeetctsOutputs.clear();

    thermalDetectNetsolar.detect(_TinputImg, detectRes, faultTypesolar);  //输入尺寸可以在这定也可以在类里面顶                          

    cout<<"新版红外故障组件检测故障树: "<<detectRes.size()<<endl;
//    cv::Mat drawimg = _TinputImg.clone();
//    drawresult()
    // thermalDetectNetspot.drawresult(drawimg,FaultSpotDeetctsOutputs);
    
//    cv::imwrite(,drawimg);

   //cout << "show img size  " << drawimg.cols << " " << drawimg.rows << endl;
 
   
}

void DetectInfraLogic::getDCinfo(cv::Mat &_Tinputimg,vector<Detectiontype>&detectRes,string namesave)
{

    cv::Mat DCMask = thermalDcSegNet.detect(_Tinputimg);
    resize(DCMask,DCMask,Size(640,512));

    // cv::imwrite("debug/dcmask/"+namesave+".jpg",DCMask);
    cv::threshold(DCMask, DCMask, 240, 255, cv::THRESH_BINARY);
     cv::imwrite("debug/dcmask/"+namesave+".jpg",DCMask);
    

	std::vector<vector<cv::Point>> DC_contours;
	std::vector<cv::Vec4i> DC_hierarchy;

	cv::findContours(DCMask, DC_contours, DC_hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
	vector<double>DCareas;
	// vector<RotatedRect> DCrects;
    Detectiontype dcres;
	double dcarea;

	if (DC_contours.size() < 1)
	{
		cout << "Not Find  Panels!" << endl;
	}
	else
	{	
        cout << "可能 是掉串 掉串 Panels!" << endl;
		vector<double>DC_area(DC_contours.size());
		for (int i = 0; i < DC_contours.size(); i++)
		{
			dcarea=contourArea(DC_contours[i]);
           
			cv::RotatedRect dcrect = cv::minAreaRect(DC_contours[i]);
			int dc_h = dcrect.boundingRect().height;
			int dc_w = dcrect.boundingRect().width;
			if(dc_h>dc_w){
				int temp_x = dc_w;
				dc_w = dc_h;
				dc_h = temp_x;
			}
			float dc_b = dc_w*0.1/dc_h*10;
			
			double tianchongbi = dcarea/dcrect.size.area();
            cout<<"掉串填充比  "<<tianchongbi<<endl;
			cout<<"掉串面积 "<<dcarea<<endl;
            cout<<"掉串长宽比 "<<dc_b<<endl;
            cout<<"掉串长宽 "<<dc_h<<" "<<dc_w<<endl;
            cout<<"掉串检测结果矩形框："<<	dcres.box<<endl;

			if(tianchongbi>0.85 && dcarea>8000 && dcarea<20000&&dc_b>4 && dc_h<70 ) //需要增加所含组件数目这个条件
			{
                
                // dcres.box = dcrect.boundingRect();
                // dcres.box.x = dcrect.boundingRect().x*2;
                // dcres.box.y = dcrect.boundingRect().y*2;
                // dcres.box.width = dcrect.boundingRect().width*2;
                // dcres.box.height = dcrect.boundingRect().height*2;  //以前错误做法  wzj20230707

                dcres.box.x = dcrect.boundingRect().x*_Tinputimg.cols/640.0;
                dcres.box.y = dcrect.boundingRect().y*_Tinputimg.cols/640.0;
                dcres.box.width = dcrect.boundingRect().width*_Tinputimg.cols/640.0;
                dcres.box.height = dcrect.boundingRect().height*_Tinputimg.cols/640.0;

                dcres.classid = 0;
                dcres.confidence = tianchongbi;
				// DCrects.pu sh_back(dcrect);
                detectRes.push_back(dcres);	
                // cout<<"掉串填充比  "<<tianchongbi<<endl;
			    // cout<<"掉串面积 "<<dcarea<<endl;
                // cout<<"掉串长宽比 "<<dc_b<<endl;
                // cout<<"掉串长宽 "<<dc_h<<" "<<dc_w<<endl;
                // cout<<"掉串检测结果矩形框："<<	dcres.box<<endl;
			
			}

		}
	} 


}

void DetectInfraLogic::writeDrawdetectedImg(cv::Mat _TinputImg,string savepath,vector<Detectiontype>&detectRes,vector<string>class_names)
{
   
    thermalDetectNetspot.drawresult(_TinputImg,detectRes,class_names);
    cv::imwrite(savepath,_TinputImg);
}


void DetectInfraLogic::combineDetectinfoback()  //这个函数可以
{   
    //规则 先判定在哪个组件内，则外框用组件框  ，存储故障四个点，如果不再组件 那么需要判定在哪个组串，
    //不再组串，则放弃  认为不在正常范围内。
    
    //infraSolarParas;
    infraFaultJson infrafaultjson;
 
    // infrafaultinfos.clear();

    //  故障组件的识别

    for (int i = 0; i < FaultSolarDeetctsOutputs.size(); i++)
    {   

        cv::Point2d falutcenter;
        falutcenter.x = int(FaultSolarDeetctsOutputs[i].box.x + FaultSolarDeetctsOutputs[i].box.width / 2);
        falutcenter.y = int(FaultSolarDeetctsOutputs[i].box.y + FaultSolarDeetctsOutputs[i].box.height / 2);
        cout<<" 红外故障组件点坐标 "<<falutcenter<<endl;

        bool isinsolarvector=false;
        // if( FaultSolarDeetctsOutputs[i].box.width < stdRect_w*1.3 &&FaultSolarDeetctsOutputs[i].box.width > stdRect_w*0.8  
        //      &&FaultSolarDeetctsOutputs[i].box.height < stdRect_h*1.3 &&  FaultSolarDeetctsOutputs[i].box.height > stdRect_h*0.8) //新增 20230625 cousin
        // {

        
        for (int j = 0; j < infraSolarParas.size(); j++)  //第一步在solar 分割效果有关
        {
            //infraSolarParas[j].rotate_rect;
            if (Isinrotaterect(falutcenter, infraSolarParas[j].rotate_rect)) 
            {   
                infrafaultjson.ZC_id = infraSolarParas[j].zc_id;  //组串id

                infrafaultjson.ZCquads = infraZcParasmap[infraSolarParas[j].zc_id];

                infrafaultjson.infrasolarquads = infraSolarParas[j].rotate_rect;
                //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id; 
                //这里对应Faulttype[4]
                infrafaultjson.infra_fault_id = faultTypesolar[FaultSolarDeetctsOutputs[i].classid];

                infrafaultjson.confidence = FaultSolarDeetctsOutputs[i].confidence;

                infrafaultjson.infrafaultsolarbox = FaultSolarDeetctsOutputs[i].box;

                infrafaultjson.infrasolarboxarea = FaultSolarDeetctsOutputs[i].box.width * FaultSolarDeetctsOutputs[i].box.height;
                cout << "this faultsolar in solar vector " << endl;

              
                isinsolarvector = true;
                break;  
            }
            // cout << "this faultsolar  not in solar vector " << endl;
        }
        if (isinsolarvector == false)  //第二步 在分割以外的地方  但是在组串内  也可能是遮挡类型和掉串类型
        {
            bool isinzc = false;
            for (auto it = infraZcParasmap.begin(); it != infraZcParasmap.end(); it++)
            {   
                if (Isinrotaterect(falutcenter, it->second)) 
                {
                    infrafaultjson.ZC_id = it->first;  //组串id

                    infrafaultjson.ZCquads = it->second;

                    infrafaultjson.infrasolarquads = boxTorotateRect(FaultSolarDeetctsOutputs[i].box); 

                    //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                    //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id;

                    infrafaultjson.infra_fault_id = faultTypesolar[FaultSolarDeetctsOutputs[i].classid];

                    infrafaultjson.confidence = FaultSolarDeetctsOutputs[i].confidence;

                    infrafaultjson.infrafaultsolarbox = FaultSolarDeetctsOutputs[i].box;

                    infrafaultjson.infrasolarboxarea = FaultSolarDeetctsOutputs[i].box.width * FaultSolarDeetctsOutputs[i].box.height;
                    isinzc = true;
                    cout << "this faultsolar not in solar vector but in ZC"<< it->first << endl;
                    break;
                }
                
                
            }
            
            if (isinzc == false) 
            {
                cout << "this faultsolar not in anythingZC" << endl;   //如果这个模型准确度高 这个可以单独列。
                // continue;
                
                infrafaultjson.ZC_id = 0;  //组串id
                infrafaultjson.infrasolarquads = boxTorotateRect(FaultSolarDeetctsOutputs[i].box);

                //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id;

                infrafaultjson.infra_fault_id = faultTypesolar[FaultSolarDeetctsOutputs[i].classid];

                infrafaultjson.confidence = FaultSolarDeetctsOutputs[i].confidence;

                infrafaultjson.infrafaultsolarbox = FaultSolarDeetctsOutputs[i].box;

                infrafaultjson.infrasolarboxarea = FaultSolarDeetctsOutputs[i].box.width * FaultSolarDeetctsOutputs[i].box.height;
                break;

            }
            
            // } //新增 20230625 cousin

        }
        infrafaultinfos.push_back(infrafaultjson);       //故障组件的添加

    }
    cout<<"红外故障组件数1: "<<infrafaultinfos.size()<<endl;
    // infrafaultinfos.push_back(infrafaultjson);
    //故障热斑的识别

    for (int  i = 0; i < FaultSpotDeetctsOutputs.size(); i++)
    {
        cv::Point2d falutcenter;
        falutcenter.x = int(FaultSpotDeetctsOutputs[i].box.x + FaultSpotDeetctsOutputs[i].box.width / 2);
        falutcenter.y = int(FaultSpotDeetctsOutputs[i].box.y + FaultSpotDeetctsOutputs[i].box.height / 2);
        cout<<" 红外故障热斑点坐标 "<<falutcenter<<endl;
        bool isinsolarvector=false;
        for (int j = 0; j < infraSolarParas.size(); j++)  //第一步在solar 分割效果有关 这个尽量好
        {
                //infraSolarParas[j].rotate_rect;
            if (Isinrotaterect(falutcenter, infraSolarParas[j].rotate_rect)) 
            {   
                infrafaultjson.ZC_id = infraSolarParas[j].zc_id;  //组串id

                infrafaultjson.ZCquads = infraZcParasmap[infraSolarParas[j].zc_id];

                infrafaultjson.infrasolarquads = infraSolarParas[j].rotate_rect;
                    //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                    //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id; 

                infrafaultjson.infra_fault_id = faultTypespot[FaultSpotDeetctsOutputs[i].classid];

                infrafaultjson.confidence = FaultSpotDeetctsOutputs[i].confidence;

                infrafaultjson.infrafaultsolarbox = FaultSpotDeetctsOutputs[i].box;

                infrafaultjson.infrasolarboxarea = FaultSpotDeetctsOutputs[i].box.width * FaultSpotDeetctsOutputs[i].box.height;
                // cout << "this hotspot in solar vector " << endl;
                isinsolarvector = true;
                break;  
            }
            // cout << "this hotspot  not in solar vector " << endl;
        }

        if (isinsolarvector == false)  //第二步 在分割以外的地方  但是在组串内  也可能是遮挡类型和掉串类型
        {
            bool isinzc = false;
            for (auto it = infraZcParasmap.begin(); it != infraZcParasmap.end(); it++)
            {   
                if (Isinrotaterect(falutcenter, it->second)) 
                {
                    infrafaultjson.ZC_id = it->first;  //组串id

                    infrafaultjson.ZCquads = it->second;

                    infrafaultjson.infrasolarquads = boxTorotateRect(FaultSpotDeetctsOutputs[i].box); 

                    //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                    //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id;

                    infrafaultjson.infra_fault_id = faultTypespot[FaultSpotDeetctsOutputs[i].classid];

                    infrafaultjson.confidence = FaultSpotDeetctsOutputs[i].confidence;

                    infrafaultjson.infrafaultsolarbox = FaultSpotDeetctsOutputs[i].box;

                    infrafaultjson.infrasolarboxarea = FaultSpotDeetctsOutputs[i].box.width * FaultSpotDeetctsOutputs[i].box.height;
                    isinzc = true;
                    // cout << "this hotspot not in solar vector but in ZC"<< it->first << endl;
                    break;
                }
                
                
            }
            
            if (isinzc == false) 
            {
                cout << "this hotspot not in anythingZC" << endl;   //如果这个模型准确度高 这个可以单独列。
                continue;
                /*
                infrafaultjson.ZC_id = 0;  //组串id
                infrafaultjson.infrasolarquads = boxTorotateRect(FaultSpotDeetctsOutputs[i].box);

                //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id;

                infrafaultjson.infra_fault_id = faultTypespot[FaultSpotDeetctsOutputs[i].classid];

                infrafaultjson.confidence = FaultSpotDeetctsOutputs[i].confidence;

                infrafaultjson.infrafaultsolarbox = FaultSpotDeetctsOutputs[i].box;

                infrafaultjson.infrasolarboxarea = FaultSpotDeetctsOutputs[i].box.width * FaultSpotDeetctsOutputs[i].box.height;
                */

            }
            


        }

        infrafaultinfos.push_back(infrafaultjson);       //故障热斑的添加


    }
    cout<<"红外故障组件数2: "<<infrafaultinfos.size()<<endl;

    //掉串


    for (int i = 0; i < FaultDCtDeetctsOutputs.size(); i++)
    {
        infrafaultjson.ZC_id = 0;
        infrafaultjson.infrasolarquads = boxTorotateRect(FaultDCtDeetctsOutputs[i].box);
        infrafaultjson.infra_fault_id = "dc";
        infrafaultjson.confidence = FaultDCtDeetctsOutputs[i].confidence;
        infrafaultjson.infrafaultsolarbox = FaultDCtDeetctsOutputs[i].box;
        infrafaultjson.infrasolarboxarea = FaultDCtDeetctsOutputs[i].box.width * FaultDCtDeetctsOutputs[i].box.height;
        infrafaultinfos.push_back(infrafaultjson);  

    }
    cout<<"红外故障组件数3: "<<infrafaultinfos.size()<<endl;
    


}


void DetectInfraLogic::combineDetectinfo()  //这个函数可以
{   
    //规则 先判定在哪个组件内，则外框用组件框  ，存储故障四个点，如果不再组件 那么需要判定在哪个组串，
    //不再组串，则放弃  认为不在正常范围内。
    
    //infraSolarParas;
    infraFaultJson infrafaultjson;
 
    // infrafaultinfos.clear();

    //  故障组件的识别

    for (int i = 0; i < FaultSolarDeetctsOutputs.size(); i++)
    {   

        cv::Point2d falutcenter;
        falutcenter.x = int(FaultSolarDeetctsOutputs[i].box.x + FaultSolarDeetctsOutputs[i].box.width / 2);
        falutcenter.y = int(FaultSolarDeetctsOutputs[i].box.y + FaultSolarDeetctsOutputs[i].box.height / 2);
        cout<<" 红外故障组件点坐标 "<<falutcenter<<endl;

        bool isinsolarvector=false;
        // if( FaultSolarDeetctsOutputs[i].box.width < stdRect_w*1.3 &&FaultSolarDeetctsOutputs[i].box.width > stdRect_w*0.8  
        //      &&FaultSolarDeetctsOutputs[i].box.height < stdRect_h*1.3 &&  FaultSolarDeetctsOutputs[i].box.height > stdRect_h*0.8) //新增 20230625 cousin
        // {

        
        for (int j = 0; j < infraSolarParas.size(); j++)  //第一步在solar 分割效果有关
        {
            //infraSolarParas[j].rotate_rect;
            if (Isinrotaterect(falutcenter, infraSolarParas[j].rotate_rect)) 
            {   
                infrafaultjson.ZC_id = infraSolarParas[j].zc_id;  //组串id

                infrafaultjson.ZCquads = infraZcParasmap[infraSolarParas[j].zc_id];

                infrafaultjson.infrasolarquads = infraSolarParas[j].rotate_rect;
                //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id; 
                //这里对应Faulttype[4]
                infrafaultjson.infra_fault_id = faultTypesolar[FaultSolarDeetctsOutputs[i].classid];

                infrafaultjson.confidence = FaultSolarDeetctsOutputs[i].confidence;

                infrafaultjson.infrafaultsolarbox = FaultSolarDeetctsOutputs[i].box;

                infrafaultjson.infrasolarboxarea = FaultSolarDeetctsOutputs[i].box.width * FaultSolarDeetctsOutputs[i].box.height;
                cout << "this faultsolar in solar vector " << endl;

              
                isinsolarvector = true;
                break;  
            }
            // cout << "this faultsolar  not in solar vector " << endl;
        }
        if (isinsolarvector == false)  //第二步 在分割以外的地方  但是在组串内  也可能是遮挡类型和掉串类型
        {
            bool isinzc = false;
            for (auto it = infraZcParasmap.begin(); it != infraZcParasmap.end(); it++)
            {   
                if (Isinrotaterect(falutcenter, it->second)) 
                {
                    infrafaultjson.ZC_id = it->first;  //组串id

                    infrafaultjson.ZCquads = it->second;

                    infrafaultjson.infrasolarquads = boxTorotateRect(FaultSolarDeetctsOutputs[i].box); 

                    //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                    //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id;

                    infrafaultjson.infra_fault_id = faultTypesolar[FaultSolarDeetctsOutputs[i].classid];

                    infrafaultjson.confidence = FaultSolarDeetctsOutputs[i].confidence;

                    infrafaultjson.infrafaultsolarbox = FaultSolarDeetctsOutputs[i].box;

                    infrafaultjson.infrasolarboxarea = FaultSolarDeetctsOutputs[i].box.width * FaultSolarDeetctsOutputs[i].box.height;
                    isinzc = true;
                    cout << "this faultsolar not in solar vector but in ZC"<< it->first << endl;
                    break;
                }
                
                
            }
            
            if (isinzc == false) 
            {
                cout << "this faultsolar not in anythingZC" << endl;   //如果这个模型准确度高 这个可以单独列。
                // continue;
                
                infrafaultjson.ZC_id = 0;  //组串id
                infrafaultjson.infrasolarquads = boxTorotateRect(FaultSolarDeetctsOutputs[i].box);

                //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id;

                infrafaultjson.infra_fault_id = faultTypesolar[FaultSolarDeetctsOutputs[i].classid];

                infrafaultjson.confidence = FaultSolarDeetctsOutputs[i].confidence;

                infrafaultjson.infrafaultsolarbox = FaultSolarDeetctsOutputs[i].box;

                infrafaultjson.infrasolarboxarea = FaultSolarDeetctsOutputs[i].box.width * FaultSolarDeetctsOutputs[i].box.height;
                break;

            }
            
            // } //新增 20230625 cousin

        }
        infrafaultinfos.push_back(infrafaultjson);       //故障组件的添加

    }
    cout<<"红外故障组件数1: "<<infrafaultinfos.size()<<endl;
    // infrafaultinfos.push_back(infrafaultjson);
    //故障热斑的识别

    for (int  i = 0; i < FaultSpotDeetctsOutputs.size(); i++)
    {
       
                
              
        infrafaultjson.ZC_id = 0;  //组串id
        infrafaultjson.infrasolarquads = boxTorotateRect(FaultSpotDeetctsOutputs[i].box);

                //这一块还需要融合，这里仅仅检测 热斑(可能需要可见识别)、聚集热斑、二极管、组件短路，掉串 不一定检测，
                //infrafaultjson.infra_fault_id = FaultSolarDeetctsOutputs[i].class_id;

        infrafaultjson.infra_fault_id = faultTypespot[FaultSpotDeetctsOutputs[i].classid];

        infrafaultjson.confidence = FaultSpotDeetctsOutputs[i].confidence;

        infrafaultjson.infrafaultsolarbox = FaultSpotDeetctsOutputs[i].box;

        infrafaultjson.infrasolarboxarea = FaultSpotDeetctsOutputs[i].box.width * FaultSpotDeetctsOutputs[i].box.height;
                

            
        infrafaultinfos.push_back(infrafaultjson);       //故障热斑的添加


    }
    cout<<"红外故障组件数2: "<<infrafaultinfos.size()<<endl;

    //掉串


    for (int i = 0; i < FaultDCtDeetctsOutputs.size(); i++)
    {
        infrafaultjson.ZC_id = 0;
        infrafaultjson.infrasolarquads = boxTorotateRect(FaultDCtDeetctsOutputs[i].box);
        infrafaultjson.infra_fault_id = "dc";
        infrafaultjson.confidence = FaultDCtDeetctsOutputs[i].confidence;
        infrafaultjson.infrafaultsolarbox = FaultDCtDeetctsOutputs[i].box;
        infrafaultjson.infrasolarboxarea = FaultDCtDeetctsOutputs[i].box.width * FaultDCtDeetctsOutputs[i].box.height;
        infrafaultinfos.push_back(infrafaultjson);  

    }
    cout<<"红外故障组件数3: "<<infrafaultinfos.size()<<endl;
    


}

void DetectInfraLogic::get_shijiao_img(cv::Mat & _Tinputimg)
{   

    cv::Mat tempimg =_Tinputimg.clone()*0;
    // cv::Mat tempimg =Mat(_Tinputimg.size(), _Tinputimg.type())*0; 

    _Tinputimg(shijiaorect).copyTo(tempimg(shijiaorect));
    _Tinputimg = tempimg.clone();

}


