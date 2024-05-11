#include <resultPackage.h>

resPack::resPack()
{
    cout<<"respack"<<endl;
}
resPack::~resPack()
{
    cout<<"respackxigou"<<endl;
}
vector<infraFaultJson> resPack::infrafaultjsonpackage(vector<infraFaultJson> infrafaultinfos)
{
    vector<infraFaultJson> infrapackages;
    vector<infraFaultJson> Spotpackages;
    vector<infraFaultJson> Solarpackages;
    cout<<"1112"<< infrapackages.size()<<endl;

   /*
    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
        //处理逻辑   主要是组件重合 结果不同  iou部分为面积小者的占比计算 取solar_utils 中getboxiouratio使用
        //是否重合//
        for(int j = 0; j < infrapackages.size(); j++)
        {
            if(getboxiouratio(infrafaultinfos[i].infrafaultsolarbox,  infrapackages[j].infrafaultsolarbox   )>0.8)
            {
                //说明重合了
            }



        }
        infrapackages.push_back(infrafaultinfos[i]);

    }*/

    //**********将热斑以外的全部放入A， 热斑全部放入B,  在对B进行和A 取交集************//
    cout<<"1a2"<< infrapackages.size()<<endl;

    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
        if(infrafaultinfos[i].infra_fault_id=="Spot")
        {
            Spotpackages.push_back(infrafaultinfos[i]);
    
        }
        else
        {
            Solarpackages.push_back(infrafaultinfos[i]);
        }
                
    }
    cout<<"a12"<< infrapackages.size()<<endl;

    for (int i = 0; i < Spotpackages.size(); i++)
    {   
        bool chonghe = false;
        for(int j = 0; j < Solarpackages.size(); j++)
        {
            if(getboxiouratio(Spotpackages[i].infrafaultsolarbox,Solarpackages[j].infrafaultsolarbox)>0.8)
            {
                chonghe=true;
                break;
            }
        }
        if (chonghe==false)
        {
            infrapackages.push_back(Spotpackages[i]);
        }

    }
    cout<<"123456"<< infrapackages.size()<<endl;

    for (int j = 0; j < Solarpackages.size(); j++)
    {
        infrapackages.push_back(Solarpackages[j]);
    }
    
    cout<<"x12"<< infrapackages.size()<<endl;

    return infrapackages;

}


vector<visbleFaultJson> resPack::visblefaultjsonpackage(vector<visbleFaultJson> visblefaultinfos)
{
    cout<<"1112"<<endl;
    vector<visbleFaultJson> visblepackages;
    vector<visbleFaultJson> Picpackages;
    cout<<"1111112"<<endl;

    vector<visbleFaultJson> Solarpackages;
   /*
    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
        //处理逻辑   主要是组件重合 结果不同  iou部分为面积小者的占比计算 取solar_utils 中getboxiouratio使用
        //是否重合//
        for(int j = 0; j < infrapackages.size(); j++)
        {
            if(getboxiouratio(infrafaultinfos[i].infrafaultsolarbox,  infrapackages[j].infrafaultsolarbox   )>0.8)
            {
                //说明重合了
            }



        }
        infrapackages.push_back(infrafaultinfos[i]);

    }*/

    //**********将组件以内的全部放入A， 全图全部放入B,  在对B进行和A 取交集************//
    vector<string> vissolarfaluttype = { "zd_nf","zd_hc","zd_yw","sl" };  
    for (int i = 0; i < visblefaultinfos.size(); i++)
    {   
        if(find(vissolarfaluttype.begin(),vissolarfaluttype.end(),visblefaultinfos[i].visble_fault_id)!=vissolarfaluttype.end())
        // if(visblefaultinfos[i].visble_fault_id=="Spot")
        {
            Solarpackages.push_back(visblefaultinfos[i]);
    
        }
        else
        {
            Picpackages.push_back(visblefaultinfos[i]);
        }
                
    }
    cout<<"111233331112"<<endl;

    for (int i = 0; i < Solarpackages.size(); i++)
    {   
        bool chonghe = false;
        for(int j = 0; j < Picpackages.size(); j++)
        {
            if(getboxiouratio(Solarpackages[i].visblefaultsolarbox,Picpackages[j].visblefaultsolarbox)>0.8)
            {
                chonghe=true;
                break;
            }
        }
        if (chonghe==false)
        {
            visblepackages.push_back(Solarpackages[i]);
        }

    }
    for (int j = 0; j < Picpackages.size(); j++)
    {
        visblepackages.push_back(Picpackages[j]);
    }
    cout<<"99998331112"<<endl;

    
    return visblepackages;

}  

void resPack::infrafaultjsonpackageDJIthermal(vector<infraFaultJson>& infrafaultinfos,string Timgpath,bool superres) 
{
    Read_Dji_Thermal m_RDThermal(Timgpath);
    m_RDThermal.getDjiRawData();
    int k =1;
    if(superres){
        k=2;
    }

    for (int i = 0; i < infrafaultinfos.size(); i++)
    {   
        float  spotmaxtemp=0,spotmeantemp=0;
        long spottemp=0;
        long long sumspottemp=0;
        // infrafaultinfos[i].infrafaultsolarbox;
        for (int w = 0; w < infrafaultinfos[i].infrafaultsolarbox.width /k ; w++)
        {
            for (int h = 0; h < infrafaultinfos[i].infrafaultsolarbox.height/k; h++)
            {
                m_RDThermal.getPointTemperature(w+infrafaultinfos[i].infrafaultsolarbox.x/k,h+infrafaultinfos[i].infrafaultsolarbox.y/k,spottemp);
                // cout<<"测试温度"<<spottemp<<endl;
                if(spottemp>30000)
                    continue;
                sumspottemp+=spottemp;
                if(spottemp>spotmaxtemp) spotmaxtemp=spottemp;
            }


        }
        
        spotmeantemp = sumspottemp*0.1/infrafaultinfos[i].infrafaultsolarbox.width/infrafaultinfos[i].infrafaultsolarbox.height*k*k ;
        spotmaxtemp =spotmaxtemp*0.1;
        
        // infrafaultinfos[i].soalrmeanTemptrue =to_string(spotmeantemp);
        // infrafaultinfos[i].soalrmaxTemptrue = to_string(spotmaxtemp);
         infrafaultinfos[i].soalrmeanTemptrue =spotmeantemp;
        infrafaultinfos[i].soalrmaxTemptrue = spotmaxtemp;
        // cout<<"测试最高恩度和平均温度温度 "<<to_string(spotmeantemp)<<"  "<< to_string(spotmaxtemp)<<"  "<<to_string(sumspottemp) <<endl;
        
        /*
        if(int(spotmaxtemp) == 0)
        {
            cout<<"最大温度为零的情形: "<<infrafaultinfos[i].infrafaultsolarbox<<endl;
        }
        else
        {
            cout<<"最大温度不为零的情形: "<<infrafaultinfos[i].infrafaultsolarbox<<endl;
        } */
        
    }
    /*
    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
         cout<<"循环测试最高温度度和平均温度温度 "<<infrafaultinfos[i].soalrmaxTemptrue<<" "<<infrafaultinfos[i].soalrmeanTemptrue<<endl;
    }
    */
    
    

}

