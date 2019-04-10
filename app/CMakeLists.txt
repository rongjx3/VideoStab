# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
# 设置opencv的动态库
add_library(libopencv_java3 SHARED IMPORTED)
set_target_properties(libopencv_java3 PROPERTIES IMPORTED_LOCATION
        ../../../../libs/${ANDROID_ABI}/libopencv_java3.so)

add_library(sp_lib SHARED src/main/cpp/stabilization/StableProcessor.cpp)
add_library(gf_lib SHARED src/main/cpp/stabilization/GuassFilter.cpp)
add_library(he_lib SHARED src/main/cpp/stabilization/HomoExtractorKlt.cpp)
add_library(ms_lib SHARED src/main/cpp/stabilization/MySemaphore.cpp)
add_library(ma_lib SHARED src/main/cpp/match.cpp)

add_library( # Sets the name of the library.
             media-handle

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).

             src/main/cpp/ffmpeg/cmdutils.c
             src/main/cpp/ffmpeg/ffmpeg.c
             src/main/cpp/ffmpeg/ffmpeg_filter.c
             src/main/cpp/ffmpeg/ffmpeg_opt.c
        )

add_library( # Sets the name of the library.
        native_opencv
        SHARED
        src/main/cpp/native-opencv.cpp)

add_library( ffmpeg
             SHARED
             IMPORTED )
set_target_properties( ffmpeg
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libffmpeg.so )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

include_directories(src/main/cpp)
include_directories(src/main/cpp/include)

find_library( # Sets the name of the path variable.
              log-lib
              log )

target_link_libraries(ms_lib ${log-lib})
target_link_libraries(gf_lib libopencv_java3 ${log-lib})
target_link_libraries(he_lib libopencv_java3 gf_lib ${log-lib})
target_link_libraries(sp_lib libopencv_java3 gf_lib he_lib ms_lib ${log-lib})
target_link_libraries(ma_lib libopencv_java3 ffmpeg ${log-lib})

target_link_libraries(native_opencv ffmpeg libopencv_java3 sp_lib he_lib gf_lib ma_lib ms_lib ${log-lib})

target_link_libraries( # Specifies the target library.
                       media-handle
                        libopencv_java3
                        native_opencv
                       -landroid #native_window
                       -ljnigraphics #bitmap
                       -lOpenSLES #openSLES
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )