/*
 * SneMinIOManager.cpp
 *
 *  Created on: Feb 22, 2023
 *      Author: sne
 */

#include "SneMinIOManager.h"
#include <dirent.h>
#include <iostream>
//#include <filesystem>
CMinIOManager::CMinIOManager() {
	// TODO Auto-generated constructor stub
//	m_address_ = "http://192.16.11.84:9000";
	m_address_ = "http://192.16.10.250:9090";
	m_user_ = "minioadmin";
	m_passwd_="minioadmin";
	m_client_ = { NULL };

}

CMinIOManager::~CMinIOManager() {
	// TODO Auto-generated destructor stub

    if (m_client_ != nullptr)
    {
        delete m_client_;
        m_client_ = NULL;
    }
    Aws::SDKOptions m_options;
    Aws::ShutdownAPI(m_options);
}
bool CMinIOManager::init(string address, string user, string passwd)
{
	m_address_ = address;
	m_user_ =  user;
	m_passwd_= passwd;

	Aws::SDKOptions m_options;
	Aws::InitAPI(m_options);
	Aws::Client::ClientConfiguration cfg;
	cfg.endpointOverride = m_address_;  // S3服务器地址和端口
	cfg.scheme = Aws::Http::Scheme::HTTP;
	cfg.verifySSL = false;
	m_client_ = new S3Client(Aws::Auth::AWSCredentials(m_user_, m_passwd_),  cfg, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
	if(m_client_)
		ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)CMinIOManager init sucess!.\n")));
	else
		ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)CMinIOManager init fault!.\n")));
	return true;
}
//objectKey必须是带有多级子目录的文件名。传一个文件夹没有用。pathkey也必须是本地带有多级子目录的绝对文件名
bool CMinIOManager::uploadfile(std::string BucketName, std::string objectKey, std::string pathkey)
{
	PutObjectRequest putObjectRequest;
    putObjectRequest.WithBucket(BucketName.c_str()).WithKey(objectKey.c_str());
    auto input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream", pathkey.c_str(), std::ios_base::in | std::ios_base::binary);
    putObjectRequest.SetBody(input_data);
    auto putObjectResult = m_client_->PutObject(putObjectRequest);
    if (putObjectResult.IsSuccess())
    {
    	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)uploadfile:%s-%s-%s sucess!.\n"),BucketName.c_str(),objectKey.c_str(),pathkey.c_str()));
    	return true;
    }
    else
    {
    	ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)PutObject error: %s:%s\n"),putObjectResult.GetError().GetExceptionName().c_str(),putObjectResult.GetError().GetMessage().c_str()));

        return false;
    }
    return true;
}
int CMinIOManager::downloadfile(std::string BucketName, 
								std::string preFixPath,
								std::string pathkey)
{
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)进入minio下载流程：%d.\n"),-2));
	int fileCount = 0;
	Aws::S3::Model::ListObjectsRequest objects_request;
//	preFix是
//	string preFixPath="/pvStore/images/测试仓库1/DJI_202309021225_测试航线";
	objects_request.WithBucket(BucketName);
	objects_request.WithPrefix(preFixPath);
