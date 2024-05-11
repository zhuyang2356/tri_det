/*
 * SneMinIOManager.h
 *
 *  Created on: Feb 22, 2023
 *      Author: sne
 */

#ifndef SNEMINIOMANAGER_H_
#define SNEMINIOMANAGER_H_

#include <iostream>
#include <fstream>
#include "LogRecord.h"
#include "aws/s3/S3Client.h"
#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentialsProvider.h"
#include "aws/s3/model/PutObjectRequest.h"
#include "aws/s3/model/GetObjectRequest.h"
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/Object.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace std;

class CMinIOManager {
public:
	CMinIOManager();
	virtual ~CMinIOManager();
	bool init(string address, string user, string passwd);
	bool uploadfile(std::string BucketName, std::string objectKey, std::string pathkey);
	// int downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey,std::string rawPreFixPath);
	int downloadfile(std::string BucketName, std::string preFixPath,std::string pathkey);

private:
	string m_address_;
	string m_user_;
	string m_passwd_;
	S3Client * m_client_ ;

	string pathbase_;
};

#endif /* SNEMINIOMANAGER_H_ */
