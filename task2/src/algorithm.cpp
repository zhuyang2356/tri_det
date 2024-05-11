/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-23 12:54:25
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-27 20:57:26
 * @FilePath: /SolarDetect_wzj/lib/src/algorithm.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include<algorithm.h>

#define SNE_PI 3.14159265359

SneDetectAI::SneDetectAI()
{   
    
    // m_z_imgpath = solarinputdata.m_zimagename;
    // m_t_imgpath = solarinputdata.m_timagename; 
    cout<<"*************初始化算法库**********************"<<endl;         //希望是 比如调用红外 可见等？ 或者给出图像型号内参等
    cout<<"*************开始调用算法逻辑引擎***************"<<endl;
    // vector<infraFaultJson> infrapackages();
    // vector<visbleFaultJson> visblepackages;
}

SneDetectAI::~SneDetectAI()
{
    cout<<"*************调用算法逻辑引擎结束***************"<<endl;
}

void SneDetectAI::run(inputdata solarinputdata)
{   

    
    //双线程 识别和分割   希望在分割的时候保证准确率 
    //融合保证有多种方式
    m_dirpath = solarinputdata.dirname;
    m_z_imgpath = solarinputdata.dirname+"/"+solarinputdata.m_zimagename;
    m_t_imgpath = solarinputdata.dirname+"/"+solarinputdata.m_timagename;
    m_z_imgname = solarinputdata.m_zimagename;
    m_t_imgname = solarinputdata.m_timagename;
    outputpath = "../results/pos/";

    cout<<"任务T图像地址："<<m_z_imgpath<<endl;
    cout<<"任务Z图像地址："<<m_t_imgpath<<endl;
    getimginfo(m_t_imgpath, djitimgdata);
    getimginfo(m_z_imgpath, djizimgdata);
    initalgorithm();
    infradetectprocess();
    visbledetectprocss();
    // thread thread_infradetect ( &SneDetectAI::infradetectprocess,this); // 开启线程，调用：thread_infra 红外处理
    // thread thread_visbledetect (&SneDetectAI::visbledetectprocss,this,100); // 开启线程，调用：thread_2(100)

    // thread_infradetect.join(); // pauses until first finishes 这个操作完了之后才能destroyed
    // thread_visbledetect.join(); // pauses until second finishes//join完了之后，才能往下执行。
    cout<<"任务T图像地址："<<m_z_imgpath<<endl;
    // vector<infraFaultJson> infrapackages_tmp;
    // vector<visbleFaultJson> visblepackages_tmp;
    // std::vector<infraFaultJson> infrapackages(std::move(infrapackages_tmp)); // 使用移动构造函数将vec2的所有资源转移给vec3
    // std::vector<visbleFaultJson> visblepackages(std::move(visblepackages_tmp)); // 使用移动构造函数将vec2的所有资源转移给vec3


    // vector<infraFaultJson>().swap(infrapackages);
    cout<<"任务T图像地址："<<m_z_imgpath<<endl;

    // vector<visbleFaultJson>().swap(visblepackages);
    InfraVisbelFusion();
    
}