//	这里是读取了整个桶的图片，没有规定桶下面的路径
	auto list_objects_outcome = m_client_->ListObjects(objects_request);
	if (list_objects_outcome.IsSuccess())
	{
		Aws::Vector<Aws::S3::Model::Object> object_list =
			list_objects_outcome.GetResult().GetContents();
		int obj_len=object_list.size();
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)进入minio：在桶:%s \n,路径:%s \n 下面有%d 个文件.\n"),BucketName.c_str(),preFixPath.c_str(),obj_len));
		for (auto const &s3_object : object_list)
		{
			Aws::S3::Model::GetObjectRequest object_request;
//			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)object_list[%d]：%s.\n"),i,s3_object.GetKey().c_str()));

			object_request.WithBucket(BucketName.c_str()).WithKey(s3_object.GetKey());
			auto get_object_outcome = m_client_->GetObject(object_request);
			if (get_object_outcome.IsSuccess())
			{
////				第一种方法 pathkey必须要已经存在
//				string local_file_path=s3_object.GetKey();
//				size_t pos=local_file_path.find_last_of("/");
////				pvStore/images/测试仓库1/DJI_202309021225_测试航线/DJI_20220714145216_0175_Z.JPG
//				string local_file_name=local_file_path.substr(pos+1);
////				string filepath = pathkey + s3_object.GetKey();
//				string filepath=pathkey+local_file_name;
////				路径必须是 /home/xxx这种绝对路径，~/pvstore这种路径都不可以,只能把文件下载到这个本地路径下的东西里
////				还不能下载到文件夹李,下载到文件夹李要新建文件夹
////				string filepath = "/home/sne/pvstore/xxx";
//				Aws::OFStream local_file;
//				local_file.open(filepath.c_str() , std::ios::out | std::ios::binary);
//				local_file.seekp(0, std::ios::beg);
//				local_file << get_object_outcome.GetResult().GetBody().rdbuf();
//				local_file.close();
//				fileCount ++;
//				ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)%s下载成功!\n下载到%s\n"),pathkey.c_str(),filepath.c_str()));
//				第二种方法---------------------
				string local_file_path=s3_object.GetKey();
				size_t pos=local_file_path.find_last_of("/");
//				检查村不存在文件夹，不存在创建文件夹
				string local_ware_path=pathkey+"/"+local_file_path.substr(0,pos);
				if (access(local_ware_path.c_str(),0)==-1){
					string mkdir_com;
					// mkdir_com="echo %s |sudo mkdir -p "+local_ware_path;
					mkdir_com="mkdir -p "+local_ware_path;
					system(mkdir_com.c_str());
					ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)创建文件夹%s\n"),(local_ware_path).c_str()));
				}
//				filepath包含文件名，是个绝对路径
				string filepath=pathkey+"/"+local_file_path;
				Aws::OFStream local_file;
				local_file.open(filepath.c_str() , std::ios::out | std::ios::binary);
				local_file.seekp(0, std::ios::beg);
				local_file << get_object_outcome.GetResult().GetBody().rdbuf();
				local_file.close();
				fileCount ++;
				ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)%s下载成功!\n下载到%s\n"),pathkey.c_str(),filepath.c_str()));
			}
			else
			{
				ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)GetObject error: %s:%s\n"),get_object_outcome.GetError().GetExceptionName().c_str(),get_object_outcome.GetError().GetMessage().c_str()));
			}
		}
	}
	else
	{
		ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)ListObjects error: %s:%s\n"),list_objects_outcome.GetError().GetExceptionName().c_str(),list_objects_outcome.GetError().GetMessage().c_str()));
	}
	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)下载了%d个文件!\n"),fileCount));
    return fileCount;
}

// preFixPath 将空格做替换后的路径，用于保存本地下载路径
// rawPreFixPath 将
// int CMinIOManager::downloadfile(std::string BucketName, 
// 								std::string preFixPath,
// 								std::string pathkey,
// 								std::string rawPreFixPath)
// {
// 	ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)进入minio下载流程s5s：%d.\n"),-2));
// 	int fileCount = 0;
// 	Aws::S3::Model::ListObjectsRequest objects_request;
// //	preFix是
// //	string preFixPath="/pvStore/images/测试仓库1/DJI_202309021225_测试航线";
// 	objects_request.WithBucket(BucketName);
// 	// objects_request.WithPrefix(preFixPath);
// 	objects_request.WithPrefix(rawPreFixPath);
// //	这里是读取了整个桶的图片，没有规定桶下面的路径
// 	auto list_objects_outcome = m_client_->ListObjects(objects_request);
// 	if (list_objects_outcome.IsSuccess())
// 	{
// 		Aws::Vector<Aws::S3::Model::Object> object_list =
// 			list_objects_outcome.GetResult().GetContents();
// 		int obj_len=object_list.size();
// 		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)进入minio：在桶:%s \n,路径:%s \n 下面有%d 个文件.\n"),BucketName.c_str(),rawPreFixPath.c_str(),obj_len));
// 		for (auto const &s3_object : object_list)
// 		{
// 			Aws::S3::Model::GetObjectRequest object_request;
// //			ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)object_list[%d]：%s.\n"),i,s3_object.GetKey().c_str()));

