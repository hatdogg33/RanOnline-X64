cmake_minimum_required(VERSION 3.22.1)
project(RanOnlineMobile LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This target is deliberately the Android platform/rendering shell only.
# RanMobileInputBridge.cpp is not compiled yet: it directly depends on the
# Windows/DirectX 9 game engine and cannot be linked into an Android build
# until those engine APIs have Android-compatible implementations.
add_library(ran_engine SHARED
    MobileTouchSystem.cpp
    RanAndroidMain.cpp
)

target_include_directories(ran_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_library(log-lib log REQUIRED)
find_library(android-lib android REQUIRED)
find_library(gles3-lib GLESv3 REQUIRED)
find_library(egl-lib EGL REQUIRED)

target_link_libraries(ran_engine PRIVATE
    ${log-lib}
    ${android-lib}
    ${gles3-lib}
    ${egl-lib}
)