void SneDetectAI::initalgorithm()
{   
    if(djitimgdata.Cameratype=="M3T")
    {   
        k1 = 0.0;
        k2 = 0.0;
        p1 = 0.0;
        p2 = 1.0-k1-k2-p1;

        superres=false;

        // real_shijiao_x = 0;
        // real_shijiao_y = 0;
        // real_shijiao_w = 640;
        // real_shijiao_h = 512;
        
        cout<<"无人机型号为M3T"<<endl;
    }
    else if (djitimgdata.Cameratype=="M30T")
    {
        k1 = -0.02;
        k2 = 0.02;
        p1 = -0.07;
        p2 = 1.0-k1-k2-p1;

        superres=true;

        // real_shijiao_x = 0;
        // real_shijiao_y = 0;
        // real_shijiao_w = 640;
        // real_shijiao_h = 512;

        cout<<"无人机型号为M30T"<<endl;
    }
    else if (djitimgdata.Cameratype=="ZH20T")
    {
        k1 = 0.0;
        k2 = 0.0;
        p1 = 0.0;
        p2 = 1.0-k1-k2-p1;

        superres=false;

        // real_shijiao_x = 0;
        // real_shijiao_y = 0;
        // real_shijiao_w = 640;
        // real_shijiao_h = 512;

        cout<<"无人机型号为M300"<<endl;
    }
    Zimg = cv::imread(m_z_imgpath);
    Timg = cv::imread(m_t_imgpath); //  inputT

    jibian(Timg,k1,k2,p1,p2);
    
}

 void SneDetectAI::infradetectprocess()
 {
     //*******************红外模块开始*******************//
    cv::Mat shijiaoimg = Timg.clone();

    // detectInfraLogic.infraZcParasmap.clear();
    detectInfraLogic.infraSolarParas.clear();//每张图像需要清除记录
    detectInfraLogic.infrafaultinfos.clear();          //融合信息
    detectInfraLogic.FaultSpotDeetctsOutputs.clear();  //热斑
    detectInfraLogic.FaultSolarDeetctsOutputs.clear(); //故障组件
    detectInfraLogic.FaultDCtDeetctsOutputs.clear(); //故障组件

    cv::Mat drawimg = Timg.clone();

    // 获取mask  wzj20230724   可以根据后面的来弄
    detectInfraLogic.T_Solarmask = detectInfraLogic.getT_SolarMask(Timg);

    // detectInfraLogic.T_Zcmask =  detectInfraLogic.getT_ZcMask(Timg);
            
    if(superres)    //M30T 会超分 同时视野超出    御3可以做到视野一致
    {   
        // cv::rectangle(drawimg,Rect(real_shijiao_x,real_shijiao_y,real_shijiao_w,real_shijiao_h),(0,255,0),2); //标记视野    20230628
        detectInfraLogic.get_shijiao_img(detectInfraLogic.T_Solarmask); //组件视角内
        detectInfraLogic.get_shijiao_img(shijiaoimg); // 视角内识别

        debugwriteimg(shijiaoimg,"debug/tmask/"+string("aaaa_")+m_t_imgname,100 , sne_debug);
        debugwriteimg(detectInfraLogic.T_Solarmask,"debug/tmasku2net/"+string("a_")+m_t_imgname,100 , sne_debug);
            
    }
            
    detectInfraLogic.T_Zcmask =  detectInfraLogic.getT_ZcMask( detectInfraLogic.T_Solarmask);

    debugwriteimg(detectInfraLogic.T_Zcmask,"debug/tmasku2net/"+string("zc_")+m_t_imgname,100 , sne_debug);
  
    string _zcmaskname = "debug/tmasku2net/"+string("zcMask_")+m_t_imgname;
            
    // detectInfraLogic.Mask2ZcInfo( detectInfraLogic.T_Zcmask,0.5);  //组串分割不理想可以屏蔽
    detectInfraLogic.Mask2ZcInfox ( detectInfraLogic.T_Zcmask,0.5,_zcmaskname);  //组串分割不理想可以屏蔽
           
    string _maskname = "debug/tmasku2net/"+m_t_imgname;
    // detectInfraLogic.Mask2SolarInfo(detectInfraLogic.T_Solarmask,0.5);
    detectInfraLogic.Mask2SolarInfox(detectInfraLogic.T_Solarmask,0.5,_maskname);


    //   测试组串标注是否正确
    //   测试id
    //   测试组件分割

    //*********************红外组件故障识别***********************//
    cout<<"红外组件小图数:  "<<detectInfraLogic.infraSolarParas.size()<<endl;
    for(int j =0 ;j<detectInfraLogic.infraSolarParas.size();j++)  //调试所用  可以不需要
    {
        // cv::putText(detectInfraLogic.T_Solarmask, to_string(detectInfraLogic.infraSolarParas[i].zc_id), 
        // detectInfraLogic.infraSolarParas[i].rotate_rect.center, FONT_HERSHEY_COMPLEX, 1, 255, 1, 2, 0);
        cv::putText(detectInfraLogic.T_Solarmask, to_string(j), 
        detectInfraLogic.infraSolarParas[j].rotate_rect.center, FONT_HERSHEY_COMPLEX, 0.5, 255, 1);
                
        cv::RotatedRect rect = detectInfraLogic.infraSolarParas[j].rotate_rect;
        //去除点超出视野外  M300超出视野外,M30T超出视角外
        cv:: Mat cropsolarimg =extractPerspective(drawimg,rect);
        cv::resize(cropsolarimg,cropsolarimg,Size(128,64));
        cv::imwrite("debug/Tcropsolarimg/"+to_string(j)+"_"+m_t_imgname,cropsolarimg);
    } 

            
    detectInfraLogic.getFaultSpot(shijiaoimg,detectInfraLogic.FaultSpotDeetctsOutputs);
    detectInfraLogic.getFaultSolar(shijiaoimg,detectInfraLogic.FaultSolarDeetctsOutputs);
    detectInfraLogic.getDCinfo(shijiaoimg,detectInfraLogic.FaultDCtDeetctsOutputs,m_t_imgname);
            
    //*********************红外故障识别**************************//
            
    debugwriteimg(detectInfraLogic.T_Solarmask,"debug/tmask/"+m_t_imgname,100 , sne_debug);
      
    detectInfraLogic.writeDrawdetectedImg(drawimg,"debug/tmask/"+string("bb_")+m_t_imgname,detectInfraLogic.FaultSpotDeetctsOutputs,detectInfraLogic.faultTypespot);
    detectInfraLogic.writeDrawdetectedImg(drawimg,"debug/Tpicdetect/"+m_t_imgname,detectInfraLogic.FaultSolarDeetctsOutputs,detectInfraLogic.faultTypesolar);

    
    //*********************红外结果融合***********************//
    detectInfraLogic.combineDetectinfo();      //串级  未进行全图和组件去重   


    //*********************红外结果融合***********************//
          
    //*********************红外处理模块结束*********************//   
 }