void resPack::VisbleInfraCombine(vector<infraFaultJson> &infrafaultinfos,vector<visbleFaultJson> &visblefaultinfos,bool superres)
{
    //在infra种热斑 寻找和可见相同点的
    //方法1.可以根据红外图像和可见图像 infrasolarbox 和 visblesolarbox 点求iou进行匹配  ，也可以联系图像配准融合
        //方法2.可以根据红外可见中的经纬度 infrasolarpointEN 和 visblesolarpointEN  进行匹配
        //按照方法2来
    int commonNum=0;
    int func=1;
    float bit= 3888.0/512;
    switch (func) 
    {
    case 1:
        if(superres)
        {
            bit=4.0;
        }         //红外为1000*750 
        for (int i = 0; i < infrafaultinfos.size(); i++)
        {
            if(infrafaultinfos[i].infra_fault_id=="Spot")
            {
                cout<<"热斑配对"<<endl;
                for (int j = 0; j < visblefaultinfos.size(); j++)
                {
                    cv::Rect infrabox,visblebox;
                    infrabox.x = (infrafaultinfos[i].infrafaultsolarbox.x-140)*bit;
                    infrabox.y = (infrafaultinfos[i].infrafaultsolarbox.y-128)*bit;

                    infrabox.width = infrafaultinfos[i].infrafaultsolarbox.width*bit ;
                    infrabox.height = infrafaultinfos[i].infrafaultsolarbox.height*bit;

                    visblebox.x = visblefaultinfos[j].visblefaultsolarbox.x;
                    visblebox.y = visblefaultinfos[j].visblefaultsolarbox.y;

                    visblebox.width = visblefaultinfos[j].visblefaultsolarbox.width;
                    visblebox.height = visblefaultinfos[j].visblefaultsolarbox.height;


                    cout<<"红外和可见配对矩形框点： "<<infrabox<<" "<<visblebox<<endl;
                    if(getboxiouratio(infrabox,visblebox)>0.5)
                    {   
                        cout<<"红外组件和可见有相同   "<<infrafaultinfos[i].commonfaultid<<"  "<<visblefaultinfos[j].commonfaultid<<endl;
                        commonNum +=1;
                        infrafaultinfos[i].commonfaultid = to_string(commonNum);
                        visblefaultinfos[j].commonfaultid = to_string(commonNum);
                        cout<<"红外组件和可见有相同ending"<<endl;

                    }
                }
                
            }
        }

        
        break;
    case 2:
        for (int i = 0; i < infrafaultinfos.size(); i++)
        {
            if(infrafaultinfos[i].infra_fault_id=="Spot")
            {
                for (int j = 0; j < visblefaultinfos.size(); j++)
                {
                    cv::Rect infraENbox,visbleENbox;
                    infraENbox.x = infrafaultinfos[i].infrasolarpointEN[0].x;
                    infraENbox.y = infrafaultinfos[i].infrasolarpointEN[0].y;

                    infraENbox.width = infrafaultinfos[i].infrasolarpointEN[3].x -infrafaultinfos[i].infrasolarpointEN[0].x;
                    infraENbox.height = infrafaultinfos[i].infrasolarpointEN[3].y -infrafaultinfos[i].infrasolarpointEN[0].y;

                    visbleENbox.x = visblefaultinfos[j].visblesolarpointEN[0].x;
                    visbleENbox.y = visblefaultinfos[j].visblesolarpointEN[0].y;

                    visbleENbox.width = visblefaultinfos[j].visblesolarpointEN[3].x -visblefaultinfos[j].visblesolarpointEN[0].x;
                    visbleENbox.height = visblefaultinfos[j].visblesolarpointEN[3].y -visblefaultinfos[j].visblesolarpointEN[0].y;

                    if(getboxiouratio(infraENbox,visbleENbox)>0.5)
                    {   
                        commonNum +=1;
                        infrafaultinfos[i].commonfaultid = to_string(commonNum);
                        visblefaultinfos[i].commonfaultid = to_string(commonNum);

                    }
                }
                
            }
        }
        break;
    default:
        break;
    }


    
    
}

void resPack::addinfrafaultpointgps(vector<infraFaultJson> &infrafaultinfos,string Timgpath)
{
    cout<<"hhh"<<endl;
}


