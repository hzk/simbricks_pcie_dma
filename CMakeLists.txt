# this file is for CACTUS
cmake_minimum_required(VERSION 3.0)

add_definitions(-DFORCACTUS)
# CACTUS Dir is ../CACTUS/
include(../CACTUS/util.cmake)

set (CUR_LIB_NAME lib_simbricks_pcie)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES "${SRC_PATH}/*.c")

add_library(${CUR_LIB_NAME} ${SOURCES})

# 定义变量
set(EXTRA_CPPFLAGS "")
set(EXTRA_CFLAGS "")
set(EXTRA_CXXFLAGS "")
set(EXTRA_LDFLAGS "")

# 设置包含路径
include_directories(common /simbricks/lib)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3 ${EXTRA_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 ${EXTRA_CXXFLAGS}")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

# 设置链接选项
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/simbricks/lib ${EXTRA_LDFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/simbricks/lib")

# 定义SIMBRICKS_FLAGS变量
set(SIMBRICKS_FLAGS "")

# 处理PARALLEL变量的条件语句
if (PARALLEL)
  set(SIMBRICKS_FLAGS "${SIMBRICKS_FLAGS} --parallel")
endif()

# 将SIMBRICKS_FLAGS变量设置为全局属性
set_property(GLOBAL PROPERTY SIMBRICKS_FLAGS ${SIMBRICKS_FLAGS})

# 输出调试信息
message(STATUS "SIMBRICKS_FLAGS: ${SIMBRICKS_FLAGS}")


#target_compile_options(${CUR_LIB_NAME} PRIVATE ${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS})
#target_link_libraries(${CUR_LIB_NAME} PRIVATE ${CMAKE_EXE_LINKER_FLAGS})
target_link_options(${CUR_LIB_NAME} PRIVATE -L/simbricks/lib ${EXTRA_LDFLAGS})