// int threadID
void SneDetectAI::visbledetectprocss()
 {
     //*******************可见模块开始*******************//
           
    //*********可见初始化清除vector*********************//
    detectVislogic.visbleSolarParas.clear();          //组件参数
    detectVislogic.solardetectres.clear();
    detectVislogic.visblecontoursolar.clear();          //wzj20230711

    detectVislogic.visblefaultinfos.clear();           //融合结果
    detectVislogic.visFaultSolarDetectsOutputs.clear(); //组件识别结果
    detectVislogic.visFaultPicDetectsOutputs.clear();   //全图识别结果
            

    Mat drawZimg = Zimg.clone();
            
    detectVislogic._Z_ZcMask=detectVislogic.getZ_ZcMask(Zimg);           //组串分割不理想可以屏蔽
    detectVislogic._Z_SolarMask=detectVislogic.getZ_SolarMask(Zimg);


    debugwriteimg(detectVislogic._Z_SolarMask,"debug/zmask2solar/"+m_z_imgname,60 , sne_debug);

    detectVislogic.Mask2ZcInfo(detectVislogic._Z_ZcMask,0.5);
    detectVislogic.Mask2SolarInfo(detectVislogic._Z_SolarMask,0.2);

    
    //*********************可见组件识别***********************//
    cout<<"可见光组件小图数:  "<<detectVislogic.visbleSolarParas.size()<<endl;
           
    for (int s = 0; s < detectVislogic.visbleSolarParas.size(); s++)
    {   
        visbleFaultJson solarfult;
        detectVislogic.visFaultSolarDetects.clear();
        cv::RotatedRect rect = detectVislogic.visbleSolarParas[s].rotate_rect;
        cv:: Mat cropsolarimg =extractPerspective(drawZimg,rect);
        detectVislogic.getFaultSolar(cropsolarimg,detectVislogic.visFaultSolarDetects);
        if(detectVislogic.SolarResinfo.classid!=-1)
        {   
            cout<<"小图检测可见光  "<< detectVislogic.SolarResinfo.classid<<endl;

            solarfult.visble_fault_id = detectVislogic.vissolarfaluttype[detectVislogic.SolarResinfo.classid];
            solarfult.ZC_id =  detectVislogic.visbleSolarParas[s].zc_id;
            solarfult.ZCquads = detectVislogic.visbleZcParasmap[solarfult.ZC_id];
            solarfult.visblefaultsolarbox = cv::Rect(rect.boundingRect());
            solarfult.visblesolarquads = rect;
            solarfult.visblesolarboxarea = cv::Rect(rect.boundingRect()).area();

            detectVislogic.solardetectres.push_back(solarfult);
            cv::imwrite("debug/Zfautsolarimg/"+to_string(s)+"_fault_"+m_z_imgname,cropsolarimg);
        }

        cv::imwrite("debug/Zcropsolarimg/"+to_string(s)+"_"+m_z_imgname,cropsolarimg);  //wzj20230711
        Mat cropquadsolarimg = getquadsolarimg(detectVislogic.visblecontoursolar[s],drawZimg);//wzj20230711
        cv::imwrite("debug/Zcropquadsolarimg/"+to_string(s)+"_"+m_z_imgname,cropquadsolarimg);//wzj20230711

        cv::drawContours(drawZimg,detectVislogic.visblecontoursolar,s,cv::Scalar(5,255,0),5);//wzj20230711

        cv::imwrite("debug/zdrawmask/"+m_z_imgname,drawZimg);//wzj20230711


        }
            
            
       
        
        cout<<"新版可见组件识别故障数："<< detectVislogic.visFaultSolarDetectsOutputs.size()<<endl;
        // cout<<"新版可见组件识别故障数："<< detectVislogic.solardetectres.size()<<endl;

        //*********************可见组件识别***********************//
        // cv::imwrite("debug/zdrawmask/"+m_sZoomFileName,detectVislogic.drawmask);
        // cv::imwrite("debug/zdrawmask/"+m_sZoomFileName,detectVislogic._Z_SolarMask);

        //*********************可见全图识别***********************//
        detectVislogic.getFaultPic(drawZimg,detectVislogic.visFaultPicDetectsOutputs);
            

        //*********************可见全图识别***********************//

        //*********************可见结果融合***********************//
        detectVislogic.combineDetectinfo();   //融合组件和 全图识别结果
        //*********************可见结果融合***********************//

        detectVislogic.writeDrawdetectedImg(drawZimg,"debug/Zpicdetect/"+m_z_imgname,detectVislogic.visFaultSolarDetectsOutputs,detectVislogic.vissolarfaluttype);
        detectVislogic.writeDrawdetectedImg(drawZimg,"debug/Zpicdetect/"+m_z_imgname,detectVislogic.visFaultPicDetectsOutputs,detectVislogic.vispicfaluttype);
            
        
        //*******************可见模块结束*******************//
 }

 void SneDetectAI::InfraVisbelFusion()
 {
    //*********************双光融合结果开始***********************//
    // vector<infraFaultJson> infrapackages;
    // vector<visbleFaultJson> visblepackages;
    cout<<"1344"<< infrapackages.size()<<endl;
    //todo 写法危险，待修改------------
    resPackProc.infrafaultinfos=detectInfraLogic.infrafaultinfos;
    resPackProc.visblefaultinfos=detectVislogic.visblefaultinfos;
    //--------------------------------
    cout<<"1xx"<< resPackProc.infrafaultinfos.size()<<"--"<<detectInfraLogic.infrafaultinfos.size()<<endl;
    cout<<"1xx"<< resPackProc.visblefaultinfos.size()<<"--"<<detectVislogic.visblefaultinfos.size()<<endl;

    // infrapackages.clear();
    // cout<<"aaaaa12"<< infrapackages.size()<<endl;

    // visblepackages.clear();
    cout<<"12"<< visblepackages.size()<<endl;

    resPackProc.infrapackages =resPackProc.infrafaultjsonpackage(detectInfraLogic.infrafaultinfos);
    //algorithm规定的字符串，无法直接被赋值，有问题,
    // infrapackages=resPackProc.infrapackages;
    // for (int i = 0; i < resPackProc.infrapackages.size(); i++)
    // {
    //     infrapackages.push_back(resPackProc.infrapackages[i]);
    // }
    
    cout<<"红外打包后长度为"<< infrapackages.size()<<endl;
            
    resPackProc.visblepackages = resPackProc.visblefaultjsonpackage(detectVislogic.visblefaultinfos);
    // visblepackages=resPackProc.visblepackages;algorithm规定的字符串，无法直接被赋值，有问题,
    // for (int i = 0; i < resPackProc.visblepackages.size(); i++)
    // {
    //     visblepackages.push_back(resPackProc.visblepackages[i]);
    // }
    cout<<"可见打包后长度为"<< visblepackages.size()<<endl;
    resPackProc.infrafaultjsonpackageDJIthermal(resPackProc.infrapackages,m_dirpath + "/" + m_t_imgname,superres);


    DJIdata djiTimgdata,djiZimgdata;
    
    getimginfo(m_t_imgpath, djiTimgdata);
    getimginfo(m_z_imgpath, djiZimgdata);

    m_dZx = djiZimgdata.ImgJD;
    m_dZy = djiZimgdata.ImgWD;
    m_ZGimbalPitchDegree = djiZimgdata.GimbalPitch;
    m_ZGimbalYawDegree = djiZimgdata.GimbalYaw;
    m_Zfocus = djiZimgdata.Focallength;
    m_dZheight = djiZimgdata.Flyheight;
            

    m_dTx = djiTimgdata.ImgJD;
    m_dTy = djiTimgdata.ImgWD;
    m_TGimbalPitchDegree = djiTimgdata.GimbalPitch;
    m_TGimbalYawDegree = djiTimgdata.GimbalYaw;
    m_Tfocus = djiTimgdata.Focallength;
    m_dTheight = djiTimgdata.Flyheight;


    // cout<<"红外图像参数: 焦距 , 云台俯仰角 ,云台偏航角，高度，经纬度  "<<m_Tfocus<<" "<<m_TGimbalPitchDegree<<" "<<
    // m_TGimbalYawDegree<<" "<<m_dTheight<<" "<<to_string(m_dTx)<<" "<<to_string(m_dTy)<<endl;

    // cout<<"可见图像参数: 焦距 , 云台俯仰角 ,云台偏航角，高度，经纬度  "<<m_Zfocus<<" "<<m_ZGimbalPitchDegree<<" "<<
    //  m_ZGimbalYawDegree<<" "<<m_dZheight<<" "<<to_string(m_dZx)<<" "<<to_string(m_dZy)<<endl;
    cout<<"pox48"<<endl;
    // infrapackages=resPackProc.infrapackages;
    cout<<"pox46"<< infrapackages.size()<<endl;
    // visblepackages=resPackProc.visblepackages;
    cout<<"pox45"<< visblepackages.size()<<endl;
    gpsInfoAssemble(resPackProc.infrapackages,resPackProc.visblepackages);
    // for (int i = 0; i < infrapackages.size(); i++)
    // {
    //     cout<<"pox32"<<endl;

    //     cout<<"红外组件最高温度："<<infrapackages[i].soalrmaxTemptrue<<endl;
    //     cout<<"红外组件平均温度："<<infrapackages[i].soalrmeanTemptrue<<endl;
    //     double pointx ,pointy ;
    //     double jd,wd;
    //     pointx = infrapackages[i].infrafaultsolarbox.x+infrapackages[i].infrafaultsolarbox.width/2 - Timg.cols/2;
    //     pointy =Timg.rows/2 -( infrapackages[i].infrafaultsolarbox.y+infrapackages[i].infrafaultsolarbox.height/2);

    //     float chipsize_w=7.68;  //红外尺寸
    
    //     m_tBlc =detectInfraLogic.stdRect_h;
    //     cout<<" 红外比例尺"<<m_tBlc<<endl;
    //     GetFaultPointGpsInfo(pointx,pointy,jd,wd,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);
    //     cout<<"红外组件经纬度："<<to_string(pointx)<<" "<<to_string(pointy)<<" "<<to_string(jd)<<" "<<to_string(wd)<<endl;
    //     infrapackages[i].infrasolarcenterEN = cv::Point2d(jd,wd);
                
    //     cv::Point lu,ru,ld,rd;
    //     cv::Point2d lu_EN,ru_EN,ld_EN,rd_EN;
    //     lu.x = infrapackages[i].infrafaultsolarbox.x - Timg.cols/2;
    //     lu.y = Timg.rows/2 - infrapackages[i].infrafaultsolarbox.y;

    //     GetFaultPointGpsInfo(lu.x,lu.y,lu_EN.x,lu_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

    //     ru.x = infrapackages[i].infrafaultsolarbox.x +  infrapackages[i].infrafaultsolarbox.width - Timg.cols/2;
    //     ru.y = Timg.rows/2 - infrapackages[i].infrafaultsolarbox.y;

    //     GetFaultPointGpsInfo(ru.x,ru.y,ru_EN.x,ru_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

    //     ld.x = infrapackages[i].infrafaultsolarbox.x - Timg.cols/2;
    //     ld.y = Timg.rows/2 - (infrapackages[i].infrafaultsolarbox.y+ infrapackages[i].infrafaultsolarbox.height);

    //     GetFaultPointGpsInfo(ld.x,ld.y,ld_EN.x,ld_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

    //     rd.x = infrapackages[i].infrafaultsolarbox.x +  infrapackages[i].infrafaultsolarbox.width - Timg.cols/2;
    //     rd.y = Timg.rows/2 - (infrapackages[i].infrafaultsolarbox.y+ infrapackages[i].infrafaultsolarbox.height);
                
    //     GetFaultPointGpsInfo(rd.x,rd.y,rd_EN.x,rd_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

    //     infrapackages[i].infrasolarpointEN.push_back(lu_EN);
    //     infrapackages[i].infrasolarpointEN.push_back(ru_EN);
    //     infrapackages[i].infrasolarpointEN.push_back(ld_EN);
    //     infrapackages[i].infrasolarpointEN.push_back(rd_EN);
    //     cout<<"红外四个点的经纬度："<<lu_EN<<" "<<ru_EN<<" "<<ld_EN<<" "<<ld_EN<<endl;

    // }

    // for (int i = 0; i < visblepackages.size(); i++)
    // {
    //     double pointx ,pointy ;
    //     double jd,wd;

    //     float chipsize_w=6.4; //御三 M30T M300都是6.4 但是 30T是无法变焦的 因此他的焦距需要×2 这个是根据红外的参数而来  奇怪吧。
        
    //     m_zBlc =detectVislogic.stdRect_h;
    //     cout<<" 可见比例尺"<<m_zBlc<<endl;
    //     pointx = visblepackages[i].visblefaultsolarbox.x +visblepackages[i].visblefaultsolarbox.width/2 - Zimg.cols/2;
    //     pointy = Zimg.rows/2 - (visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height/2) ;
                
    //     if(superres)
    //     {
    //         m_Zfocus*=2; //但是 30T是无法变焦的 因此他的焦距需要×2 这个是根据红外的参数而来  奇怪吧。
    //     }
    //     GetFaultPointGpsInfo(pointx,pointy,jd,wd,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);
    //     cout<<"可见组件经纬度："<<pointx<<" "<<pointy<<" "<<jd<<" "<<wd<<endl;
    //     visblepackages[i].visblesolarcenterEN =cv::Point2d (jd,wd);

    //     cv::Point lu,ru,ld,rd;
    //     cv::Point2d lu_EN,ru_EN,ld_EN,rd_EN;

    //     lu.x = visblepackages[i].visblefaultsolarbox.x - Zimg.cols/2;
    //     lu.y = Zimg.rows/2 - visblepackages[i].visblefaultsolarbox.y;

    //     GetFaultPointGpsInfo(lu.x,lu.y,lu_EN.x,lu_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

    //     ru.x = visblepackages[i].visblefaultsolarbox.x+visblepackages[i].visblefaultsolarbox.width - Zimg.cols/2;
    //     ru.y = Zimg.rows/2 - visblepackages[i].visblefaultsolarbox.y;

    //     GetFaultPointGpsInfo(ru.x,ru.y,ru_EN.x,ru_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

    //     ld.x = visblepackages[i].visblefaultsolarbox.x - Zimg.cols/2;
    //     ld.y = Zimg.rows/2 - (visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height);

    //     GetFaultPointGpsInfo(ld.x,ld.y,ld_EN.x,ld_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

    //     rd.x = visblepackages[i].visblefaultsolarbox.x +visblepackages[i].visblefaultsolarbox.width - Zimg.cols/2;
    //     rd.y = Zimg.rows/2 - (visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height);

    //     GetFaultPointGpsInfo(rd.x,rd.y,rd_EN.x,rd_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

    //     visblepackages[i].visblesolarpointEN.push_back(lu_EN);
    //     visblepackages[i].visblesolarpointEN.push_back(ru_EN);
    //     visblepackages[i].visblesolarpointEN.push_back(ld_EN);
    //     visblepackages[i].visblesolarpointEN.push_back(rd_EN);
    //     cout<<"可见四个点的经纬度："<<lu_EN<<" "<<ru_EN<<" "<<ld_EN<<" "<<ld_EN<<endl;

    // }
                    
    //************加入温度数据和故障定位*******************//

    //**********融合重新构建红外和可见的合并的**************//      方法A

            
    //*********对含有公共部分取conmonNum*****************//
    cout<<"进行红外和可见配对"<<endl;
    resPackProc.VisbleInfraCombine(resPackProc.infrapackages,resPackProc.visblepackages,superres);

    //*********对含有公共部分取conmonNum*****************//

    //**********融合重新构建红外和可见的合并的**************//
 }