void resPack::JsonPackagesComine(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfos,cv::Mat timg,cv::Mat zimg)  //用于风光线
{
    Json::Value root;
    Json::Reader reader;
 
    Json::Value infraFaultlist;
    Json::Value visbleFaultlist;
    Json::FastWriter writer;

    //遍历红外list
    reader.parse(ResJsons,root);
    cout<<"打包JSON  "<<infrafaultinfos.size()<<" "<<visblefaultinfos.size()<<endl;
   
    if(infrafaultinfos.size()>0|| visblefaultinfos.size()>0)
    {
        root["imgStatus"]=1;
    }
    else
    {
        root["imgStatus"]=0;
    }

    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
        Json::Value infraitem;
        infraitem["faultNum"] = infrafaultinfos[i].infra_fault_id;  //{"ejggz","jjxrb","zjdl","Spot"};
 
       /*
        switch (infrafaultinfos[i].infra_fault_id)
        {
        case "Spot":
            infraitem["faultType"] = "热斑";
            break;
        case "ejggz":
            infraitem["faultType"] = "二极管故障";
            break;
        case "jjxrb":
            infraitem["faultType"] = "聚集性热斑";
            break;
        case "zjdl":
            infraitem["faultType"] = "组件短路";
            break;
        case "dc":
            infraitem["faultType"] = "掉串";
            break;
        
        default:
            break;
        } */

        if(infrafaultinfos[i].infra_fault_id=="Spot")
        {
            infraitem["faultType"] = "热斑";
            // cout<<"JSON: 热斑"<<endl;
       
        }
        else if(infrafaultinfos[i].infra_fault_id=="ejggz")
        {
            infraitem["faultType"] = "二极管故障";
       
        }
        else if(infrafaultinfos[i].infra_fault_id=="jjxrb")
        {
            infraitem["faultType"] = "聚集性热斑";
           
        }
        else if(infrafaultinfos[i].infra_fault_id=="zjdl")
        {
            infraitem["faultType"] = "组件短路";
          
        }
        else if(infrafaultinfos[i].infra_fault_id=="dc")
        {
            infraitem["faultType"] = "组件电流弱";
         
        }

        //***************cousin20230608 新增 给风光线增加 裁剪对应图像的box点集******************//
        //***********同时新增 timg  zimg    为了拟合多个类型镜头****************//
        cv::Rect fitBox;
        float bit;
        if(timg.cols==1280)
        {
            //超分模式
            bit = 4000.0/1000;
            fitBox.x = int((infrafaultinfos[i].infrafaultsolarbox.x-140)*bit);
            fitBox.y = int((infrafaultinfos[i].infrafaultsolarbox.y-128)*bit);
            fitBox.width = infrafaultinfos[i].infrafaultsolarbox.width*bit;
            fitBox.height = infrafaultinfos[i].infrafaultsolarbox.height*bit;
        }
        else
        {
            bit = 3888.0/512;
            fitBox.x = int(infrafaultinfos[i].infrafaultsolarbox.x*bit);
            fitBox.y = int(infrafaultinfos[i].infrafaultsolarbox.y*bit);
            fitBox.width = infrafaultinfos[i].infrafaultsolarbox.width*bit;
            fitBox.height = infrafaultinfos[i].infrafaultsolarbox.height*bit;
        }

        rebuildcrossedrect(fitBox,zimg);
        infraitem["fitBox"] = 
                    "["+to_string(fitBox.x)+","+to_string(fitBox.y)+","
                    +to_string(fitBox.x+fitBox.width)+","+to_string(fitBox.y)+","
                    +to_string(fitBox.x)+","+to_string(fitBox.y+fitBox.height)+","
                    +to_string(fitBox.x+fitBox.width)+","+to_string(fitBox.y+fitBox.height)+"]";

         //***************cousin20230608 新增 给风光线增加 裁剪对应图像的box点集 ending******************//



        infraitem["faultZcId"] = infrafaultinfos[i].ZC_id;
      
        infraitem["faultBox"]= 
                    "["+to_string(infrafaultinfos[i].infrafaultsolarbox.x)+","+to_string(infrafaultinfos[i].infrafaultsolarbox.y)+","
                    +to_string(infrafaultinfos[i].infrafaultsolarbox.x+infrafaultinfos[i].infrafaultsolarbox.width)+","+to_string(infrafaultinfos[i].infrafaultsolarbox.y)+","
                    +to_string(infrafaultinfos[i].infrafaultsolarbox.x)+","+to_string(infrafaultinfos[i].infrafaultsolarbox.y+infrafaultinfos[i].infrafaultsolarbox.height)+","
                    +to_string(infrafaultinfos[i].infrafaultsolarbox.x+infrafaultinfos[i].infrafaultsolarbox.width)+","+to_string(infrafaultinfos[i].infrafaultsolarbox.y+infrafaultinfos[i].infrafaultsolarbox.height)+"]";
        
        infraitem["maxTemp"] = infrafaultinfos[i].soalrmaxTemptrue;
        infraitem["meanTemp"] = infrafaultinfos[i].soalrmeanTemptrue;
        infraitem["pixinfo"] =  "["+to_string( infrafaultinfos[i].infrafaultsolarbox.x+infrafaultinfos[i].infrafaultsolarbox.width/2)+","+ 
                                to_string( infrafaultinfos[i].infrafaultsolarbox.y+infrafaultinfos[i].infrafaultsolarbox.height/2) +"]";
        infraitem["moduleGps"] = to_string(infrafaultinfos[i].infrasolarcenterEN.x) + ","+to_string(infrafaultinfos[i].infrasolarcenterEN.y);
        infraitem["moduleGpsBox"] = "["+ to_string(infrafaultinfos[i].infrasolarpointEN[0].x)+","+ to_string(infrafaultinfos[i].infrasolarpointEN[0].y)+"]"+","+
                                    "["+ to_string(infrafaultinfos[i].infrasolarpointEN[1].x)+","+ to_string(infrafaultinfos[i].infrasolarpointEN[1].y)+"]"+","+
                                    "["+ to_string(infrafaultinfos[i].infrasolarpointEN[2].x)+","+ to_string(infrafaultinfos[i].infrasolarpointEN[2].y)+"]"+","+
                                    "["+ to_string(infrafaultinfos[i].infrasolarpointEN[3].x)+","+ to_string(infrafaultinfos[i].infrasolarpointEN[3].y)+"]";

        infraitem["commonFaultNum"]=infrafaultinfos[i].commonfaultid;

        cout<<"infraitem "<<infraitem<<endl;
        // infraFaultlist.append(infraitem);
        root["infraFaultList"].append(infraitem);
        infraitem.clear();
    }

    //打包可见
    for (int i = 0; i < visblefaultinfos.size(); i++)
    {
        Json::Value visbleitem;
      
        visbleitem["faultNum"] = visblefaultinfos[i].visble_fault_id;
        // "zd_nf","zd_hc","zd_yw","sl"    "fl_qx","fl","back","fengxi","zd_yy","zd_cm"
        //"遮挡鸟粪","遮挡阴影","遮挡草木","遮挡灰尘","遮挡异物","翻落缺失","碎裂","未知"

        if(visblefaultinfos[i].visble_fault_id=="zd_nf")
        {
            visbleitem["faultType"]=  "遮挡鸟粪";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_hc")
        {
            visbleitem["faultType"]=  "遮挡灰尘";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yw")
        {
            visbleitem["faultType"]=  "遮挡异物";
        }

        else if(visblefaultinfos[i].visble_fault_id=="sl")
        {
            visbleitem["faultType"]=  "碎裂";
        }

        else if(visblefaultinfos[i].visble_fault_id=="fl_qx"|| visblefaultinfos[i].visble_fault_id=="fl")
        {
            visbleitem["faultType"]=  "翻落缺失";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yy")
        {
            visbleitem["faultType"]=  "遮挡阴影";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_cm")
        {
            visbleitem["faultType"]=  "遮挡草木";
        }


        //***************cousin20230608 新增 给风光线增加 裁剪对应图像的box点集******************//
        //***********同时新增 timg  zimg    为了拟合多个类型镜头****************//
        cv::Rect fitBox;
        float bit;
        if(timg.cols==1280)
        {
                //超分模式
            bit = 4000.0/1000;

            fitBox.x = int(visblefaultinfos[i].visblefaultsolarbox.x/bit)+140;
            fitBox.y = int(visblefaultinfos[i].visblefaultsolarbox.y/bit)+128;
            fitBox.width = int(visblefaultinfos[i].visblefaultsolarbox.width/bit);
            fitBox.height = int(visblefaultinfos[i].visblefaultsolarbox.height/bit);

        }
        else
        {
            bit = 3888.0/512;
            fitBox.x = int(visblefaultinfos[i].visblefaultsolarbox.x/bit);
            fitBox.y = int(visblefaultinfos[i].visblefaultsolarbox.y/bit);
            fitBox.width = int(visblefaultinfos[i].visblefaultsolarbox.width/bit);
            fitBox.height = int(visblefaultinfos[i].visblefaultsolarbox.height/bit);


        }

        rebuildcrossedrect(fitBox,timg);
        visbleitem["fitBox"] = 
                    "["+to_string(fitBox.x)+","+to_string(fitBox.y)+","
                    +to_string(fitBox.x+fitBox.width)+","+to_string(fitBox.y)+","
                    +to_string(fitBox.x)+","+to_string(fitBox.y+fitBox.height)+","
                    +to_string(fitBox.x+fitBox.width)+","+to_string(fitBox.y+fitBox.height)+"]";

         //***************cousin20230608 新增 给风光线增加 裁剪对应图像的box点集 ending******************//





        visbleitem["faultZcId"] = visblefaultinfos[i].ZC_id;

        visbleitem["pixinfo"] = "["+to_string( visblefaultinfos[i].visblefaultsolarbox.x+visblefaultinfos[i].visblefaultsolarbox.width/2)+","+ 
                                to_string( visblefaultinfos[i].visblefaultsolarbox.y+visblefaultinfos[i].visblefaultsolarbox.height/2) +"]";
                                 
        visbleitem["faultBox"]= 
                    "["+to_string(visblefaultinfos[i].visblefaultsolarbox.x)+","+to_string(visblefaultinfos[i].visblefaultsolarbox.y)+","
                    +to_string(visblefaultinfos[i].visblefaultsolarbox.x+visblefaultinfos[i].visblefaultsolarbox.width)+","+to_string(visblefaultinfos[i].visblefaultsolarbox.y)+","
                    +to_string(visblefaultinfos[i].visblefaultsolarbox.x)+","+to_string(visblefaultinfos[i].visblefaultsolarbox.y+visblefaultinfos[i].visblefaultsolarbox.height)+","
                    +to_string(visblefaultinfos[i].visblefaultsolarbox.x+visblefaultinfos[i].visblefaultsolarbox.width)+","+to_string(visblefaultinfos[i].visblefaultsolarbox.y+visblefaultinfos[i].visblefaultsolarbox.height)+"]";
        

        visbleitem["moduleGps"] = to_string(visblefaultinfos[i].visblesolarcenterEN.x) + ","+to_string(visblefaultinfos[i].visblesolarcenterEN.y);
        visbleitem["moduleGpsBox"] = "["+ to_string(visblefaultinfos[i].visblesolarpointEN[0].x)+","+ to_string(visblefaultinfos[i].visblesolarpointEN[0].y)+"]"+","+
                                    "["+ to_string(visblefaultinfos[i].visblesolarpointEN[1].x)+","+ to_string(visblefaultinfos[i].visblesolarpointEN[1].y)+"]"+","+
                                    "["+ to_string(visblefaultinfos[i].visblesolarpointEN[2].x)+","+ to_string(visblefaultinfos[i].visblesolarpointEN[2].y)+"]"+","+
                                    "["+ to_string(visblefaultinfos[i].visblesolarpointEN[3].x)+","+ to_string(visblefaultinfos[i].visblesolarpointEN[3].y)+"]";

        visbleitem["commonFaultNum"]=visblefaultinfos[i].commonfaultid;

        // visbleFaultlist.append(visbleitem);
        root["visibleFaultList"].append(visbleitem);
        visbleitem.clear();
    }
    
    

    ResJsons = writer.write(root);


}


void resPack::JsonPackagesComineOld(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfos)  //用于云尚诊断
{
    Json::Value root;
    Json::Reader reader;
 
    Json::Value fualtlist;
 
    Json::FastWriter writer;
    vector<string > falutnumid; //用来计数用的
    //遍历红外list
    reader.parse(ResJsons,root);
    cout<<"打包JSON  "<<infrafaultinfos.size()<<" "<<visblefaultinfos.size()<<endl;
   
    if(infrafaultinfos.size()>0|| visblefaultinfos.size()>0)
    {
        root["imgStatus"]=1;
    }
    else
    {
        root["imgStatus"]=0;
    }

    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
        Json::Value item;
        //{"ejggz","jjxrb","zjdl","Spot"};

        
        if(!infrafaultinfos[i].commonfaultid.empty())  //在old中  需要保证相同区域为   感觉可以不需要
        {   
            continue;
            
        }
        else
        {   
            falutnumid.push_back(infrafaultinfos[i].infra_fault_id);
            int faultID = count(falutnumid.begin(), falutnumid.end(), infrafaultinfos[i].infra_fault_id);
            item["faultNum"] = infrafaultinfos[i].infra_fault_id+"_"+to_string(faultID);

            if(infrafaultinfos[i].infra_fault_id=="Spot")
            {
                item["faultType"] = "电池片故障";
                // cout<<"JSON: 热斑"<<endl;
        
            }
            else if(infrafaultinfos[i].infra_fault_id=="ejggz")
            {
                item["faultType"] = "二极管故障";
        
            }
            else if(infrafaultinfos[i].infra_fault_id=="jjxrb")
            {
                item["faultType"] = "聚集性热斑";
            
            }
            else if(infrafaultinfos[i].infra_fault_id=="zjdl")
            {
                item["faultType"] = "组件短路";
            
            }
            else if(infrafaultinfos[i].infra_fault_id=="dc")
            {
                item["faultType"] = "掉串";
            
            }

            
            
        }
       

        item["max_temp"] = infrafaultinfos[i].soalrmaxTemptrue;
        item["moduleGps"] = to_string(infrafaultinfos[i].infrasolarcenterEN.x)+","+to_string(infrafaultinfos[i].infrasolarcenterEN.y);

        root["fualtlist"].append(item);
        // fualtlist.append(item);
        item.clear();
    }

    //打包可见
    for (int i = 0; i < visblefaultinfos.size(); i++)
    {
        Json::Value item;
        falutnumid.push_back(visblefaultinfos[i].visble_fault_id);
        int faultID = count(falutnumid.begin(), falutnumid.end(), visblefaultinfos[i].visble_fault_id);
        item["faultNum"] = visblefaultinfos[i].visble_fault_id +"_"+to_string(faultID);
        // "zd_nf","zd_hc","zd_yw","sl"    "fl_qx","fl","back","fengxi","zd_yy","zd_cm"
        //"遮挡鸟粪","遮挡阴影","遮挡草木","遮挡灰尘","遮挡异物","翻落缺失","碎裂","未知"

        if(visblefaultinfos[i].visble_fault_id=="zd_nf")
        {
            item["faultType"]=  "遮挡鸟粪";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_hc")
        {
            item["faultType"]=  "遮挡灰尘";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yw")
        {
            item["faultType"]=  "遮挡异物";
        }

        else if(visblefaultinfos[i].visble_fault_id=="sl")
        {
            item["faultType"]=  "碎裂";
        }

        else if(visblefaultinfos[i].visble_fault_id=="fl_qx"|| visblefaultinfos[i].visble_fault_id=="fl")
        {
            item["faultType"]=  "翻落缺失";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yy")
        {
            item["faultType"]=  "遮挡阴影";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_cm")
        {
            item["faultType"]=  "遮挡草木";
        }

     

      
                                 
      

        item["moduleGps"] = to_string(visblefaultinfos[i].visblesolarcenterEN.x) + ","+to_string(visblefaultinfos[i].visblesolarcenterEN.y);
        
        // fualtlist.append(item);
        root["fualtlist"].append(item);
        item.clear();


    }
    
    
    // root["fualtlist"].append(fualtlist);

    ResJsons = writer.write(root);


}

void resPack::JsonPackagesComineYS_split(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfos ,cv::Mat timg,cv::Mat zimg)  //用于云尚诊断
{
    Json::Value root;
    Json::Reader reader;
 
    Json::Value fualtlist;
 
    Json::FastWriter writer;
    vector<string > falutnumid; //用来计数用的
    //遍历红外list
    reader.parse(ResJsons,root);
    cout<<"打包JSON  "<<infrafaultinfos.size()<<" "<<visblefaultinfos.size()<<endl;
    // cout<<"输出红外图像名称"<<root["infraRedName"]<<"  "<<root["visibleLightName"]<<" " <<root["savepath"] <<endl;
    if(infrafaultinfos.size()>0|| visblefaultinfos.size()>0)
    {
        root["imgStatus"]=1;
    }
    else
    {
        root["imgStatus"]=0;
    }

    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
        Json::Value item;
        //{"ejggz","jjxrb","zjdl","Spot"};

        
        if(!infrafaultinfos[i].commonfaultid.empty())  //在old中  需要保证相同区域为   感觉可以不需要
        {   
            /////裁剪图像
            rebuildcrossedrect(infrafaultinfos[i].infrafaultsolarbox,timg); //wzj20230705
            cv::Mat cropTimg = timg(infrafaultinfos[i].infrafaultsolarbox);
            // cv::Mat cropZimg = timg(infrafaultinfos[i].infrafaultsolarbox);
            
            
            int position = root["infraRedName"].asString().find('.');
            std::string Tname = root["infraRedName"].asString().substr(0, position);
            std::string Tpanel_name =  Tname + "_com_" + infrafaultinfos[i].commonfaultid + ".jpg";
            cv::imwrite(root["savepath"].asString()+Tpanel_name,cropTimg);
            item["faultRayPic"] = Tpanel_name;
            
            for (int j = 0; j < visblefaultinfos.size(); j++)
            {
                if(infrafaultinfos[i].commonfaultid==visblefaultinfos[j].commonfaultid )
                {   
                    rebuildcrossedrect(visblefaultinfos[j].visblefaultsolarbox,zimg);
                    cv::Mat cropZimg = zimg(visblefaultinfos[j].visblefaultsolarbox);

                    int position = root["visibleLightName"].asString().find('.');
                    std::string Zname = root["visibleLightName"].asString().substr(0, position);
                    std::string Zpanel_name =  Zname + "_com_" + visblefaultinfos[j].commonfaultid + ".jpg";
                    cv::imwrite(root["savepath"].asString()+Zpanel_name,cropZimg);
                    item["faultVisPic"] = Zpanel_name;
                }
            }
            
            ////裁剪图像


            continue;
            
        }
        else
        {   
            falutnumid.push_back(infrafaultinfos[i].infra_fault_id);
            int faultID = count(falutnumid.begin(), falutnumid.end(), infrafaultinfos[i].infra_fault_id);
            item["faultNum"] = infrafaultinfos[i].infra_fault_id+"_"+to_string(faultID);

            if(infrafaultinfos[i].infra_fault_id=="Spot")
            {
                item["faultType"] = "电池片故障";
                // cout<<"JSON: 热斑"<<endl;
        
            }
            else if(infrafaultinfos[i].infra_fault_id=="ejggz")
            {
                item["faultType"] = "二极管故障";
        
            }
            else if(infrafaultinfos[i].infra_fault_id=="jjxrb")
            {
                // item["faultType"] = "聚集性热斑";
                item["faultType"] = "电池片故障";
            
            }
            else if(infrafaultinfos[i].infra_fault_id=="zjdl")
            {
                item["faultType"] = "组件短路";
            
            }
            else if(infrafaultinfos[i].infra_fault_id=="dc")
            {
                item["faultType"] = "掉串";
            
            }


            /////裁剪图像  红外图像
            cout<< item["faultType"] <<endl;
            rebuildcrossedrect(infrafaultinfos[i].infrafaultsolarbox,timg);
            cout<<" cropTimg   "<<infrafaultinfos[i].infrafaultsolarbox <<endl;

            cv::Mat cropTimg = timg(infrafaultinfos[i].infrafaultsolarbox);

            // cout<<" cropTimg   "<<infrafaultinfos[i].infrafaultsolarbox <<endl;
            
            int position = root["infraRedName"].asString().find('.');
            std::string Tname = root["infraRedName"].asString().substr(0, position);
            std::string Tpanel_name =  Tname + "_" + to_string(i) + ".jpg";

            cv::imwrite(root["savepath"].asString()+Tpanel_name,cropTimg);
            item["faultRayPic"] = Tpanel_name;

            /////裁剪图像  可见图像
            cv::Rect cropzimgrect;
            float bit;
            if(timg.cols==1280)
            {
                //超分模式
                bit = 4000.0/1000;

                cropzimgrect.x = int((infrafaultinfos[i].infrafaultsolarbox.x-140)*bit);
                cropzimgrect.y = int((infrafaultinfos[i].infrafaultsolarbox.y-128)*bit);
                cropzimgrect.width = infrafaultinfos[i].infrafaultsolarbox.width*bit;
                cropzimgrect.height = infrafaultinfos[i].infrafaultsolarbox.height*bit;

            }
            else
            {
                bit = 3888.0/512;
                cropzimgrect.x = int(infrafaultinfos[i].infrafaultsolarbox.x*bit);
                cropzimgrect.y = int(infrafaultinfos[i].infrafaultsolarbox.y*bit);
                cropzimgrect.width = infrafaultinfos[i].infrafaultsolarbox.width*bit;
                cropzimgrect.height = infrafaultinfos[i].infrafaultsolarbox.height*bit;


            }

            
            rebuildcrossedrect(cropzimgrect,zimg);


     
            cout<<"截取可见光图像矩形框 "<<cropzimgrect.x<<" "<<cropzimgrect.y<<" "<<cropzimgrect.width<<" "<<cropzimgrect.height<<"  "<<bit<<endl;
            cv::Mat cropZimg = zimg(cropzimgrect);
         
        
            position = root["visibleLightName"].asString().find('.');
            std::string Zname = root["visibleLightName"].asString().substr(0, position);
            std::string Zpanel_name =  Zname + "_z_" + to_string(i) + ".jpg";
            
            cv::imwrite(root["savepath"].asString()+Zpanel_name,cropZimg);
            item["faultVisPic"] = Zpanel_name;

            
        }
        
        // item["faultRayPic"] = 
        item["max_temp"] = infrafaultinfos[i].soalrmaxTemptrue;
        item["moduleGps"] = to_string(infrafaultinfos[i].infrasolarcenterEN.x)+","+to_string(infrafaultinfos[i].infrasolarcenterEN.y);

        root["fualtlist"].append(item);
        // fualtlist.append(item);
        item.clear();
    }

    //打包可见
    for (int i = 0; i < visblefaultinfos.size(); i++)
    {
        Json::Value item;
        falutnumid.push_back(visblefaultinfos[i].visble_fault_id);
        int faultID = count(falutnumid.begin(), falutnumid.end(), visblefaultinfos[i].visble_fault_id);
        item["faultNum"] = visblefaultinfos[i].visble_fault_id +"_"+to_string(faultID);
        // "zd_nf","zd_hc","zd_yw","sl"    "fl_qx","fl","back","fengxi","zd_yy","zd_cm"
        //"遮挡鸟粪","遮挡阴影","遮挡草木","遮挡灰尘","遮挡异物","翻落缺失","碎裂","未知"
        cout<<" wwwwwwwwwwwwwwwwwwwwwww"<<endl;
        if(visblefaultinfos[i].visble_fault_id=="zd_nf")
        {
            // item["faultType"]=  "遮挡鸟粪";
            item["faultType"]=  "表面污渍";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_hc")
        {
            // item["faultType"]=  "遮挡灰尘";
            item["faultType"]=  "灰尘堆积";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yw")
        {
            // item["faultType"]=  "遮挡异物";
            item["faultType"]=  "异物遮挡";
        }

        else if(visblefaultinfos[i].visble_fault_id=="sl")
        {
            item["faultType"]=  "组件碎裂";
        }

        else if(visblefaultinfos[i].visble_fault_id=="fl_qx"|| visblefaultinfos[i].visble_fault_id=="fl")
        {
            // item["faultType"]=  "翻落缺失";
            item["faultType"]=  "组件翻落";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yy")
        {
            // item["faultType"]=  "遮挡阴影";
            item["faultType"]=  "阴影遮挡";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_cm")
        {
            // item["faultType"]=  "遮挡草木";
            item["faultType"]=  "草木遮挡";
        }
        item["moduleGps"] = to_string(visblefaultinfos[i].visblesolarcenterEN.x) + ","+to_string(visblefaultinfos[i].visblesolarcenterEN.y);
        



        for (int j = 0; j < infrafaultinfos.size(); j++)
        {
            if(infrafaultinfos[j].commonfaultid==visblefaultinfos[i].commonfaultid )
            {   
               break;
            }
        }

        /////裁剪图像  可见图像
        rebuildcrossedrect(visblefaultinfos[i].visblefaultsolarbox,zimg);
        cv::Mat cropZimg = zimg(visblefaultinfos[i].visblefaultsolarbox);

        cout<<"corpZimg  "<< visblefaultinfos[i].visblefaultsolarbox <<endl;


        int position = root["visibleLightName"].asString().find('.');
        std::string Zname = root["visibleLightName"].asString().substr(0, position);
        std::string Zpanel_name =  Zname + "_" + to_string(i) + ".jpg";
        cv::imwrite(root["savepath"].asString()+Zpanel_name,cropZimg);
        item["faultVisPic"] = Zpanel_name;
        cout<<"可见保存名称  "<<root["savepath"].asString()+Zpanel_name<<endl;

        float bit;
        cv::Rect croptimgrect;
        if(timg.cols==1280)
        {
                //超分模式
            bit = 4000.0/1000;

            croptimgrect.x = int(visblefaultinfos[i].visblefaultsolarbox.x/bit)+140;
            croptimgrect.y = int(visblefaultinfos[i].visblefaultsolarbox.y/bit)+128;
            croptimgrect.width = int(visblefaultinfos[i].visblefaultsolarbox.width/bit);
            croptimgrect.height = int(visblefaultinfos[i].visblefaultsolarbox.height/bit);

        }
        else
        {
            bit = 3888.0/512;
            croptimgrect.x = int(visblefaultinfos[i].visblefaultsolarbox.x/bit);
            croptimgrect.y = int(visblefaultinfos[i].visblefaultsolarbox.y/bit);
            croptimgrect.width = int(visblefaultinfos[i].visblefaultsolarbox.width/bit);
            croptimgrect.height = int(visblefaultinfos[i].visblefaultsolarbox.height/bit);
            cout<<"截取红外图像矩形框 "<<croptimgrect.x<<" "<< croptimgrect.y<<" "<< croptimgrect.width<<" "<<croptimgrect.height<<endl;

        }

        rebuildcrossedrect(croptimgrect,timg);

        cout<<"截取红外图像矩形框 "<<croptimgrect<<endl;
        cv::Mat cropTimg = timg(croptimgrect);
        cout<<"截取红外图像成功"<<endl;
            
        position = root["infraRedName"].asString().find('.');
        std::string Tname = root["infraRedName"].asString().substr(0, position);
        std::string Tpanel_name =  Tname + "_t_" + to_string(i) + ".jpg";
        cv::imwrite(root["savepath"].asString()+Tpanel_name,cropTimg);
        item["faultRayPic"] = Tpanel_name;

        // fualtlist.append(item);
        root["fualtlist"].append(item);
        item.clear();


    }
    
    
    // root["fualtlist"].append(fualtlist);

    ResJsons = writer.write(root);


}


void resPack::JsonPackagesComineYS_merge(string& ResJsons,vector<infraFaultJson> infrafaultinfos,vector<visbleFaultJson> visblefaultinfos ,cv::Mat timg,cv::Mat zimg)  //用于云尚诊断  只有红外没有可见
{
    Json::Value root;
    Json::Reader reader;
 
    Json::Value fualtlist;
 
    Json::FastWriter writer;
    vector<string > falutnumid; //用来计数用的
    //遍历红外list
    reader.parse(ResJsons,root);
    cout<<"打包JSON  "<<infrafaultinfos.size()<<" "<<visblefaultinfos.size()<<endl;
    // cout<<"输出红外图像名称"<<root["infraRedName"]<<"  "<<root["visibleLightName"]<<" " <<root["savepath"] <<endl;
    if(infrafaultinfos.size()>0|| visblefaultinfos.size()>0)
    {
        root["imgStatus"]=1;
    }
    else
    {
        root["imgStatus"]=0;
    }

    for (int i = 0; i < infrafaultinfos.size(); i++)
    {
        Json::Value item;
        //{"ejggz","jjxrb","zjdl","Spot"};

        
        if(!infrafaultinfos[i].commonfaultid.empty())  //在old中  需要保证相同区域为   感觉可以不需要
        {   
            /////裁剪图像
            rebuildcrossedrect(infrafaultinfos[i].infrafaultsolarbox,timg);// wzj20230705
            cv::Mat cropTimg = timg(infrafaultinfos[i].infrafaultsolarbox);
            // cv::Mat cropZimg = timg(infrafaultinfos[i].infrafaultsolarbox);
            
            
            int position = root["infraRedName"].asString().find('.');
            std::string Tname = root["infraRedName"].asString().substr(0, position);
            std::string Tpanel_name =  Tname + "_com_" + infrafaultinfos[i].commonfaultid + ".jpg";
            cv::imwrite(root["savepath"].asString()+Tpanel_name,cropTimg);
            item["faultRayPic"] = Tpanel_name;
            
            for (int j = 0; j < visblefaultinfos.size(); j++)
            {
                if(infrafaultinfos[i].commonfaultid==visblefaultinfos[j].commonfaultid )
                {   
                    rebuildcrossedrect(visblefaultinfos[j].visblefaultsolarbox,zimg);
                    cv::Mat cropZimg = zimg(visblefaultinfos[j].visblefaultsolarbox);

                    int position = root["visibleLightName"].asString().find('.');
                    std::string Zname = root["visibleLightName"].asString().substr(0, position);
                    std::string Zpanel_name =  Zname + "_com_" + visblefaultinfos[j].commonfaultid + ".jpg";
                    cv::imwrite(root["savepath"].asString()+Zpanel_name,cropZimg);
                    item["faultVisPic"] = Zpanel_name;
                }
            }
            
            ////裁剪图像


            continue;
            
        }
        else
        {   
            falutnumid.push_back(infrafaultinfos[i].infra_fault_id);
            int faultID = count(falutnumid.begin(), falutnumid.end(), infrafaultinfos[i].infra_fault_id);
            item["faultNum"] = infrafaultinfos[i].infra_fault_id+"_"+to_string(faultID);

            if(infrafaultinfos[i].infra_fault_id=="Spot")
            {
                item["faultType"] = "电池片故障";
                // cout<<"JSON: 热斑"<<endl;
        
            }
            else if(infrafaultinfos[i].infra_fault_id=="ejggz")
            {
                item["faultType"] = "二极管故障";
        
            }
            else if(infrafaultinfos[i].infra_fault_id=="jjxrb")
            {
                // item["faultType"] = "聚集性热斑";
                item["faultType"] = "电池片故障";
            
            }
            else if(infrafaultinfos[i].infra_fault_id=="zjdl")
            {
                item["faultType"] = "组件短路";
            
            }
            else if(infrafaultinfos[i].infra_fault_id=="dc")
            {
                item["faultType"] = "掉串";
            
            }


            /////裁剪图像  红外图像
            cout<< item["faultType"] <<endl;
            // cout<<" cropTimg   "<<infrafaultinfos[i].infrafaultsolarbox <<endl;

            rebuildcrossedrect(infrafaultinfos[i].infrafaultsolarbox,timg);// wzj 20230705  

            cv::Mat cropTimg = timg(infrafaultinfos[i].infrafaultsolarbox);

            
            
            int position = root["infraRedName"].asString().find('.');
            std::string Tname = root["infraRedName"].asString().substr(0, position);
            std::string Tpanel_name =  Tname + "_" + to_string(i) + ".jpg";

            cv::imwrite(root["savepath"].asString()+Tpanel_name,cropTimg);
            item["faultRayPic"] = Tpanel_name;

            /////裁剪图像  可见图像
            cv::Rect cropzimgrect;
            float bit;
            if(timg.cols==1280)
            {
                //超分模式
                bit = 4000.0/1000;

                cropzimgrect.x = int((infrafaultinfos[i].infrafaultsolarbox.x-140)*bit);
                cropzimgrect.y = int((infrafaultinfos[i].infrafaultsolarbox.y-128)*bit);
                cropzimgrect.width = infrafaultinfos[i].infrafaultsolarbox.width*bit;
                cropzimgrect.height = infrafaultinfos[i].infrafaultsolarbox.height*bit;

            }
            else
            {
                // bit = 3888.0/512;
                bit = zimg.rows*1.0/timg.rows;
                cropzimgrect.x = int(infrafaultinfos[i].infrafaultsolarbox.x*bit);
                cropzimgrect.y = int(infrafaultinfos[i].infrafaultsolarbox.y*bit);
                cropzimgrect.width = infrafaultinfos[i].infrafaultsolarbox.width*bit;
                cropzimgrect.height = infrafaultinfos[i].infrafaultsolarbox.height*bit;


            }

            
            rebuildcrossedrect(cropzimgrect,zimg);


     
            cout<<"截取可见光图像矩形框 "<<cropzimgrect.x<<" "<<cropzimgrect.y<<" "<<cropzimgrect.width<<" "<<cropzimgrect.height<<"  "<<bit<<endl;
            cv::Mat cropZimg = zimg(cropzimgrect);
         
        
            position = root["visibleLightName"].asString().find('.');
            std::string Zname = root["visibleLightName"].asString().substr(0, position);
            std::string Zpanel_name =  Zname + "_z_" + to_string(i) + ".jpg";
            
            cv::imwrite(root["savepath"].asString()+Zpanel_name,cropZimg);
            item["faultVisPic"] = Zpanel_name;

            
        }
        
        // item["faultRayPic"] = 
        item["max_temp"] = infrafaultinfos[i].soalrmaxTemptrue;
        item["moduleGps"] = to_string(infrafaultinfos[i].infrasolarcenterEN.x)+","+to_string(infrafaultinfos[i].infrasolarcenterEN.y);

        root["fualtlist"].append(item);
        // fualtlist.append(item);
        item.clear();
    }

    //打包可见
    for (int i = 0; i < visblefaultinfos.size(); i++)
    {
        Json::Value item;
        falutnumid.push_back(visblefaultinfos[i].visble_fault_id);
        int faultID = count(falutnumid.begin(), falutnumid.end(), visblefaultinfos[i].visble_fault_id);
        item["faultNum"] = visblefaultinfos[i].visble_fault_id +"_"+to_string(faultID);
        // "zd_nf","zd_hc","zd_yw","sl"    "fl_qx","fl","back","fengxi","zd_yy","zd_cm"
        //"遮挡鸟粪","遮挡阴影","遮挡草木","遮挡灰尘","遮挡异物","翻落缺失","碎裂","未知"
        cout<<" wwwwwwwwwwwwwwwwwwwwwww"<<endl;

        for (int j = 0; j < infrafaultinfos.size(); j++)
        {
            if(infrafaultinfos[j].commonfaultid==visblefaultinfos[i].commonfaultid )
            {   
               break;
            }
        }

        if(visblefaultinfos[i].visble_fault_id=="zd_nf")
        {
            // item["faultType"]=  "遮挡鸟粪";
            break; //新增 wzj 20230703
            item["faultType"]=  "表面污渍";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_hc")
        {
            // item["faultType"]=  "遮挡灰尘";
            break; //新增 wzj20230703
            item["faultType"]=  "灰尘堆积";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yw")
        {
            // item["faultType"]=  "遮挡异物";
            item["faultType"]=  "异物遮挡";
        }

        else if(visblefaultinfos[i].visble_fault_id=="sl")
        {
            item["faultType"]=  "组件碎裂";
        }

        else if(visblefaultinfos[i].visble_fault_id=="fl_qx"|| visblefaultinfos[i].visble_fault_id=="fl")
        {
            // item["faultType"]=  "翻落缺失";
            item["faultType"]=  "组件翻落";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_yy")
        {
            // item["faultType"]=  "遮挡阴影";
            item["faultType"]=  "阴影遮挡";
        }

        else if(visblefaultinfos[i].visble_fault_id=="zd_cm")
        {
            // item["faultType"]=  "遮挡草木";
            item["faultType"]=  "草木遮挡";
        }
        item["moduleGps"] = to_string(visblefaultinfos[i].visblesolarcenterEN.x) + ","+to_string(visblefaultinfos[i].visblesolarcenterEN.y);
        



        
        /////裁剪图像  可见图像
        rebuildcrossedrect(visblefaultinfos[i].visblefaultsolarbox,zimg);
        cv::Mat cropZimg = zimg(visblefaultinfos[i].visblefaultsolarbox);

        cout<<"corpZimg  "<< visblefaultinfos[i].visblefaultsolarbox <<endl;


        int position = root["visibleLightName"].asString().find('.');
        std::string Zname = root["visibleLightName"].asString().substr(0, position);
        std::string Zpanel_name =  Zname + "_" + to_string(i) + ".jpg";
        cv::imwrite(root["savepath"].asString()+Zpanel_name,cropZimg);
        item["faultVisPic"] = Zpanel_name;
        cout<<"可见保存名称  "<<root["savepath"].asString()+Zpanel_name<<endl;

        float bit;
        cv::Rect croptimgrect;
        if(timg.cols==1280)
        {
                //超分模式
            bit = 4000.0/1000;

            croptimgrect.x = int(visblefaultinfos[i].visblefaultsolarbox.x/bit)+140;
            croptimgrect.y = int(visblefaultinfos[i].visblefaultsolarbox.y/bit)+128;
            croptimgrect.width = int(visblefaultinfos[i].visblefaultsolarbox.width/bit);
            croptimgrect.height = int(visblefaultinfos[i].visblefaultsolarbox.height/bit);

        }
        else
        {
            bit = 3888.0/512;
            bit = zimg.rows*1.0/timg.rows;
            croptimgrect.x = int(visblefaultinfos[i].visblefaultsolarbox.x/bit);
            croptimgrect.y = int(visblefaultinfos[i].visblefaultsolarbox.y/bit);
            croptimgrect.width = int(visblefaultinfos[i].visblefaultsolarbox.width/bit);
            croptimgrect.height = int(visblefaultinfos[i].visblefaultsolarbox.height/bit);
            cout<<"截取红外图像矩形框 "<<croptimgrect.x<<" "<< croptimgrect.y<<" "<< croptimgrect.width<<" "<<croptimgrect.height<<endl;

        }

        rebuildcrossedrect(croptimgrect,timg);

        cout<<"截取红外图像矩形框 "<<croptimgrect<<endl;
        cv::Mat cropTimg = timg(croptimgrect);
        cout<<"截取红外图像成功"<<endl;
            
        position = root["infraRedName"].asString().find('.');
        std::string Tname = root["infraRedName"].asString().substr(0, position);
        std::string Tpanel_name =  Tname + "_t_" + to_string(i) + ".jpg";
        cv::imwrite(root["savepath"].asString()+Tpanel_name,cropTimg);
        item["faultRayPic"] = Tpanel_name;

        // fualtlist.append(item);
        root["fualtlist"].append(item);
        item.clear();


    }
    
    
    // root["fualtlist"].append(fualtlist);

    ResJsons = writer.write(root);


}