cmake_minimum_required(VERSION 3.5)
project(test_slave_node)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(dynamixel_sdk REQUIRED)

#set(dynamixel_sdk_path "/home/quasi/projects/cpp/DynamixelSDK/c++/")
#set(dynamixel_sdk_INCLUDE_DIRS "${dynamixel_sdk_path}/include")
#set(dynamixel_sdk_LIBRARIES "${dynamixel_sdk_path}/build/linux64/libdxl_x64_cpp.so")

#set(dynamixel_sdk_INCLUDE_DIRS "/usr/local/include")
#set(dynamixel_sdk_LIBRARIES "/usr/local/lib/libdxl_x64_cpp.so")

set(DEPENDENCIES
  "dynamixel_sdk"
  )
include_directories(
  ${dynamixel_sdk_INCLUDE_DIRS}
  )

#message(STATUS ${dynamixel_sdk_INCLUDE_DIRS})
#message(STATUS ${dynamixel_sdk_LIBRARIES})

add_executable(${PROJECT_NAME} test_node.cpp port_handler_ext.cpp serial_channel.cpp)
target_link_libraries(${PROJECT_NAME} ${dynamixel_sdk_LIBRARIES} pthread)