void SneDetectAI::SolarJsonFusionFGX(string &RESjson ,string faultimg_pos)
{
    resPackProc.JsonPackagesComine(RESjson,resPackProc.infrapackages,resPackProc.visblepackages,Timg,Zimg);
    debugwriteimg(Timg,faultimg_pos+m_t_imgname,100,sne_debug);
    debugwriteimg(Zimg,faultimg_pos+m_z_imgname,100,sne_debug);
}


void SneDetectAI::SolarJsonFusionIVCV(string &RESjson)
{   
    cout<<"RESJSON打包前："<<RESjson<<endl;
    // JsonPackagesComineYS_split(RESjson,infrapackages,visblepackages,Timg,Zimg);
    resPackProc.JsonPackagesComineYS_merge(RESjson,infrapackages,visblepackages,Timg,Zimg);
    cout<<"RESJSON打包后："<<RESjson<<endl;
}

void SneDetectAI::SolarIvCvReProcess(bool DEBUG_DRAW)
{
    
    cv::Mat _T_Solar_img ,_Z_Solar_img;
    vector<string > falutnumid; //用来计数用的
               
    if(infrapackages.size()>0|| visblepackages.size()>0)
    {
                    
        cv::Mat debugTimgDraw,debugZimgDraw;
        debugTimgDraw = Timg.clone();
        debugZimgDraw = Zimg.clone();
        if(DEBUG_DRAW)
        {   
            //标注红外框
                        
            for (int i = 0; i < infrapackages.size(); i++)
            {   
                cv::rectangle(debugTimgDraw,infrapackages[i].infrafaultsolarbox,cv::Scalar(255,0,0),2);
                if(infrapackages[i].commonfaultid.empty())
                {   
                    falutnumid.push_back(infrapackages[i].infra_fault_id);  //, FONT_HERSHEY_COMPLEX, 2, 255, 2
                     int faultID = count(falutnumid.begin(), falutnumid.end(), infrapackages[i].infra_fault_id);
                                
                    cv::putText(debugTimgDraw, infrapackages[i].infra_fault_id+"-"+to_string(faultID),
                    cv::Point(int(infrapackages[i].infrafaultsolarbox.x+infrapackages[i].infrafaultsolarbox.width/2),
                    int(infrapackages[i].infrafaultsolarbox.y+infrapackages[i].infrafaultsolarbox.height/2)), FONT_HERSHEY_COMPLEX, 0.5, 255, 1);
                                
                }
                            
                
            }

            for (int i = 0; i < visblepackages.size(); i++)
            {
                cv::rectangle(debugZimgDraw,visblepackages[i].visblefaultsolarbox,cv::Scalar(255,0,0),8);
                if(visblepackages[i].commonfaultid.empty())
                { 
                                
                    falutnumid.push_back(visblepackages[i].visble_fault_id);  //, FONT_HERSHEY_COMPLEX, 2, 255, 2
                    int faultID = count(falutnumid.begin(), falutnumid.end(), visblepackages[i].visble_fault_id);
                    cv::putText(debugZimgDraw, visblepackages[i].visble_fault_id+"-"+to_string(faultID),
                    cv::Point(int(visblepackages[i].visblefaultsolarbox.x+visblepackages[i].visblefaultsolarbox.width/2),
                    int(visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height/2)), FONT_HERSHEY_COMPLEX, 2, 255, 5);
                                
                }
                            
                else
                {
                    falutnumid.push_back(visblepackages[i].visble_fault_id);  //, FONT_HERSHEY_COMPLEX, 2, 255, 2
                    int faultID = count(falutnumid.begin(), falutnumid.end(), visblepackages[i].visble_fault_id);

                    for (int j = 0; j < infrapackages.size(); j++)
                    {
                        if(visblepackages[i].commonfaultid==infrapackages[j].commonfaultid)
                        {

                            cv::putText(debugTimgDraw, visblepackages[i].visble_fault_id+"-"+to_string(faultID),
                            cv::Point(int(infrapackages[j].infrafaultsolarbox.x+infrapackages[j].infrafaultsolarbox.width/2),
                            int(infrapackages[j].infrafaultsolarbox.y+infrapackages[j].infrafaultsolarbox.height/2)), FONT_HERSHEY_COMPLEX, 0.5, (20,255,0), 1);


                            cv::putText(debugZimgDraw, visblepackages[i].visble_fault_id+"-"+to_string(faultID),
                            cv::Point(int(visblepackages[i].visblefaultsolarbox.x+visblepackages[i].visblefaultsolarbox.width/2),
                                int(visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height/2)), FONT_HERSHEY_COMPLEX, 2, (20,255,0), 5);
                        }
                    }
                                    
                }

            }
                        
                        
        }

        if(superres)
        {
            _T_Solar_img=  debugTimgDraw(Rect(real_shijiao_x,real_shijiao_y,real_shijiao_w,real_shijiao_h)) ;
        }
        else
        {
           _T_Solar_img = debugTimgDraw.clone();
                        
        }
                    
        // cv::circle(debugZimgDraw,cv::Point(debugZimgDraw.cols/2,debugZimgDraw.rows/2),10,cv::Scalar(0,255,0),5);
           
        // SouthAndNorth(Mat debugZimgDraw,double m_ZGimbalYawDegree);
        // cout<<"测试保存图像位置 "<<" "<<endl;
        debugwriteimg(_T_Solar_img,outputpath+m_t_imgname,60 , true);
        debugwriteimg(debugZimgDraw,outputpath+m_z_imgname,60 , true);

    
    }
             
            
}

