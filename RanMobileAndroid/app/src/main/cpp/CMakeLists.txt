cmake_minimum_required(VERSION 3.22.1)
project(RanOnlineMobile)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ran_engine SHARED
    MobileTouchSystem.cpp
    RanMobileInputBridge.cpp
    RanAndroidMain.cpp
)

target_include_directories(ran_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__Engine/Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__Engine/Sources/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__Engine/Sources/DxCommon
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__Engine/Sources/DxCommon9
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__Engine/Sources/DxTools
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__RanClient/Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__RanClient/Sources/G-Logic/Client
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../[Lib]__RanClientUI/Sources
)

find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)
find_library(egl-lib EGL)

target_link_libraries(ran_engine
    ${log-lib}
    ${android-lib}
    ${gles3-lib}
    ${egl-lib}
)
