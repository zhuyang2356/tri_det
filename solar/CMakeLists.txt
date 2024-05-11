PROJECT(SNESolarDetect_8050)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

set (CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(exiv2 REQUIRED)

set (JSON_INCLUDE_DIRS "/usr/include")
set (LOGGER_DIRS "${PROJECT_SOURCE_DIR}/include/spdlog")

set (DETECT_HEADER_FILES "${PROJECT_SOURCE_DIR}/include")

#添加cousin 20230510
set (ONNX_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/onnxruntime")
set (EXIF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/exif")
set (DJI_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/DJI")
set (SEGMENTATION_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/modelinferce/segmentation/u2net")
set (DETECTION_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/modelinferce/detection/yolov5")

message("opencv路径")
message(${OpenCV_INCLUDE_DIRS})
message(${PROJECT_SOURCE_DIR})
include_directories (   
    ${OpenCV_INCLUDE_DIRS}
    ${DETECT_HEADER_FILES}
    ${JSON_INCLUDE_DIRS}
    ${RJPEG_HEADER_FILES} ###add cousin 20230510 start 
    ${ONNX_INCLUDE_DIRS}    ##存放头文件
    ${EXIF_INCLUDE_DIRS}
    ${LOGGER_DIRS}
)


set (ONNX_LIB_FILES "${CMAKE_SOURCE_DIR}/3rdLib/onnxruntime")
LINK_DIRECTORIES(${ONNX_LIB_FILES})

SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib")

add_library(SolarDetect SHARED ./src/vis_detect.cpp  ./src/infra_detect.cpp ./src/resultPackage.cpp 
        ./src/solar_utils.cpp ./src/algorithm.cpp
        ./include/exif/exif.cpp ./include/DJI/read_dji_thremal.cpp 
        ./include/modelinferce/segmentation/u2net/u2net.cpp   
        ./include/modelinferce/detection/yolov5/yolov5.cpp)             


add_executable (${PROJECT_NAME} ./src/main.cpp ./src/httpserver.cpp  ./src/detectthread.cpp )
target_link_libraries(${PROJECT_NAME} libPocoFoundationd.so libPocoNetd.so SolarDetect)

target_link_libraries (${PROJECT_NAME} ${OpenCV_LIBRARIES} exiv2lib json onnxruntime dirp glog -lpthread )  
##-lpthread  thread 线程库