void SneDetectAI::GetFaultPointGpsInfo(double pointx ,double pointy ,double & jd,double &wd , int img_width,float focus ,double centerE,double centerN,float chipsize_w  )
{
    
    //cousin 20230609 修改 验证多次 
    double point_to_N = pointy * cos(m_TGimbalYawDegree/ 180 * SNE_PI) - pointx * sin(m_TGimbalYawDegree/ 180 * SNE_PI);
	double point_to_E = pointy * sin(m_TGimbalYawDegree/ 180 * SNE_PI) + pointx * cos(m_TGimbalYawDegree/ 180 * SNE_PI); 
    //cousin 20230609 修改 验证多次 

    // cout<<"N像素偏差"<<point_to_N<<" "<<"E像素偏差"<<point_to_E<<endl;
    // cout<<"计算过程: "<< pointy * cos(m_TGimbalYawDegree/ 180 * SNE_PI)<<endl;
    // cout<<"计算过程: "<<  pointx * sin(m_TGimbalYawDegree/ 180 * SNE_PI)<<endl;

    // cout<<"计算过程: "<< pointy * sin(m_TGimbalYawDegree/ 180 * SNE_PI) <<endl;
    // cout<<"计算过程: "<< pointx * cos(m_TGimbalYawDegree/ 180 * SNE_PI)<<endl;

    cout<<"N E像素偏差"<<point_to_N<<" "<< point_to_E<<"  "<<"原像素偏差"<<pointx<<" "<<pointy <<endl;
 
	double shijiao_w = m_dTheight /focus * chipsize_w ;

    double shijiao_w_pix =0.97 / m_zBlc ;   //需要优化
    
    double shijiao_w_pix_gps= shijiao_w / img_width ;
    
    cout<<"比例尺"<<m_zBlc<<" "<<shijiao_w<<" "<<shijiao_w_pix<<endl;

    if(m_zBlc == 0)
    {
        shijiao_w_pix= shijiao_w / img_width ;
        cout<<"9999比例尺"<<m_zBlc<<endl;
    }
	double delta_E_shijiao = shijiao_w_pix * point_to_E ;
	double delta_N_shijiao = shijiao_w_pix * point_to_N ;

    double delta_E_shijiao_gps = shijiao_w_pix_gps * point_to_E ;
	double delta_N_shijiao_gps = shijiao_w_pix_gps * point_to_N ;

    double delta_N_gps = delta_N_shijiao_gps /111322 +centerN;
	double delta_E_gps = delta_E_shijiao_gps /110933/ cos(m_dTy / 180 * SNE_PI)+centerE;

    // cout<<"视角度值"<<shijiao_w_pix<<" "<<"视角精度维度值"<<delta_E_shijiao<<" "<<delta_N_shijiao<<endl;
    cout<<"像素分辨率对比: "<<"比例尺： "<<shijiao_w_pix<<" 视角法： "<<shijiao_w / img_width <<endl;

    double delta_N = delta_N_shijiao /111322 ;
	double delta_E = delta_E_shijiao /110933/ cos(m_dTy / 180 * SNE_PI);

	// wd = delta_N + centerN;
	// jd = delta_E + centerE;

    wd = delta_N_gps ;
	jd = delta_E_gps ;
	
    cout<<"N偏差"<<delta_N<<" "<<"E偏差"<<delta_E<<endl;
	// cout << "俯仰角:" << m_TGimbalPitchDegree << " "<< "高度:" << m_dTheight << endl;
	printf(" 图片经度:%11f\t 图片纬度:%11f\n 比例尺组件经度:%f\t 比例尺组件纬度:%f\n 视角组件经度:%f\t 视角组件纬度:%f\n", m_dTx, m_dTy, delta_E_gps, delta_N + centerN,delta_E_gps,delta_N_gps);
}

