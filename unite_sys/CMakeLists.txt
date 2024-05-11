cmake_minimum_required(VERSION 3.5)

project(ISFINAL)

set (CMAKE_CXX_STANDARD 17)

#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb3")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
find_package(OpenCV REQUIRED)
find_package(exiv2 REQUIRED)

# 5. 添加头文件路径
set (3RD_INCLUDE_PATH  "${CMAKE_SOURCE_DIR}/3rdInc")
set (aws_crt_INCLUDE_PATH  "${CMAKE_SOURCE_DIR}/aws_place/aws_install/include")
set (AMQP_INCLUDE_PATH  "${CMAKE_SOURCE_DIR}/3rdInc/amqp")
set (ACE_INCLUDE_PATH  "${CMAKE_SOURCE_DIR}/3rdInc/ace")
set (HIREDIS_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/3rdInc/hiredis")
set (JSON_INCLUDE_DIRS "/usr/include")
set (EXIF_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/3rdInc/exif")
set (DJI_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/3rdInc/DJI")
set (ONNX_INCLUDE_PATH  "${CMAKE_SOURCE_DIR}/3rdInc/onnxruntime")




message("打印-----------")
message(${CMAKE_CURRENT_SOURCE_DIR})#/home/sne/data_7t/tri_det/cp19/process
message(${CMAKE_SOURCE_DIR})#/home/sne/data_7t/tri_det/cp19
message(${PROJECT_SOURCE_DIR})#/home/sne/data_7t/tri_det/cp19/process

message("---------------------")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/solar/include)
include_directories(${3RD_INCLUDE_PATH})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${aws_crt_INCLUDE_PATH})
include_directories(${AMQP_INCLUDE_PATH})
include_directories(${ACE_INCLUDE_PATH})
include_directories(${JSON_INCLUDE_DIRS})
include_directories(${HIREDIS_INCLUDE_DIRS})
include_directories(${EXIF_INCLUDE_DIRS})
include_directories(${DJI_INCLUDE_DIRS})
include_directories(${ONNX_INCLUDE_PATH})


#添加库文件
set (GLOGLIB_FILES "${CMAKE_SOURCE_DIR}/3rdLib/glog")
set (ACELIB_FILES "${CMAKE_SOURCE_DIR}/3rdLib/ace")
set (AWSLIB_FILES "${CMAKE_SOURCE_DIR}/aws_place/aws_install/lib")
set (SYSTEMLIB_FILES "${CMAKE_SOURCE_DIR}/3rdLib/system")
set (DIRPLIB_FILES "${CMAKE_SOURCE_DIR}/3rdLib/dirp")
set (AMQPLIB_FILES "${CMAKE_SOURCE_DIR}/3rdLib/amqp")
set (ALGO_MINIO_FILES "${CMAKE_SOURCE_DIR}/lib")
set (ONNX_FILES "${CMAKE_SOURCE_DIR}/3rdLib/onnxruntime")


LINK_DIRECTORIES(${GLOGLIB_FILES})
LINK_DIRECTORIES(${ACELIB_FILES})
LINK_DIRECTORIES(${AWSLIB_FILES})
LINK_DIRECTORIES(${SYSTEMLIB_FILES})
LINK_DIRECTORIES(${DIRPLIB_FILES})
LINK_DIRECTORIES(${AMQPLIB_FILES})
LINK_DIRECTORIES(${ALGO_MINIO_FILES})
LINK_DIRECTORIES(${ONNX_FILES})


# 6. 添加源文件
FILE(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
message(${CMAKE_CURRENT_SOURCE_DIR})
message(${SOURCE_FILES})
# 7. 添加链接库
message("打印----CMAKE_BINARY_DIR-------")
message(${CMAKE_BINARY_DIR})#/home/sne/data_7t/tri_det/cp19/build


# 生成so包
# ADD_LIBRARY(${PROJECT_NAME} SHARED ${ALL_SRCS})
#生成libDriverSolarM30.so 
set(SOLAR_LIBS  ${ALGO_MINIO_FILES}/libSolarDetect.so)

set(ONNX_LIBS  ${CMAKE_SOURCE_DIR}/3rdLib/onnxruntime/libonnxruntime.so
	${CMAKE_SOURCE_DIR}/3rdLib/onnxruntime/libonnxruntime.so.1.11.1
)
file (GLOB DETECT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM DETECT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/main.cpp")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib")
set(LIB_NAME "DriverSolarM30")
add_library(${LIB_NAME} SHARED ${DETECT_SOURCE_FILES} ${ACELIB_FILES})
target_link_libraries(${LIB_NAME} ${OpenCV_LIBRARIES} ${SOLAR_LIBS} ${ONNX_LIBS})

# 9. 设置环境变量，编译用到的源文件全部都要放到这里，否则编译能够通过，
#但是执行的时候会出现各种问题，比如"symbol lookup error xxxxx , undefined symbol"
SET(ALL_SRCS ${SOURCE_FILES})
message(${ALL_SRCS})

# 10.add executable file，添加要编译的可执行文件
ADD_EXECUTABLE(${PROJECT_NAME} ./src/Main.cpp ./src/CheckSysStatus.cpp ./src/DetectTask.cpp  ./src/SystemManager.cpp)
target_link_libraries(${PROJECT_NAME} json)
target_link_libraries (${PROJECT_NAME} glog ACE aws-cpp-sdk-core aws-cpp-sdk-s3 dirp ${ONNX_LIBS} ${OpenCV_LIBRARIES})
target_link_libraries (${PROJECT_NAME} dl m pthread rt cudart)
target_link_libraries (${PROJECT_NAME} png12 ev amqpcpp SneMinioManager)

target_link_libraries (${PROJECT_NAME} "/usr/local/lib/libhiredis.so")

# 不可以
#TARGET_LINK_LIBRARIES(${CMAKE_BINARY_DIR}/module/libmodule.so)