// 			object_request.WithBucket(BucketName.c_str()).WithKey(s3_object.GetKey());
// 			auto get_object_outcome = m_client_->GetObject(object_request);
// 			if (get_object_outcome.IsSuccess())
// 			{
// ////				第一种方法 pathkey必须要已经存在
// //				string local_file_path=s3_object.GetKey();
// //				size_t pos=local_file_path.find_last_of("/");
// ////				pvStore/images/测试仓库1/DJI_202309021225_测试航线/DJI_20220714145216_0175_Z.JPG
// //				string local_file_name=local_file_path.substr(pos+1);
// ////				string filepath = pathkey + s3_object.GetKey();
// //				string filepath=pathkey+local_file_name;
// ////				路径必须是 /home/xxx这种绝对路径，~/pvstore这种路径都不可以,只能把文件下载到这个本地路径下的东西里
// ////				还不能下载到文件夹李,下载到文件夹李要新建文件夹
// ////				string filepath = "/home/sne/pvstore/xxx";
// //				Aws::OFStream local_file;
// //				local_file.open(filepath.c_str() , std::ios::out | std::ios::binary);
// //				local_file.seekp(0, std::ios::beg);
// //				local_file << get_object_outcome.GetResult().GetBody().rdbuf();
// //				local_file.close();
// //				fileCount ++;
// //				ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)%s下载成功!\n下载到%s\n"),pathkey.c_str(),filepath.c_str()));
// //				第二种方法---------------------
// 				string local_file_path=s3_object.GetKey();
// 				size_t pos=local_file_path.find_last_of("/");
// 				string local_file_name=local_file_path.substr(pos);
// //				检查村不存在文件夹，不存在创建文件夹
// 				// string local_ware_path=pathkey+"/"+local_file_path.substr(0,pos);
// 				string local_ware_path=pathkey+preFixPath;

// 				if (access(local_ware_path.c_str(),0)==-1){
// 					string mkdir_com;
// 					// mkdir_com="echo %s |sudo mkdir -p "+local_ware_path;
// 					mkdir_com="mkdir -p "+local_ware_path;
// 					system(mkdir_com.c_str());
// 					ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)创建文件夹%s\n"),(local_ware_path).c_str()));
// 				}
// //				filepath包含文件名，是个绝对路径
// 				// string filepath=pathkey+"/"+local_file_path;
// 				string filepath=pathkey+preFixPath+"/"+local_file_name;
// 				Aws::OFStream local_file;
// 				local_file.open(filepath.c_str() , std::ios::out | std::ios::binary);
// 				local_file.seekp(0, std::ios::beg);
// 				local_file << get_object_outcome.GetResult().GetBody().rdbuf();
// 				local_file.close();
// 				fileCount ++;
// 				ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)%s下载成功!\n下载到%s\n"),pathkey.c_str(),filepath.c_str()));
// 			}
// 			else
// 			{
// 				ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)GetObject error: %s:%s\n"),get_object_outcome.GetError().GetExceptionName().c_str(),get_object_outcome.GetError().GetMessage().c_str()));
// 			}
// 		}
// 	}
// 	else
// 	{
// 		ACE_ERROR((LM_ERROR,ACE_TEXT("(%D)ListObjects error: %s:%s\n"),list_objects_outcome.GetError().GetExceptionName().c_str(),list_objects_outcome.GetError().GetMessage().c_str()));
// 	}
// 	ACE_DEBUG((LM_INFO,ACE_TEXT("(%D)下载了%d个文件!\n"),fileCount));
//     return fileCount;
// }


