cmake_minimum_required(VERSION 3.10)
project(NetworkProtocols)

# Toolchain and flags
SET(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexec-charset=GBK")
add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
FIND_LIBRARY(NETSIM_LIB libnetsim.a ${PROJECT_SOURCE_DIR}/lib)

# Stop-Wait Protocol
SET(STOP_WAIT_SRC
    ${PROJECT_SOURCE_DIR}/src/StopWait.cpp
    ${PROJECT_SOURCE_DIR}/src/StopWaitRdtSender.cpp
    ${PROJECT_SOURCE_DIR}/src/StopWaitRdtReceiver.cpp
)
ADD_EXECUTABLE(stop_wait ${STOP_WAIT_SRC})
TARGET_LINK_LIBRARIES(stop_wait ${NETSIM_LIB})

# GBN Protocol
SET(GBN_SRC
    ${PROJECT_SOURCE_DIR}/src/Gbn.cpp
    ${PROJECT_SOURCE_DIR}/src/GbnRdtSender.cpp
    ${PROJECT_SOURCE_DIR}/src/GbnRdtReceiver.cpp
)
ADD_EXECUTABLE(gbn ${GBN_SRC})
TARGET_LINK_LIBRARIES(gbn ${NETSIM_LIB})

# SR Protocol
SET(SR_SRC
    ${PROJECT_SOURCE_DIR}/src/Sr.cpp
    ${PROJECT_SOURCE_DIR}/src/SrRdtSender.cpp
    ${PROJECT_SOURCE_DIR}/src/SrRdtReceiver.cpp
)
ADD_EXECUTABLE(sr ${SR_SRC})
TARGET_LINK_LIBRARIES(sr ${NETSIM_LIB})

# TCP Protocol
SET(TCP_SRC
    ${PROJECT_SOURCE_DIR}/src/TCP.cpp
    ${PROJECT_SOURCE_DIR}/src/TCPRdtSender.cpp
    ${PROJECT_SOURCE_DIR}/src/TCPRdtReceiver.cpp
)
ADD_EXECUTABLE(tcp ${TCP_SRC})
TARGET_LINK_LIBRARIES(tcp ${NETSIM_LIB})