cmake_minimum_required(VERSION 3.5)

# if(CMAKE_COMPILER_IS_GNUCC)
#     message("COMPILER IS GNUCC")
#         ADD_DEFINITIONS ( -std=c++11 )
# endif(CMAKE_COMPILER_IS_GNUCC)
set (CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb3")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/solar)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/unite_sys)