void SneDetectAI::gpsInfoAssemble(vector<infraFaultJson> &infrapackages,vector<visbleFaultJson> &visblepackages){
    for (int i = 0; i < infrapackages.size(); i++)
    {
        cout<<"pox32"<<endl;

        cout<<"红外组件最高温度："<<infrapackages[i].soalrmaxTemptrue<<endl;
        cout<<"红外组件平均温度："<<infrapackages[i].soalrmeanTemptrue<<endl;
        double pointx ,pointy ;
        double jd,wd;
        pointx = infrapackages[i].infrafaultsolarbox.x+infrapackages[i].infrafaultsolarbox.width/2 - Timg.cols/2;
        pointy =Timg.rows/2 -( infrapackages[i].infrafaultsolarbox.y+infrapackages[i].infrafaultsolarbox.height/2);

        float chipsize_w=7.68;  //红外尺寸
    
        m_tBlc =detectInfraLogic.stdRect_h;
        cout<<" 红外比例尺"<<m_tBlc<<endl;
        GetFaultPointGpsInfo(pointx,pointy,jd,wd,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);
        cout<<"红外组件经纬度："<<to_string(pointx)<<" "<<to_string(pointy)<<" "<<to_string(jd)<<" "<<to_string(wd)<<endl;
        infrapackages[i].infrasolarcenterEN = cv::Point2d(jd,wd);
                
        cv::Point lu,ru,ld,rd;
        cv::Point2d lu_EN,ru_EN,ld_EN,rd_EN;
        lu.x = infrapackages[i].infrafaultsolarbox.x - Timg.cols/2;
        lu.y = Timg.rows/2 - infrapackages[i].infrafaultsolarbox.y;

        GetFaultPointGpsInfo(lu.x,lu.y,lu_EN.x,lu_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

        ru.x = infrapackages[i].infrafaultsolarbox.x +  infrapackages[i].infrafaultsolarbox.width - Timg.cols/2;
        ru.y = Timg.rows/2 - infrapackages[i].infrafaultsolarbox.y;

        GetFaultPointGpsInfo(ru.x,ru.y,ru_EN.x,ru_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

        ld.x = infrapackages[i].infrafaultsolarbox.x - Timg.cols/2;
        ld.y = Timg.rows/2 - (infrapackages[i].infrafaultsolarbox.y+ infrapackages[i].infrafaultsolarbox.height);

        GetFaultPointGpsInfo(ld.x,ld.y,ld_EN.x,ld_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

        rd.x = infrapackages[i].infrafaultsolarbox.x +  infrapackages[i].infrafaultsolarbox.width - Timg.cols/2;
        rd.y = Timg.rows/2 - (infrapackages[i].infrafaultsolarbox.y+ infrapackages[i].infrafaultsolarbox.height);
                
        GetFaultPointGpsInfo(rd.x,rd.y,rd_EN.x,rd_EN.y,Timg.cols,m_Tfocus,m_dTx,m_dTy,chipsize_w);

        infrapackages[i].infrasolarpointEN.push_back(lu_EN);
        infrapackages[i].infrasolarpointEN.push_back(ru_EN);
        infrapackages[i].infrasolarpointEN.push_back(ld_EN);
        infrapackages[i].infrasolarpointEN.push_back(rd_EN);
        cout<<"红外四个点的经纬度："<<lu_EN<<" "<<ru_EN<<" "<<ld_EN<<" "<<ld_EN<<endl;

    }

    for (int i = 0; i < visblepackages.size(); i++)
    {
        double pointx ,pointy ;
        double jd,wd;

        float chipsize_w=6.4; //御三 M30T M300都是6.4 但是 30T是无法变焦的 因此他的焦距需要×2 这个是根据红外的参数而来  奇怪吧。
        
        m_zBlc =detectVislogic.stdRect_h;
        cout<<" 可见比例尺"<<m_zBlc<<endl;
        pointx = visblepackages[i].visblefaultsolarbox.x +visblepackages[i].visblefaultsolarbox.width/2 - Zimg.cols/2;
        pointy = Zimg.rows/2 - (visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height/2) ;
                
        if(superres)
        {
            m_Zfocus*=2; //但是 30T是无法变焦的 因此他的焦距需要×2 这个是根据红外的参数而来  奇怪吧。
        }
        GetFaultPointGpsInfo(pointx,pointy,jd,wd,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);
        cout<<"可见组件经纬度："<<pointx<<" "<<pointy<<" "<<jd<<" "<<wd<<endl;
        visblepackages[i].visblesolarcenterEN =cv::Point2d (jd,wd);

        cv::Point lu,ru,ld,rd;
        cv::Point2d lu_EN,ru_EN,ld_EN,rd_EN;

        lu.x = visblepackages[i].visblefaultsolarbox.x - Zimg.cols/2;
        lu.y = Zimg.rows/2 - visblepackages[i].visblefaultsolarbox.y;

        GetFaultPointGpsInfo(lu.x,lu.y,lu_EN.x,lu_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

        ru.x = visblepackages[i].visblefaultsolarbox.x+visblepackages[i].visblefaultsolarbox.width - Zimg.cols/2;
        ru.y = Zimg.rows/2 - visblepackages[i].visblefaultsolarbox.y;

        GetFaultPointGpsInfo(ru.x,ru.y,ru_EN.x,ru_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

        ld.x = visblepackages[i].visblefaultsolarbox.x - Zimg.cols/2;
        ld.y = Zimg.rows/2 - (visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height);

        GetFaultPointGpsInfo(ld.x,ld.y,ld_EN.x,ld_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

        rd.x = visblepackages[i].visblefaultsolarbox.x +visblepackages[i].visblefaultsolarbox.width - Zimg.cols/2;
        rd.y = Zimg.rows/2 - (visblepackages[i].visblefaultsolarbox.y+visblepackages[i].visblefaultsolarbox.height);

        GetFaultPointGpsInfo(rd.x,rd.y,rd_EN.x,rd_EN.y,Zimg.cols,m_Zfocus,m_dZx,m_dZy,chipsize_w);

        visblepackages[i].visblesolarpointEN.push_back(lu_EN);
        visblepackages[i].visblesolarpointEN.push_back(ru_EN);
        visblepackages[i].visblesolarpointEN.push_back(ld_EN);
        visblepackages[i].visblesolarpointEN.push_back(rd_EN);
        cout<<"可见四个点的经纬度："<<lu_EN<<" "<<ru_EN<<" "<<ld_EN<<" "<<ld_EN<<endl;

    }
}
